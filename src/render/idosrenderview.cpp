#include <QEasingCurve>
#include <QEvent>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHash>
#include <QList>
#include <QMouseEvent>
#include <QPoint>
#include <QPropertyAnimation>
#include <QString>
#include <QThread>
#include <QVBoxLayout>

#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutputWindow.h>
#include <vtkProp.h>
#include <vtkPropPicker.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStringOutputWindow.h>

#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idostool.h"
#include "idostoolbar.h"

#include "idosrenderview.h"

void IDOSRenderView::configureVtkOutputWindow()
{
    vtkStringOutputWindow* outputWindow = vtkStringOutputWindow::New();
    vtkOutputWindow::SetInstance(outputWindow);
    outputWindow->Delete();
}

IDOSRenderView::IDOSRenderView(QWidget* parent)
    :
    QWidget(parent)
    , m_toolsChanging(false)
    , m_pendingClick(false)
    , m_vtkWidget(nullptr)
    , m_activeBorder(nullptr)
    , m_flashOverlay(nullptr)
    , m_flashOpacityEffect(nullptr)
    , m_flashAnimation(nullptr)
    , m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New())
    , m_renderer(vtkSmartPointer<vtkRenderer>::New())
    , m_twoDInteractorStyle(vtkSmartPointer<vtkInteractorStyleImage>::New())
    , m_threeDInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New())
    , m_axesActor(vtkSmartPointer<vtkAxesActor>::New())
    , m_orientationMarker(vtkSmartPointer<vtkOrientationMarkerWidget>::New())
    , m_scene(new IDOSRenderScene(this))
    , m_actorObjectIds()
    , m_pressedPosition()
    , m_selection()
    , m_orientation(IDOSOrientation::Isometric)
    , m_active(false)
    , m_legendVisible(true)
    , m_legends()
{
    configureVtkOutputWindow();
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    m_vtkWidget->setFocusPolicy(Qt::StrongFocus);
    m_flashOverlay = new QWidget(m_vtkWidget);
    m_flashOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_flashOverlay->setStyleSheet(QStringLiteral("background-color: rgba(255, 255, 255, 220);"));
    m_flashOverlay->setGeometry(m_vtkWidget->rect());
    m_flashOverlay->hide();
    m_flashOpacityEffect = new QGraphicsOpacityEffect(m_flashOverlay);
    m_flashOpacityEffect->setOpacity(1.0);
    m_flashOverlay->setGraphicsEffect(m_flashOpacityEffect);
    m_flashAnimation = new QPropertyAnimation(m_flashOpacityEffect, QByteArrayLiteral("opacity"), this);
    m_flashAnimation->setDuration(180);
    m_flashAnimation->setStartValue(1.0);
    m_flashAnimation->setEndValue(0.0);
    m_flashAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_flashAnimation, &QPropertyAnimation::finished, m_flashOverlay, &QWidget::hide);

    m_activeBorder = new QFrame(this);
    m_activeBorder->setObjectName(QStringLiteral("activeViewBorder"));
    QVBoxLayout* borderLayout = new QVBoxLayout(m_activeBorder);
    borderLayout->setContentsMargins(2, 2, 2, 2);
    borderLayout->addWidget(m_vtkWidget);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_activeBorder);
    updateActiveBorder();

    m_vtkWidget->setRenderWindow(m_renderWindow);
    m_vtkWidget->interactor()->SetInteractorStyle(m_threeDInteractorStyle);
    m_renderWindow->AddRenderer(m_renderer);
    m_renderer->SetBackground(0.08, 0.09, 0.11);

    m_orientationMarker->SetOrientationMarker(m_axesActor);
    m_orientationMarker->SetInteractor(m_vtkWidget->interactor());
    m_orientationMarker->SetViewport(0.0, 0.0, 0.18, 0.18);
    m_orientationMarker->SetEnabled(1);
    m_orientationMarker->InteractiveOff();
    m_vtkWidget->installEventFilter(this);
    connect(m_scene, &IDOSRenderScene::objectAboutToBeRemoved, this, &IDOSRenderView::onObjectAboutToBeRemoved);
    connect(m_scene, &IDOSRenderScene::changed, this, &IDOSRenderView::onSceneChanged);
}

IDOSRenderView::~IDOSRenderView()
{
    clearEnabledTools();
    delete m_toolBar.data();
    m_toolBar = nullptr;
    disconnect(m_scene, nullptr, this, nullptr);
    m_renderer->RemoveAllViewProps();
    delete m_scene;
}

IDOSRenderScene* IDOSRenderView::scene() const
{
    return m_scene;
}

bool IDOSRenderView::axesVisible() const
{
    return m_orientationMarker->GetEnabled() != 0;
}

void IDOSRenderView::setAxesVisible(bool visible)
{
    m_orientationMarker->SetEnabled(visible ? 1 : 0);
    if (visible)
    {
        m_orientationMarker->InteractiveOff();
    }
    emit decorationsChanged();
    emit viewStateChanged();
    m_renderWindow->Render();
}

bool IDOSRenderView::legendVisible() const
{
    return m_legendVisible;
}

bool IDOSRenderView::legendAvailable() const
{
    return !m_legends.isEmpty();
}

void IDOSRenderView::setLegendVisible(bool visible)
{
    m_legendVisible = visible;
    for (int index = 0; index < m_legends.size(); ++index)
    {
        m_legends.at(index)->SetVisibility(visible);
    }
    emit decorationsChanged();
    emit viewStateChanged();
    m_renderWindow->Render();
}

void IDOSRenderView::resetCamera()
{
    m_renderer->ResetCamera();
    m_renderWindow->Render();
    emit viewStateChanged();
}

void IDOSRenderView::setTwoDimensional(bool enabled)
{
    if (isTwoDimensional() == enabled)
    {
        return;
    }
    vtkCamera* camera = m_renderer->GetActiveCamera();
    camera->SetParallelProjection(enabled ? 1 : 0);
    if (enabled)
    {
        m_vtkWidget->interactor()->SetInteractorStyle(m_twoDInteractorStyle);
    }
    else
    {
        m_vtkWidget->interactor()->SetInteractorStyle(m_threeDInteractorStyle);
    }
    m_renderWindow->Render();
    emit viewModeChanged();
    emit viewStateChanged();
}

bool IDOSRenderView::isTwoDimensional() const
{
    return m_renderer->GetActiveCamera()->GetParallelProjection() != 0;
}

IDOSOrientation IDOSRenderView::orientation() const
{
    return m_orientation;
}

QColor IDOSRenderView::background() const
{
    double background[3];
    m_renderer->GetBackground(background);
    return QColor::fromRgbF(background[0], background[1], background[2]);
}

void IDOSRenderView::setBackground(const QColor& color)
{
    if (!color.isValid())
    {
        return;
    }

    m_renderer->SetBackground(color.redF(), color.greenF(), color.blueF());
    m_renderWindow->Render();
    emit viewStateChanged();
}

void IDOSRenderView::setActive(bool active)
{
    if (m_active == active)
    {
        return;
    }

    m_active = active;
    updateActiveBorder();
}

void IDOSRenderView::updateActiveBorder()
{
    const QColor borderColor = m_active ? QColor(QStringLiteral("#1677D2")) : QColor(QStringLiteral("#707782"));
    const int borderWidth = m_active ? 2 : 1;
    m_activeBorder->setStyleSheet(
        QStringLiteral("QFrame#activeViewBorder { background-color: transparent; border: %1px solid %2; }")
            .arg(borderWidth)
            .arg(borderColor.name()));
}

QImage IDOSRenderView::captureImage()
{
    m_renderWindow->Render();
    return m_vtkWidget->grab().toImage();
}

void IDOSRenderView::flashScreenshot()
{
    m_flashOverlay->setGeometry(m_vtkWidget->rect());
    m_flashOverlay->show();
    m_flashOverlay->raise();
    m_flashAnimation->stop();
    m_flashOpacityEffect->setOpacity(1.0);
    m_flashAnimation->start();
}

bool IDOSRenderView::setOrientation(IDOSOrientation orientation)
{
    double bounds[6];
    m_renderer->ComputeVisiblePropBounds(bounds);
    const bool hasVisibleBounds = bounds[0] <= bounds[1] && bounds[2] <= bounds[3] && bounds[4] <= bounds[5];

    double direction[3] = {0.0, 0.0, 0.0};
    double viewUp[3] = {0.0, 0.0, 1.0};
    switch (orientation)
    {
    case IDOSOrientation::Front:
        direction[1] = -1.0;
        break;
    case IDOSOrientation::Back:
        direction[1] = 1.0;
        break;
    case IDOSOrientation::Left:
        direction[0] = -1.0;
        break;
    case IDOSOrientation::Right:
        direction[0] = 1.0;
        break;
    case IDOSOrientation::Top:
        direction[2] = 1.0;
        viewUp[1] = 1.0;
        viewUp[2] = 0.0;
        break;
    case IDOSOrientation::Bottom:
        direction[2] = -1.0;
        viewUp[1] = -1.0;
        viewUp[2] = 0.0;
        break;
    case IDOSOrientation::Perspective:
    case IDOSOrientation::Isometric:
        direction[0] = 1.0;
        direction[1] = -1.0;
        direction[2] = 1.0;
        break;
    default:
        direction[0] = 1.0;
        direction[1] = -1.0;
        direction[2] = 1.0;
        break;
    }

    vtkCamera* camera = m_renderer->GetActiveCamera();
    double focalPoint[3];
    camera->GetFocalPoint(focalPoint);
    if (hasVisibleBounds)
    {
        focalPoint[0] = (bounds[0] + bounds[1]) * 0.5;
        focalPoint[1] = (bounds[2] + bounds[3]) * 0.5;
        focalPoint[2] = (bounds[4] + bounds[5]) * 0.5;
    }
    const double distance = camera->GetDistance();
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0] + direction[0], focalPoint[1] + direction[1], focalPoint[2] + direction[2]);
    camera->SetViewUp(viewUp);
    camera->OrthogonalizeViewUp();
    if (hasVisibleBounds)
    {
        m_renderer->ResetCamera();
    }
    else
    {
        camera->SetPosition(focalPoint[0] + direction[0] * distance,
                            focalPoint[1] + direction[1] * distance,
                            focalPoint[2] + direction[2] * distance);
    }
    m_orientation = orientation;
    m_renderWindow->Render();
    emit viewStateChanged();
    return true;
}

bool IDOSRenderView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_vtkWidget)
    {
        if (event->type() == QEvent::ContextMenu)
        {
            event->accept();
            return true;
        }
        if (event->type() == QEvent::Resize)
        {
            m_flashOverlay->setGeometry(m_vtkWidget->rect());
            positionToolBar();
        }
        if (event->type() == QEvent::FocusOut || event->type() == QEvent::Hide)
        {
            cancelToolInteraction();
        }

        if (event->type() == QEvent::FocusIn || event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::Wheel)
        {
            emit focused();
        }

        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                m_pressedPosition = mouseEvent->pos();
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton && (mouseEvent->pos() - m_pressedPosition).manhattanLength() < 4)
            {
                if (m_enabledTools.isEmpty())
                {
                    activateObjectAt(mouseEvent->pos());
                }
                else
                {
                    m_pendingClick = true;
                    m_clickPosition = mouseEvent->pos();
                    QMetaObject::invokeMethod(this, "onCanvasClick", Qt::QueuedConnection);
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

IDOSRenderObject* IDOSRenderView::pick(const QPoint& position) const
{
    vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
    if (picker->Pick(position.x(), m_vtkWidget->height() - position.y() - 1, 0.0, m_renderer) == 0)
    {
        return nullptr;
    }
    return m_scene->object(m_actorObjectIds.value(picker->GetActor()));
}

void IDOSRenderView::activateObjectAt(const QPoint& position)
{
    IDOSRenderObject* object = pick(position);
    if (object != nullptr)
    {
        setSelection(QList<QString>{object->id()});
        emit activated(object->id());
    }
}
void IDOSRenderView::rebuildActors()
{
    m_renderer->RemoveAllViewProps();
    m_actorObjectIds.clear();
    m_legends.clear();

    if (m_scene == nullptr)
    {
        emit decorationsChanged();
        m_renderWindow->Render();
        return;
    }

    const QList<IDOSRenderObject*>& objects = m_scene->objects();
    for (int index = 0; index < objects.size(); ++index)
    {
        IDOSRenderObject* object = objects.at(index);
        if (object == nullptr)
        {
            continue;
        }
        IDOSRenderProvider* provider = object->provider();
        if (provider == nullptr)
        {
            continue;
        }
        const QList<vtkActor*> actors = provider->actors();
        for (vtkActor* actor : actors)
        {
            m_renderer->AddActor(actor);
            m_actorObjectIds.insert(actor, object->id());
        }
        const QList<vtkSmartPointer<vtkProp>> legends = provider->legends();
        for (int legendIndex = 0; legendIndex < legends.size(); ++legendIndex)
        {
            vtkSmartPointer<vtkProp> legend;
            legend.TakeReference(legends.at(legendIndex)->NewInstance());
            legend->ShallowCopy(legends.at(legendIndex));
            m_renderer->AddViewProp(legend);
            legend->SetVisibility(m_legendVisible ? 1 : 0);
            m_legends.append(legend.GetPointer());
        }
    }

    emit decorationsChanged();
    m_renderWindow->Render();
}

QList<QString> IDOSRenderView::selection() const
{
    return m_selection;
}

void IDOSRenderView::setSelection(const QList<QString>& ids)
{
    QList<QString> valid;
    for (const QString& id : ids)
    {
        if (m_scene->contains(id) && !valid.contains(id))
        {
            valid.append(id);
        }
    }
    bool unchanged = valid.size() == m_selection.size();
    for (int index = 0; unchanged && index < valid.size(); ++index)
    {
        unchanged = valid.at(index) == m_selection.at(index);
    }
    if (unchanged)
    {
        return;
    }
    m_selection = valid;
    m_scene->beginUpdate();
    const QList<IDOSRenderObject*> objects = m_scene->objects();
    for (IDOSRenderObject* object : objects)
    {
        const bool highlighted = valid.contains(object->id());
        if (object->highlighted() != highlighted)
        {
            object->setHighlighted(highlighted);
            m_scene->notifyChanged(object->id());
        }
    }
    m_scene->endUpdate();
    emit selectionChanged(m_selection);
    emit viewStateChanged();
}

void IDOSRenderView::onObjectAboutToBeRemoved(const QString& id)
{
    // Release VTK references and pick mappings before the provider is destroyed.
    m_renderer->RemoveAllViewProps();
    m_actorObjectIds.clear();
    m_legends.clear();
    if (m_selection.removeAll(id) > 0)
    {
        emit selectionChanged(m_selection);
    }
}

void IDOSRenderView::onSceneChanged()
{
    rebuildActors();
    emit viewStateChanged();
}

void IDOSRenderView::render()
{
    m_renderWindow->Render();
}

QVTKOpenGLNativeWidget* IDOSRenderView::vtkWidget() const
{
    return m_vtkWidget;
}

IDOSToolBar* IDOSRenderView::toolBar() const
{
    return m_toolBar.data();
}

bool IDOSRenderView::isToolTransitioning() const
{
    return m_toolsChanging;
}

bool IDOSRenderView::toolsBusy() const
{
    for (const QPointer<IDOSTool>& tool : m_enabledTools)
    {
        if (tool != nullptr && tool->isBusy())
        {
            return true;
        }
    }
    return false;
}

bool IDOSRenderView::setToolBar(IDOSToolBar* toolBar)
{
    if (QThread::currentThread() != thread() || toolsBusy() || m_toolsChanging ||
        (toolBar != nullptr && (toolBar->thread() != thread() ||
         (toolBar->view() != nullptr && toolBar->view() != this))) ||
        (toolBar != nullptr && toolBar->isBusy()) ||
        (m_toolBar != nullptr && m_toolBar->isBusy()))
    {
        return false;
    }
    if (m_toolBar == toolBar)
    {
        return true;
    }
    delete m_toolBar.data();
    m_toolBar = toolBar;
    if (toolBar != nullptr)
    {
        connect(toolBar, &QObject::destroyed, this, &IDOSRenderView::onToolBarDestroyed);
        connect(toolBar, &IDOSToolBar::layoutChanged, this, &IDOSRenderView::onToolBarLayoutChanged);
        toolBar->setView(this);
        positionToolBar();
    }
    return true;
}

void IDOSRenderView::positionToolBar()
{
    if (m_toolBar == nullptr)
    {
        return;
    }
    const QSize preferred = m_toolBar->sizeHint();
    const int available = qMax(0, m_vtkWidget->width() - 16);
    const int width = qMin(available, preferred.width());
    m_toolBar->setGeometry((m_vtkWidget->width() - width) / 2, 8, width, preferred.height());
    m_toolBar->raise();
}

bool IDOSRenderView::enableTool(IDOSTool* tool)
{
    if (tool == nullptr || tool->view() != this || QThread::currentThread() != thread() || m_toolsChanging)
    {
        return false;
    }
    if (toolsBusy())
    {
        m_pendingTools.append(QPointer<IDOSTool>(tool));
        m_pendingEnabled.append(true);
        QMetaObject::invokeMethod(this, "onPendingTools", Qt::QueuedConnection);
        return true;
    }
    if (isToolEnabled(tool))
    {
        return true;
    }
    const QString group = tool->exclusiveGroup();
    const QList<QPointer<IDOSTool>> existing = m_enabledTools;
    for (const QPointer<IDOSTool>& other : existing)
    {
        if (other != nullptr && !group.isEmpty() && other->exclusiveGroup() == group)
        {
            disableTool(other);
        }
    }
    m_toolsChanging = true;
    m_enabledTools.append(QPointer<IDOSTool>(tool));
    connect(tool, &QObject::destroyed, this, &IDOSRenderView::onToolDestroyed);
    connect(tool, &IDOSTool::callbackFinished, this, &IDOSRenderView::onToolCallbackFinished);
    tool->setActive(true);
    emit toolEnabled(tool);
    m_toolsChanging = false;
    emit viewStateChanged();
    return true;
}

bool IDOSRenderView::disableTool(IDOSTool* tool)
{
    if (tool == nullptr || QThread::currentThread() != thread() || m_toolsChanging)
    {
        return false;
    }
    if (toolsBusy())
    {
        m_pendingTools.append(QPointer<IDOSTool>(tool));
        m_pendingEnabled.append(false);
        QMetaObject::invokeMethod(this, "onPendingTools", Qt::QueuedConnection);
        return true;
    }
    if (!isToolEnabled(tool))
    {
        return false;
    }
    m_toolsChanging = true;
    releaseTool(tool);
    tool->cancel();
    m_enabledTools.removeAll(QPointer<IDOSTool>(tool));
    tool->setActive(false);
    disconnect(tool, &QObject::destroyed, this, &IDOSRenderView::onToolDestroyed);
    emit toolDisabled(tool);
    disconnect(tool, &IDOSTool::callbackFinished, this, &IDOSRenderView::onToolCallbackFinished);
    m_toolsChanging = false;
    emit viewStateChanged();
    return true;
}

bool IDOSRenderView::isToolEnabled(const IDOSTool* tool) const
{
    for (const QPointer<IDOSTool>& enabled : m_enabledTools)
    {
        if (enabled != nullptr && enabled.data() == tool)
        {
            return true;
        }
    }
    return false;
}

QList<IDOSTool*> IDOSRenderView::enabledTools() const
{
    QList<IDOSTool*> result;
    for (const QPointer<IDOSTool>& tool : m_enabledTools)
    {
        if (tool != nullptr)
        {
            result.append(tool.data());
        }
    }
    return result;
}

bool IDOSRenderView::captureTool(IDOSTool* tool)
{
    if (!isToolEnabled(tool) || (m_capturedTool != nullptr && m_capturedTool != tool))
    {
        return false;
    }
    m_capturedTool = tool;
    if (m_vtkWidget->isVisible())
    {
        m_vtkWidget->grabMouse();
    }
    return true;
}

void IDOSRenderView::releaseTool(IDOSTool* tool)
{
    if (tool != nullptr && m_capturedTool == tool)
    {
        m_capturedTool = nullptr;
        if (QWidget::mouseGrabber() == m_vtkWidget)
        {
            m_vtkWidget->releaseMouse();
        }
    }
}

IDOSTool* IDOSRenderView::capturedTool() const
{
    return m_capturedTool.data();
}

void IDOSRenderView::cancelToolInteraction()
{
    m_pendingClick = false;
    releaseTool(m_capturedTool.data());
    const QList<QPointer<IDOSTool>> tools = m_enabledTools;
    for (const QPointer<IDOSTool>& tool : tools)
    {
        if (tool != nullptr)
        {
            tool->cancel();
        }
    }
}

void IDOSRenderView::onCanvasClick()
{
    if (m_pendingClick)
    {
        m_pendingClick = false;
        activateObjectAt(m_clickPosition);
    }
}

void IDOSRenderView::clearEnabledTools()
{
    const QList<QPointer<IDOSTool>> tools = m_enabledTools;
    for (const QPointer<IDOSTool>& tool : tools)
    {
        if (tool != nullptr)
        {
            disableTool(tool);
        }
    }
    m_pendingTools.clear();
    m_pendingEnabled.clear();
}

void IDOSRenderView::onToolDestroyed(QObject* object)
{
    Q_UNUSED(object)
    for (int index = m_enabledTools.size() - 1; index >= 0; --index)
    {
        if (m_enabledTools.at(index) == nullptr)
        {
            m_enabledTools.removeAt(index);
        }
    }
    if (m_capturedTool == nullptr && QWidget::mouseGrabber() == m_vtkWidget)
    {
        m_vtkWidget->releaseMouse();
    }
}

void IDOSRenderView::onToolBarDestroyed(QObject* object)
{
    Q_UNUSED(object)
    m_toolBar = nullptr;
}

void IDOSRenderView::onPendingTools()
{
    if (toolsBusy() || m_toolsChanging)
    {
        return;
    }
    const QList<QPointer<IDOSTool>> pending = m_pendingTools;
    const QList<bool> enabled = m_pendingEnabled;
    m_pendingTools.clear();
    m_pendingEnabled.clear();
    for (int index = 0; index < pending.size(); ++index)
    {
        if (pending.at(index) != nullptr)
        {
            if (enabled.at(index))
            {
                enableTool(pending.at(index));
            }
            else
            {
                disableTool(pending.at(index));
            }
        }
    }
}

void IDOSRenderView::onToolCallbackFinished(bool consumed)
{
    if (consumed)
    {
        m_pendingClick = false;
    }
    if (!m_pendingTools.isEmpty())
    {
        QMetaObject::invokeMethod(this, "onPendingTools", Qt::QueuedConnection);
    }
}

void IDOSRenderView::onToolBarLayoutChanged()
{
    positionToolBar();
}
