#include <cmath>
#include <limits>

#include <QEasingCurve>
#include <QEvent>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHash>
#include <QLabel>
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
#include <vtkCellPicker.h>
#include <vtkCommand.h>
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
#include "idosrenderscene.h"
#include "idostool.h"
#include "idostoolbar.h"
#include "providers/idosrenderprovider.h"

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
    , m_vtkWidget(nullptr)
    , m_activeBorder(nullptr)
    , m_flashOverlay(nullptr)
    , m_cursorCoordinates(nullptr)
    , m_flashOpacityEffect(nullptr)
    , m_flashAnimation(nullptr)
    , m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New())
    , m_renderer(vtkSmartPointer<vtkRenderer>::New())
    , m_homeCamera(vtkSmartPointer<vtkCamera>::New())
    , m_savedCamera()
    , m_linkedCamera()
    , m_cameraObserverTag(0)
    , m_interactionObserverTag(0)
    , m_twoDInteractorStyle(vtkSmartPointer<vtkInteractorStyleImage>::New())
    , m_threeDInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New())
    , m_axesActor(vtkSmartPointer<vtkAxesActor>::New())
    , m_orientationMarker(vtkSmartPointer<vtkOrientationMarkerWidget>::New())
    , m_scene(new IDOSRenderScene(this))
    , m_actorObjectIds()
    , m_selection()
    , m_displayVisibilitySnapshot()
    , m_orientation(IDOSOrientation::Isometric)
    , m_active(false)
    , m_twoDimensional(false)
    , m_homeTwoDimensional(false)
    , m_homeViewInitialized(false)
    , m_cursorTrackingEnabled(false)
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
    m_cursorCoordinates = new QLabel(m_vtkWidget);
    m_cursorCoordinates->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_cursorCoordinates->setStyleSheet(QStringLiteral("QLabel { background: rgba(247, 249, 252, 220); color: #334155; padding: 3px 6px; border-radius: 3px; }"));
    m_cursorCoordinates->move(8, m_vtkWidget->height() - 30);
    m_cursorCoordinates->hide();
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
    m_interactionObserverTag = m_vtkWidget->interactor()->AddObserver(
        vtkCommand::EndInteractionEvent, this, &IDOSRenderView::onCameraInteraction);
    m_renderWindow->AddRenderer(m_renderer);
    m_renderer->SetBackground(0.08, 0.09, 0.11);
    m_homeCamera->DeepCopy(m_renderer->GetActiveCamera());

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
    m_vtkWidget->interactor()->RemoveObserver(m_interactionObserverTag);
    unlinkCamera();
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

bool IDOSRenderView::fitSelection()
{
    if (m_selection.isEmpty())
    {
        return false;
    }

    double bounds[6] = {std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    bool hasBounds = false;
    for (const QString& id : m_selection)
    {
        IDOSRenderObject* object = m_scene->object(id);
        if (object == nullptr || !object->visible() || object->provider() == nullptr)
        {
            continue;
        }
        const QList<vtkActor*> actors = object->provider()->actors();
        for (vtkActor* actor : actors)
        {
            if (actor == nullptr || actor->GetVisibility() == 0)
            {
                continue;
            }
            double actorBounds[6];
            actor->GetBounds(actorBounds);
            if (actorBounds[0] > actorBounds[1] || actorBounds[2] > actorBounds[3] || actorBounds[4] > actorBounds[5])
            {
                continue;
            }
            bounds[0] = qMin(bounds[0], actorBounds[0]);
            bounds[1] = qMax(bounds[1], actorBounds[1]);
            bounds[2] = qMin(bounds[2], actorBounds[2]);
            bounds[3] = qMax(bounds[3], actorBounds[3]);
            bounds[4] = qMin(bounds[4], actorBounds[4]);
            bounds[5] = qMax(bounds[5], actorBounds[5]);
            hasBounds = true;
        }
    }
    if (!hasBounds)
    {
        return false;
    }

    m_renderer->ResetCamera(bounds);
    m_renderWindow->Render();
    emit viewStateChanged();
    return true;
}

void IDOSRenderView::rotateView(bool clockwise)
{
    vtkCamera* camera = m_renderer->GetActiveCamera();
    camera->Roll(clockwise ? -90.0 : 90.0);
    camera->OrthogonalizeViewUp();
    m_renderWindow->Render();
    emit viewStateChanged();
}

void IDOSRenderView::saveHomeView()
{
    m_homeCamera->DeepCopy(m_renderer->GetActiveCamera());
    m_homeTwoDimensional = m_twoDimensional;
    m_homeViewInitialized = true;
}

void IDOSRenderView::resetToHomeView()
{
    const bool modeChanged = m_twoDimensional != m_homeTwoDimensional;
    m_twoDimensional = m_homeTwoDimensional;
    if (m_twoDimensional)
    {
        m_vtkWidget->interactor()->SetInteractorStyle(m_twoDInteractorStyle);
    }
    else
    {
        m_vtkWidget->interactor()->SetInteractorStyle(m_threeDInteractorStyle);
    }
    m_renderer->GetActiveCamera()->DeepCopy(m_homeCamera);
    m_renderer->ResetCameraClippingRange();
    m_renderWindow->Render();
    if (modeChanged)
    {
        emit viewModeChanged();
    }
    emit viewStateChanged();
}

bool IDOSRenderView::setParallelProjection(bool enabled)
{
    vtkCamera* camera = m_renderer->GetActiveCamera();
    if ((camera->GetParallelProjection() != 0) == enabled)
    {
        return false;
    }
    camera->SetParallelProjection(enabled ? 1 : 0);
    m_renderWindow->Render();
    emit viewStateChanged();
    return true;
}

bool IDOSRenderView::parallelProjection() const
{
    return m_renderer->GetActiveCamera()->GetParallelProjection() != 0;
}

void IDOSRenderView::scaleView(double factor)
{
    if (factor <= 0.0)
    {
        return;
    }
    m_renderer->GetActiveCamera()->Zoom(factor);
    m_renderer->ResetCameraClippingRange();
    m_renderWindow->Render();
    emit viewStateChanged();
}

void IDOSRenderView::setDisplayVisible(bool visible)
{
    m_scene->beginUpdate();
    const QList<IDOSRenderObject*> objects = m_scene->objects();
    if (!visible)
    {
        m_displayVisibilitySnapshot.clear();
        for (IDOSRenderObject* object : objects)
        {
            m_displayVisibilitySnapshot.insert(object->id(), object->visible());
        }
    }
    for (IDOSRenderObject* object : objects)
    {
        const bool targetVisible = visible
                                       ? m_displayVisibilitySnapshot.value(object->id(), m_displayVisibilitySnapshot.isEmpty())
                                       : false;
        if (object->visible() != targetVisible)
        {
            object->setVisible(targetVisible);
            m_scene->notifyChanged(object->id());
        }
    }
    if (visible)
    {
        m_displayVisibilitySnapshot.clear();
    }
    m_scene->endUpdate();
}

bool IDOSRenderView::linkCameraTo(IDOSRenderView* sourceView)
{
    if (sourceView == nullptr || sourceView == this || sourceView->isTwoDimensional() || isTwoDimensional())
    {
        return false;
    }
    vtkCamera* sourceCamera = sourceView->m_renderer->GetActiveCamera();
    if (isCameraLinked() && m_linkedCamera.GetPointer() == sourceCamera)
    {
        return true;
    }
    unlinkCamera();
    m_savedCamera = vtkSmartPointer<vtkCamera>::New();
    m_savedCamera->DeepCopy(m_renderer->GetActiveCamera());
    m_renderer->SetActiveCamera(sourceCamera);
    m_linkedCamera = sourceCamera;
    m_cameraObserverTag = sourceCamera->AddObserver(
        vtkCommand::ModifiedEvent, this, &IDOSRenderView::onLinkedCameraModified);
    if (m_cameraObserverTag == 0)
    {
        m_renderer->SetActiveCamera(m_savedCamera);
        m_savedCamera = nullptr;
        m_linkedCamera = nullptr;
        return false;
    }
    m_renderWindow->Render();
    emit viewStateChanged();
    return true;
}

void IDOSRenderView::unlinkCamera()
{
    vtkCamera* linkedCamera = m_linkedCamera.GetPointer();
    if (linkedCamera != nullptr && m_cameraObserverTag != 0)
    {
        linkedCamera->RemoveObserver(m_cameraObserverTag);
    }
    m_cameraObserverTag = 0;
    m_linkedCamera = nullptr;
    if (m_savedCamera != nullptr)
    {
        m_renderer->SetActiveCamera(m_savedCamera);
        m_savedCamera = nullptr;
    }
    if (m_renderWindow != nullptr)
    {
        m_renderWindow->Render();
    }
    emit viewStateChanged();
}

bool IDOSRenderView::isCameraLinked() const
{
    return m_cameraObserverTag != 0 && m_linkedCamera.GetPointer() != nullptr;
}

void IDOSRenderView::onLinkedCameraModified(vtkObject* source, unsigned long eventId, void* callData)
{
    Q_UNUSED(source)
    Q_UNUSED(eventId)
    Q_UNUSED(callData)
    m_renderWindow->Render();
    emit viewStateChanged();
}

bool IDOSRenderView::cursorTrackingEnabled() const
{
    return m_cursorTrackingEnabled;
}

void IDOSRenderView::setCursorTrackingEnabled(bool enabled)
{
    m_cursorTrackingEnabled = enabled;
    m_vtkWidget->setMouseTracking(enabled);
    if (!enabled)
    {
        m_cursorCoordinates->hide();
    }
    emit viewStateChanged();
}

void IDOSRenderView::setTwoDimensional(bool enabled)
{
    if (m_twoDimensional == enabled)
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
    m_twoDimensional = enabled;
    m_renderWindow->Render();
    emit viewModeChanged();
    emit viewStateChanged();
}

bool IDOSRenderView::isTwoDimensional() const
{
    return m_twoDimensional;
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
    if (!m_homeViewInitialized)
    {
        m_homeCamera->DeepCopy(camera);
        m_homeTwoDimensional = m_twoDimensional;
        m_homeViewInitialized = true;
    }
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
            m_cursorCoordinates->hide();
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

        if (event->type() == QEvent::Leave)
        {
            m_cursorCoordinates->hide();
        }
        if (event->type() == QEvent::MouseMove && m_cursorTrackingEnabled)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            double worldPoint[3];
            if (pickWorldPosition(mouseEvent->pos(), worldPoint))
            {
                const double x = worldPoint[0];
                const double y = worldPoint[1];
                const double z = worldPoint[2];
                m_cursorCoordinates->setText(QStringLiteral("X %1   Y %2   Z %3")
                                                 .arg(x, 0, 'g', 5)
                                                 .arg(y, 0, 'g', 5)
                                                 .arg(z, 0, 'g', 5));
                m_cursorCoordinates->adjustSize();
                m_cursorCoordinates->move(
                    qBound(0, mouseEvent->pos().x() + 14, qMax(0, m_vtkWidget->width() - m_cursorCoordinates->width())),
                    qBound(0, mouseEvent->pos().y() + 18, qMax(0, m_vtkWidget->height() - m_cursorCoordinates->height())));
                m_cursorCoordinates->show();
                m_cursorCoordinates->raise();
            }
            else
            {
                m_cursorCoordinates->hide();
            }
        }

    }
    return QWidget::eventFilter(watched, event);
}

IDOSRenderObject* IDOSRenderView::pick(const QPoint& position) const
{
    vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
    if (m_vtkWidget->width() <= 0 || m_vtkWidget->height() <= 0)
    {
        return nullptr;
    }
    const int* size = m_renderWindow->GetSize();
    const double x = static_cast<double>(position.x()) * size[0] / m_vtkWidget->width();
    const double y = size[1] - 1.0 - static_cast<double>(position.y()) * size[1] / m_vtkWidget->height();
    if (picker->Pick(x, y, 0.0, m_renderer) == 0)
    {
        return nullptr;
    }
    return m_scene->object(m_actorObjectIds.value(picker->GetActor()));
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

bool IDOSRenderView::pickWorldPosition(const QPoint& position, double worldPosition[3]) const
{
    if (worldPosition == nullptr || m_renderer == nullptr || m_vtkWidget == nullptr)
    {
        return false;
    }
    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);
    picker->PickFromListOn();
    for (vtkActor* actor : m_actorObjectIds.keys())
    {
        if (actor->GetVisibility() != 0)
        {
            picker->AddPickList(actor);
        }
    }
    const int* renderSize = m_renderWindow->GetSize();
    if (m_vtkWidget->width() <= 0 || m_vtkWidget->height() <= 0 ||
        renderSize[0] <= 0 || renderSize[1] <= 0)
    {
        return false;
    }
    const double displayX = static_cast<double>(position.x()) * renderSize[0] / m_vtkWidget->width();
    const double displayY = renderSize[1] - 1.0 -
                           static_cast<double>(position.y()) * renderSize[1] / m_vtkWidget->height();
    if (!picker->Pick(displayX, displayY, 0.0, m_renderer))
    {
        return false;
    }
    picker->GetPickPosition(worldPosition);
    return true;
}

QString IDOSRenderView::objectIdAt(const QPoint& position) const
{
    IDOSRenderObject* object = pick(position);
    return object != nullptr ? object->id() : QString();
}

void IDOSRenderView::zoomToWorldPosition(const double worldPosition[3])
{
    if (worldPosition == nullptr)
    {
        return;
    }
    vtkCamera* camera = m_renderer->GetActiveCamera();
    double focalPoint[3];
    double cameraPosition[3];
    camera->GetFocalPoint(focalPoint);
    camera->GetPosition(cameraPosition);
    camera->SetFocalPoint(worldPosition);
    camera->SetPosition(worldPosition[0] + cameraPosition[0] - focalPoint[0],
                         worldPosition[1] + cameraPosition[1] - focalPoint[1],
                         worldPosition[2] + cameraPosition[2] - focalPoint[2]);
    camera->Zoom(1.5);
    m_renderer->ResetCameraClippingRange();
    m_renderWindow->Render();
    emit viewStateChanged();
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
        connect(toolBar, &IDOSToolBar::presentationChanged, this, &IDOSRenderView::viewStateChanged);
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
    int x = (m_vtkWidget->width() - width) / 2;
    int y = 8;
    if (m_toolBar->isUserPositioned())
    {
        x = qBound(0, m_toolBar->x(), qMax(0, m_vtkWidget->width() - width));
        y = qBound(0, m_toolBar->y(), qMax(0, m_vtkWidget->height() - preferred.height()));
    }
    m_toolBar->setGeometry(x, y, width, preferred.height());
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
    Q_UNUSED(consumed)
    if (!m_pendingTools.isEmpty())
    {
        QMetaObject::invokeMethod(this, "onPendingTools", Qt::QueuedConnection);
    }
}

void IDOSRenderView::onToolBarLayoutChanged()
{
    positionToolBar();
}

QVariantMap IDOSRenderView::cameraState(vtkCamera* camera)
{
    QVariantMap state;
    QVariantList position;
    QVariantList focalPoint;
    QVariantList viewUp;
    for (int axis = 0; axis < 3; ++axis)
    {
        position.append(camera->GetPosition()[axis]);
        focalPoint.append(camera->GetFocalPoint()[axis]);
        viewUp.append(camera->GetViewUp()[axis]);
    }
    state.insert(QStringLiteral("position"), position);
    state.insert(QStringLiteral("focalPoint"), focalPoint);
    state.insert(QStringLiteral("viewUp"), viewUp);
    state.insert(QStringLiteral("parallel"), camera->GetParallelProjection() != 0);
    state.insert(QStringLiteral("parallelScale"), camera->GetParallelScale());
    state.insert(QStringLiteral("viewAngle"), camera->GetViewAngle());
    return state;
}

bool IDOSRenderView::restoreCameraState(vtkCamera* camera, const QVariantMap& state)
{
    const QVariantList position = state.value(QStringLiteral("position")).toList();
    const QVariantList focal = state.value(QStringLiteral("focalPoint")).toList();
    const QVariantList up = state.value(QStringLiteral("viewUp")).toList();
    const double scale = state.value(QStringLiteral("parallelScale")).toDouble();
    const double angle = state.value(QStringLiteral("viewAngle")).toDouble();
    if (position.size() != 3 || focal.size() != 3 || up.size() != 3 ||
        !std::isfinite(scale) || scale <= 0.0 || !std::isfinite(angle) || angle <= 0.0 || angle >= 180.0)
    {
        return false;
    }
    double p[3];
    double f[3];
    double u[3];
    double distance = 0.0;
    double upLength = 0.0;
    for (int axis = 0; axis < 3; ++axis)
    {
        bool validP = false;
        bool validF = false;
        bool validU = false;
        p[axis] = position.at(axis).toDouble(&validP);
        f[axis] = focal.at(axis).toDouble(&validF);
        u[axis] = up.at(axis).toDouble(&validU);
        if (!validP || !validF || !validU || !std::isfinite(p[axis]) || !std::isfinite(f[axis]) || !std::isfinite(u[axis]))
        {
            return false;
        }
        distance += (p[axis] - f[axis]) * (p[axis] - f[axis]);
        upLength += u[axis] * u[axis];
    }
    if (!(distance > 0.0) || !(upLength > 0.0) || !std::isfinite(distance) || !std::isfinite(upLength))
    {
        return false;
    }
    camera->SetPosition(p);
    camera->SetFocalPoint(f);
    camera->SetViewUp(u);
    camera->SetParallelProjection(state.value(QStringLiteral("parallel")).toBool() ? 1 : 0);
    camera->SetParallelScale(scale);
    camera->SetViewAngle(angle);
    return true;
}

QVariantMap IDOSRenderView::presentationState() const
{
    QVariantMap state;
    state.insert(QStringLiteral("twoDimensional"), isTwoDimensional());
    state.insert(QStringLiteral("orientation"), static_cast<int>(orientation()));
    state.insert(QStringLiteral("background"), background().name(QColor::HexArgb));
    state.insert(QStringLiteral("axes"), axesVisible());
    state.insert(QStringLiteral("legend"), legendVisible());
    state.insert(QStringLiteral("cursorTracking"), cursorTrackingEnabled());
    state.insert(QStringLiteral("camera"), cameraState(m_savedCamera != nullptr ? m_savedCamera.GetPointer() : m_renderer->GetActiveCamera()));
    state.insert(QStringLiteral("homeCamera"), cameraState(m_homeCamera));
    state.insert(QStringLiteral("homeTwoDimensional"), m_homeTwoDimensional);
    state.insert(QStringLiteral("homeInitialized"), m_homeViewInitialized);
    if (m_toolBar != nullptr)
    {
        state.insert(QStringLiteral("toolBar"), m_toolBar->presentationState());
    }
    return state;
}

bool IDOSRenderView::restorePresentationState(const QVariantMap& state)
{
    const QColor color(state.value(QStringLiteral("background")).toString());
    const int direction = state.value(QStringLiteral("orientation")).toInt();
    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    if (!color.isValid() || direction < static_cast<int>(IDOSOrientation::Perspective) ||
        direction > static_cast<int>(IDOSOrientation::Isometric) ||
        !restoreCameraState(camera, state.value(QStringLiteral("camera")).toMap()))
    {
        return false;
    }
    unlinkCamera();
    setTwoDimensional(state.value(QStringLiteral("twoDimensional")).toBool());
    m_orientation = static_cast<IDOSOrientation>(direction);
    setBackground(color);
    setAxesVisible(state.value(QStringLiteral("axes")).toBool());
    setLegendVisible(state.value(QStringLiteral("legend")).toBool());
    setCursorTrackingEnabled(state.value(QStringLiteral("cursorTracking")).toBool());
    m_renderer->GetActiveCamera()->DeepCopy(camera);
    m_renderer->ResetCameraClippingRange();
    m_homeViewInitialized = state.value(QStringLiteral("homeInitialized")).toBool() &&
        restoreCameraState(m_homeCamera, state.value(QStringLiteral("homeCamera")).toMap());
    m_homeTwoDimensional = state.value(QStringLiteral("homeTwoDimensional")).toBool();
    if (m_toolBar != nullptr)
    {
        m_toolBar->restorePresentationState(state.value(QStringLiteral("toolBar")).toMap());
        positionToolBar();
    }
    render();
    emit viewStateChanged();
    return true;
}

void IDOSRenderView::onCameraInteraction()
{
    emit viewStateChanged();
}

bool IDOSRenderView::isCameraLinkedTo(const IDOSRenderView* view) const
{
    return view != nullptr && view != this && isCameraLinked() &&
        m_linkedCamera.GetPointer() == view->m_renderer->GetActiveCamera();
}
