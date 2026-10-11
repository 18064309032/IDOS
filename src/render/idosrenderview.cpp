#include <QAction>
#include <QContextMenuEvent>
#include <QEasingCurve>
#include <QEvent>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHash>
#include <QInputDialog>
#include <QList>
#include <QMenu>
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

#include "idosproject.h"
#include "idosrenderguiprovider.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderregistry.h"
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
    , m_scene(new IDOSRenderScene(this))
    , m_actorObjectIds()
    , m_pressedPosition()
    , m_selection()
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

    double focalPoint[3] = {(bounds[0] + bounds[1]) * 0.5, (bounds[2] + bounds[3]) * 0.5,
                            (bounds[4] + bounds[5]) * 0.5};
    vtkCamera* camera = m_renderer->GetActiveCamera();
    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(focalPoint[0] + direction[0], focalPoint[1] + direction[1], focalPoint[2] + direction[2]);
    camera->SetViewUp(viewUp);
    camera->OrthogonalizeViewUp();
    m_renderer->ResetCamera();
    m_renderWindow->Render();
    return true;
}

bool IDOSRenderView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_vtkWidget)
    {
        if (event->type() == QEvent::ContextMenu)
        {
            QContextMenuEvent* context = static_cast<QContextMenuEvent*>(event);
            IDOSRenderObject* object = context->reason() == QContextMenuEvent::Keyboard
                                          ? (!selectedObjects().isEmpty() ? selectedObjects().first() : nullptr)
                                          : pick(context->pos());
            QMenu* menu = createMenu(object != nullptr ? object->id() : QString());
            menu->exec(context->globalPos());
            delete menu;
            m_menuId.clear();
            return true;
        }
        if (event->type() == QEvent::Resize)
        {
            m_flashOverlay->setGeometry(m_vtkWidget->rect());
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
                activateObjectAt(mouseEvent->pos());
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
    if (valid == m_selection)
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
}

void IDOSRenderView::render()
{
    m_renderWindow->Render();
}

void IDOSRenderView::fit(const QList<QString>& ids)
{
    double bounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    bool found = false;
    for (const QString& id : ids)
    {
        IDOSRenderObject* object = m_scene->object(id);
        if (object == nullptr)
        {
            continue;
        }
        const QList<vtkActor*> actors = object->provider()->actors();
        for (vtkActor* actor : actors)
        {
            const double* actorBounds = actor->GetBounds();
            if (actorBounds == nullptr || actorBounds[0] > actorBounds[1])
            {
                continue;
            }
            for (int axis = 0; axis < 3; ++axis)
            {
                bounds[axis * 2] = found ? qMin(bounds[axis * 2], actorBounds[axis * 2]) : actorBounds[axis * 2];
                bounds[axis * 2 + 1] =
                    found ? qMax(bounds[axis * 2 + 1], actorBounds[axis * 2 + 1]) : actorBounds[axis * 2 + 1];
            }
            found = true;
        }
    }
    if (found)
    {
        m_renderer->ResetCamera(bounds);
        render();
    }
}

void IDOSRenderView::setRegistry(IDOSRenderRegistry* registry)
{
    m_registry = registry;
}

void IDOSRenderView::setProject(IDOSProject* project)
{
    m_project = project;
}

IDOSProject* IDOSRenderView::project() const
{
    return m_project.data();
}

IDOSRenderObject* IDOSRenderView::menuObject() const
{
    return m_scene->object(m_menuId);
}

QList<IDOSRenderObject*> IDOSRenderView::selectedObjects() const
{
    QList<IDOSRenderObject*> result;
    for (const QString& id : m_selection)
    {
        IDOSRenderObject* object = m_scene->object(id);
        if (object != nullptr)
        {
            result.append(object);
        }
    }
    return result;
}

QMenu* IDOSRenderView::createMenu(const QString& objectId)
{
    m_menuId = objectId;
    QMenu* menu = new QMenu(this);
    IDOSRenderObject* object = menuObject();
    if (object != nullptr)
    {
        QAction* fitAction = menu->addAction(tr("Fit to Object"));
        fitAction->setData(objectId);
        connect(fitAction, &QAction::triggered, this, &IDOSRenderView::onFitObject);
        QAction* opacityAction = menu->addAction(tr("Opacity..."));
        opacityAction->setData(objectId);
        connect(opacityAction, &QAction::triggered, this, &IDOSRenderView::onOpacity);
        QAction* hideAction = menu->addAction(tr("Hide"));
        hideAction->setData(objectId);
        connect(hideAction, &QAction::triggered, this, &IDOSRenderView::onHideObject);
    }
    if (m_registry != nullptr)
    {
        QList<QPointer<IDOSRenderGuiProvider>> providers;
        for (IDOSRenderGuiProvider* provider : m_registry->guiProviders())
        {
            providers.append(provider);
        }
        for (const QPointer<IDOSRenderGuiProvider>& provider : providers)
        {
            if (provider != nullptr && provider->supports(menuObject()))
            {
                provider->populateMenu(menu, this);
            }
        }
    }
    QAction* reset = menu->addAction(tr("Fit All"));
    connect(reset, &QAction::triggered, this, &IDOSRenderView::onResetCamera);
    emit contextMenuAboutToShow(menu);
    return menu;
}

IDOSRenderObject* IDOSRenderView::actionObject() const
{
    QAction* action = qobject_cast<QAction*>(sender());
    return action != nullptr ? m_scene->object(action->data().toString()) : nullptr;
}

void IDOSRenderView::onFitObject()
{
    IDOSRenderObject* object = actionObject();
    if (object != nullptr)
    {
        fit(QList<QString>{object->id()});
    }
}

void IDOSRenderView::onOpacity()
{
    IDOSRenderObject* object = actionObject();
    if (object == nullptr)
    {
        return;
    }
    const QString id = object->id();
    bool accepted = false;
    const double opacity = QInputDialog::getDouble(this, tr("Opacity"), tr("Opacity:"),
                                                  object->opacity(), 0.0, 1.0, 2, &accepted);
    object = m_scene->object(id);
    if (accepted && object != nullptr)
    {
        object->setOpacity(opacity);
        m_scene->notifyChanged(id);
    }
}

void IDOSRenderView::onHideObject()
{
    IDOSRenderObject* object = actionObject();
    if (object != nullptr)
    {
        object->setVisible(false);
        m_scene->notifyChanged(object->id());
    }
}

void IDOSRenderView::onResetCamera()
{
    resetCamera();
}
