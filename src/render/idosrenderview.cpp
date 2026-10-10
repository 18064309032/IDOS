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
#include <QVBoxLayout>

#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
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
    , m_vtkWidget(nullptr)
    , m_activeBorder(nullptr)
    , m_flashOverlay(nullptr)
    , m_flashOpacityEffect(nullptr)
    , m_flashAnimation(nullptr)
    , m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New())
    , m_renderer(vtkSmartPointer<vtkRenderer>::New())
    , m_axesActor(vtkSmartPointer<vtkAxesActor>::New())
    , m_orientationMarker(vtkSmartPointer<vtkOrientationMarkerWidget>::New())
    , m_scene(new IDOSRenderScene())
    , m_actorObjectIds()
    , m_pressedPosition()
    , m_highlightedObjectId()
    , m_active(false)
    , m_ownsScene(true)
    , m_legendVisible(true)
    , m_legends()
{
    configureVtkOutputWindow();
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    m_vtkWidget->setFocusPolicy(Qt::StrongFocus);
    m_flashOverlay = new QWidget(m_vtkWidget);
    m_flashOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_flashOverlay->setStyleSheet(
        QStringLiteral("background-color: rgba(255, 255, 255, 220);"));
    m_flashOverlay->setGeometry(m_vtkWidget->rect());
    m_flashOverlay->hide();
    m_flashOpacityEffect = new QGraphicsOpacityEffect(m_flashOverlay);
    m_flashOpacityEffect->setOpacity(1.0);
    m_flashOverlay->setGraphicsEffect(m_flashOpacityEffect);
    m_flashAnimation = new QPropertyAnimation(m_flashOpacityEffect,
                                               QByteArrayLiteral("opacity"),
                                               this);
    m_flashAnimation->setDuration(180);
    m_flashAnimation->setStartValue(1.0);
    m_flashAnimation->setEndValue(0.0);
    m_flashAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_flashAnimation,
            &QPropertyAnimation::finished,
            m_flashOverlay,
            &QWidget::hide);

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
    m_renderWindow->AddRenderer(m_renderer);
    m_renderer->SetBackground(0.08, 0.09, 0.11);

    m_orientationMarker->SetOrientationMarker(m_axesActor);
    m_orientationMarker->SetInteractor(m_vtkWidget->interactor());
    m_orientationMarker->SetViewport(0.0, 0.0, 0.18, 0.18);
    m_orientationMarker->SetEnabled(1);
    m_orientationMarker->InteractiveOff();
    m_vtkWidget->installEventFilter(this);
}

IDOSRenderView::~IDOSRenderView()
{
    if (m_ownsScene)
    {
        delete m_scene;
    }
}

IDOSRenderScene* IDOSRenderView::scene() const
{
    return m_scene;
}

void IDOSRenderView::setScene(IDOSRenderScene* scene)
{
    if (m_scene == scene)
    {
        return;
    }
    if (m_ownsScene)
    {
        delete m_scene;
    }
    m_scene = scene;
    m_ownsScene = false;
    rebuildActors();
    resetCamera();
}

void IDOSRenderView::addObject(IDOSRenderObject* object)
{
    if (m_scene == nullptr)
    {
        return;
    }
    m_scene->addObject(object);
    rebuildActors();
}

void IDOSRenderView::setObject(IDOSRenderObject* object)
{
    if (m_scene == nullptr)
    {
        return;
    }
    m_scene->setObject(object);
    rebuildActors();
    resetCamera();
}

QString IDOSRenderView::currentObjectId() const
{
    if (m_scene == nullptr)
    {
        return QString();
    }
    const QList<IDOSRenderObject*>& objects = m_scene->objects();
    if (objects.size() != 1 || objects.first() == nullptr)
    {
        return QString();
    }
    return objects.first()->id();
}

void IDOSRenderView::clear()
{
    if (m_scene != nullptr)
    {
        m_scene->clear();
    }
    rebuildActors();
}

void IDOSRenderView::refresh()
{
    rebuildActors();
}

bool IDOSRenderView::orientationMarkerVisible() const
{
    return m_orientationMarker->GetEnabled() != 0;
}

void IDOSRenderView::setOrientationMarkerVisible(bool visible)
{
    m_orientationMarker->SetEnabled(visible ? 1 : 0);
    if (visible)
    {
        m_orientationMarker->InteractiveOff();
    }
    emit decorationsChanged();
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
    m_renderWindow->Render();
}

void IDOSRenderView::resetCamera()
{
    m_renderer->ResetCamera();
    m_renderWindow->Render();
}

void IDOSRenderView::setParallelProjection(bool enabled)
{
    vtkCamera* camera = m_renderer->GetActiveCamera();
    camera->SetParallelProjection(enabled ? 1 : 0);
    m_renderWindow->Render();
}

bool IDOSRenderView::parallelProjection() const
{
    return m_renderer->GetActiveCamera()->GetParallelProjection() != 0;
}

QColor IDOSRenderView::backgroundColor() const
{
    double background[3];
    m_renderer->GetBackground(background);
    return QColor::fromRgbF(background[0], background[1], background[2]);
}

void IDOSRenderView::setBackgroundColor(const QColor& color)
{
    if (!color.isValid())
    {
        return;
    }

    m_renderer->SetBackground(color.redF(), color.greenF(), color.blueF());
    m_renderWindow->Render();
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

bool IDOSRenderView::isActive() const
{
    return m_active;
}

void IDOSRenderView::updateActiveBorder()
{
    const QColor borderColor = m_active ? QColor(QStringLiteral("#1677D2"))
                                        : QColor(QStringLiteral("#707782"));
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
    if (bounds[0] > bounds[1] || bounds[2] > bounds[3] || bounds[4] > bounds[5])
    {
        return false;
    }

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

    double focalPoint[3] = {(bounds[0] + bounds[1]) * 0.5,
                            (bounds[2] + bounds[3]) * 0.5,
                            (bounds[4] + bounds[5]) * 0.5};
    vtkCamera* camera = m_renderer->GetActiveCamera();
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0] + direction[0],
                        focalPoint[1] + direction[1],
                        focalPoint[2] + direction[2]);
    camera->SetViewUp(viewUp);
    camera->OrthogonalizeViewUp();
    m_renderer->ResetCamera();
    m_renderWindow->Render();
    return true;
}

void IDOSRenderView::setHighlightedObjectId(const QString& objectId)
{
    if (m_highlightedObjectId == objectId)
    {
        return;
    }

    m_highlightedObjectId = objectId;
}

bool IDOSRenderView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_vtkWidget)
    {
        if (event->type() == QEvent::Resize)
        {
            m_flashOverlay->setGeometry(m_vtkWidget->rect());
        }

        if (event->type() == QEvent::FocusIn || event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::Wheel)
        {
            emit activated();
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
            if (mouseEvent->button() == Qt::LeftButton
                && (mouseEvent->pos() - m_pressedPosition).manhattanLength() < 4)
            {
                activateObjectAt(mouseEvent->pos());
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void IDOSRenderView::activateObjectAt(const QPoint& position)
{
    vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
    const int picked = picker->Pick(position.x(), m_vtkWidget->height() - position.y() - 1,
                                    0.0, m_renderer);
    if (picked == 0)
    {
        return;
    }

    vtkActor* actor = picker->GetActor();
    if (actor == nullptr || !m_actorObjectIds.contains(actor))
    {
        return;
    }

    emit objectActivated(m_actorObjectIds.value(actor));
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
        IDOSRenderProvider* provider = object->renderProvider();
        if (provider == nullptr)
        {
            continue;
        }
        provider->setHighlighted(object->id() == m_highlightedObjectId);
        provider->update();
        const QList<vtkActor*> actors = provider->actors();
        for (vtkActor* actor : actors)
        {
            m_renderer->AddActor(actor);
            m_actorObjectIds.insert(actor, object->id());
        }
        const QList<vtkSmartPointer<vtkProp>> legends = provider->legends();
        for (int legendIndex = 0; legendIndex < legends.size(); ++legendIndex)
        {
            vtkProp* legend = legends.at(legendIndex).GetPointer();
            m_renderer->AddViewProp(legend);
            legend->SetVisibility(m_legendVisible ? 1 : 0);
            m_legends.append(legend);
        }
    }

    emit decorationsChanged();
    m_renderWindow->Render();
}
