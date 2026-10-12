#include <QEvent>
#include <QMouseEvent>
#include <QThread>

#include <QVTKOpenGLNativeWidget.h>
#include <vtkCommand.h>
#include <vtkObject.h>

#include "idosrenderview.h"

#include "idostool.h"

IDOSTool::IDOSTool(IDOSRenderView* view)
    :
    QObject(nullptr)
    , m_view(view)
    , m_active(false)
    , m_callbackDepth(0)
{
}

IDOSTool::~IDOSTool()
{
    if (m_view != nullptr)
    {
        m_view->vtkWidget()->removeEventFilter(this);
    }
    removeVtkObservers();
}

IDOSRenderView* IDOSTool::view() const
{
    return m_view.data();
}

bool IDOSTool::isActive() const
{
    return m_active;
}

bool IDOSTool::isBusy() const
{
    return m_callbackDepth != 0;
}

QString IDOSTool::exclusiveGroup() const
{
    return QString();
}

void IDOSTool::cancel()
{
}

void IDOSTool::activate()
{
    if (m_active || m_view == nullptr)
    {
        return;
    }
    m_active = true;
    m_view->vtkWidget()->installEventFilter(this);
    // Keep framework focus and cancellation handling ahead of tool filters.
    m_view->vtkWidget()->installEventFilter(m_view);
    onActivated();
}

void IDOSTool::deactivate()
{
    if (!m_active)
    {
        return;
    }
    m_active = false;
    if (m_view != nullptr)
    {
        m_view->vtkWidget()->removeEventFilter(this);
    }
    onDeactivated();
    removeVtkObservers();
}

bool IDOSTool::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_active || m_view == nullptr || watched != m_view->vtkWidget())
    {
        return false;
    }
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::Wheel:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        break;
    default:
        return false;
    }
    const bool pointerEvent = event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease;
    IDOSTool* captured = m_view->capturedTool();
    if (pointerEvent && captured != nullptr && captured != this)
    {
        return false;
    }
    ++m_callbackDepth;
    const bool consumed = onEventFilter(watched, event);
    if (captured == this && event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->buttons() == Qt::NoButton)
        {
            m_view->releaseTool(this);
        }
    }
    emit callbackFinished(consumed || (pointerEvent && captured == this));
    --m_callbackDepth;
    return consumed || (pointerEvent && captured == this);
}

bool IDOSTool::onEventFilter(QObject* watched, QEvent* event)
{
    return QObject::eventFilter(watched, event);
}

void IDOSTool::onActivated()
{
}

void IDOSTool::onDeactivated()
{
}

bool IDOSTool::observeVtkEvent(vtkObject* source, unsigned long eventId, float priority)
{
    if (!m_active || source == nullptr || eventId == vtkCommand::NoEvent ||
        QThread::currentThread() != thread())
    {
        return false;
    }
    for (int index = 0; index < m_observerSources.size(); ++index)
    {
        if (m_observerSources.at(index).GetPointer() == source && m_observerEvents.at(index) == eventId)
        {
            return false;
        }
    }
    const unsigned long tag = source->AddObserver(eventId, this, &IDOSTool::onVtkEventReceived, priority);
    if (tag == 0)
    {
        return false;
    }
    m_observerSources.append(vtkWeakPointer<vtkObject>(source));
    m_observerEvents.append(eventId);
    m_observerTags.append(tag);
    return true;
}

void IDOSTool::onVtkEventReceived(vtkObject* source, unsigned long eventId, void* callData)
{
    if (!m_active || QThread::currentThread() != thread())
    {
        return;
    }
    ++m_callbackDepth;
    onVtkEvent(source, eventId, callData);
    emit callbackFinished(false);
    --m_callbackDepth;
}

void IDOSTool::onVtkEvent(vtkObject* source, unsigned long eventId, void* callData)
{
    Q_UNUSED(source)
    Q_UNUSED(eventId)
    Q_UNUSED(callData)
}

bool IDOSTool::setActive(bool active)
{
    if (m_view == nullptr || !m_view->isToolTransitioning() ||
        m_view->isToolEnabled(this) != active || isBusy())
    {
        return false;
    }
    if (active)
    {
        activate();
    }
    else
    {
        deactivate();
    }
    return true;
}

void IDOSTool::removeVtkObservers()
{
    for (int index = 0; index < m_observerTags.size(); ++index)
    {
        vtkObject* source = m_observerSources.at(index).GetPointer();
        if (source != nullptr)
        {
            source->RemoveObserver(m_observerTags.at(index));
        }
    }
    m_observerSources.clear();
    m_observerEvents.clear();
    m_observerTags.clear();
}
