#include <cmath>
#include <limits>

#include <QCursor>
#include <QDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QStringList>
#include <QTimer>
#include <QToolTip>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkAxisActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellPicker.h>
#include <vtkClipPolyData.h>
#include <vtkCommand.h>
#include <vtkCoordinate.h>
#include <vtkCursor2D.h>
#include <vtkCutter.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkDistanceWidget.h>
#include <vtkExtractCells.h>
#include <vtkExtractEdges.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkIdList.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkMapper.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPointPicker.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkUnstructuredGrid.h>

#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"

#include "idosrenderinteractiontool.h"

IDOSRenderInteractionTool::IDOSRenderInteractionTool(IDOSRenderView* view, Mode mode)
    :
    IDOSTool(view)
    , m_mode(mode)
    , m_promptLabel(new QLabel(view))
    , m_measurementRepresentation(vtkSmartPointer<vtkDistanceRepresentation2D>::New())
    , m_measurementWidget(vtkSmartPointer<vtkDistanceWidget>::New())
    , m_probeActor()
    , m_clipWidget(vtkSmartPointer<vtkImplicitPlaneWidget2>::New())
    , m_clipRepresentation(vtkSmartPointer<vtkImplicitPlaneRepresentation>::New())
    , m_clipPlane(vtkSmartPointer<vtkPlane>::New())
    , m_clipCallback(vtkSmartPointer<vtkCallbackCommand>::New())
    , m_clipObserverTag(0)
    , m_clippedMappers()
    , m_clipObjectId()
    , m_slicePreviewActor()
    , m_sliceDialog(nullptr)
    , m_sliceRenderWindow()
    , m_sliceObjectId()
    , m_slicePoints()
    , m_hasFirstPoint(false)
    , m_firstPoint{0.0, 0.0, 0.0}
{
    m_promptLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_promptLabel->setTextFormat(Qt::PlainText);
    m_promptLabel->setStyleSheet(QStringLiteral("QLabel { background: #F7F9FC; color: #334155; padding: 6px; border: 1px solid #D7DFE8; border-radius: 4px; }"));
    m_promptLabel->hide();
    m_measurementRepresentation->GetAxisProperty()->SetColor(1.0, 0.75, 0.1);
    m_measurementRepresentation->GetAxisProperty()->SetLineWidth(1.5);
    m_measurementRepresentation->SetLabelFormat("%.2f");
    m_measurementRepresentation->GetAxis()->TickVisibilityOff();
    m_measurementRepresentation->GetAxis()->LabelVisibilityOff();
    m_measurementRepresentation->GetAxis()->GetTitleTextProperty()->SetColor(1.0, 0.85, 0.2);
    m_measurementRepresentation->GetAxis()->UseFontSizeFromPropertyOn();
    m_measurementRepresentation->GetAxis()->GetTitleTextProperty()->SetFontSize(14);
    m_measurementRepresentation->GetAxis()->GetTitleTextProperty()->ShadowOn();
    vtkSmartPointer<vtkCursor2D> cursor = vtkSmartPointer<vtkCursor2D>::New();
    cursor->SetModelBounds(-4.0, 4.0, -4.0, 4.0, 0.0, 0.0);
    cursor->OutlineOff();
    cursor->PointOff();
    cursor->AxesOn();
    cursor->Update();
    vtkSmartPointer<vtkPointHandleRepresentation2D> handle =
        vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
    handle->SetCursorShape(cursor->GetOutput());
    handle->GetProperty()->SetColor(1.0, 0.85, 0.2);
    handle->GetProperty()->SetLineWidth(1.5);
    handle->GetSelectedProperty()->SetColor(1.0, 0.85, 0.2);
    handle->GetSelectedProperty()->SetLineWidth(1.5);
    m_measurementRepresentation->SetHandleRepresentation(handle);
    m_measurementRepresentation->InstantiateHandleRepresentation();
    m_measurementWidget->SetRepresentation(m_measurementRepresentation);
    m_measurementWidget->ProcessEventsOff();
    m_clipCallback->SetClientData(this);
    m_clipCallback->SetCallback(&IDOSRenderInteractionTool::onClipPlaneInteraction);
    m_clipRepresentation->SetPlaceFactor(1.0);
    m_clipWidget->SetRepresentation(m_clipRepresentation);
    m_clipWidget->SetInteractor(view->vtkWidget()->interactor());
    connect(view->scene(), &IDOSRenderScene::changed, this, &IDOSRenderInteractionTool::onSceneChanged);
    connect(view->scene(), &IDOSRenderScene::objectAboutToBeRemoved,
            this, &IDOSRenderInteractionTool::onSceneObjectAboutToBeRemoved);
}

IDOSRenderInteractionTool::~IDOSRenderInteractionTool()
{
    clearMeasurement();
    clearProbe();
    clearClippingPlane();
    clearSlicePreview();
    clearSliceProfileWindow();
    delete m_promptLabel;
}

QString IDOSRenderInteractionTool::exclusiveGroup() const
{
    return QStringLiteral("render-interaction");
}

void IDOSRenderInteractionTool::cancel()
{
    clearMeasurement();
    clearProbe();
    clearClippingPlane();
    clearSlicePreview();
    m_slicePoints.clear();
    m_hasFirstPoint = false;
    m_promptLabel->hide();
    QToolTip::hideText();
}

bool IDOSRenderInteractionTool::onEventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            cancel();
            QTimer::singleShot(0, this, &IDOSRenderInteractionTool::onDeactivateRequested);
            return true;
        }
        if (m_mode == Mode::SliceByPoints &&
            (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter))
        {
            executeSlice();
            return true;
        }
        if (m_mode == Mode::SliceByPoints && keyEvent->key() == Qt::Key_Backspace)
        {
            if (!m_slicePoints.isEmpty())
            {
                m_slicePoints.removeLast();
                updateSlicePreview();
                showPrompt(tr("Path points: %1. Click more points or press Enter to create the slice.")
                               .arg(m_slicePoints.size()));
            }
            return true;
        }
    }
    if (event->type() == QEvent::MouseButtonRelease)
    {
        if (m_mode == Mode::ClippingPlane)
        {
            return false;
        }
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        return mouseEvent->button() == Qt::LeftButton;
    }
    if (event->type() != QEvent::MouseButtonPress)
    {
        return false;
    }
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() != Qt::LeftButton)
    {
        return false;
    }
    IDOSRenderView* renderView = view();
    if (renderView == nullptr)
    {
        return false;
    }
    if (m_mode == Mode::ClippingPlane)
    {
        if (!m_clipObjectId.isEmpty())
        {
            return false;
        }
        const QString objectId = renderView->objectIdAt(mouseEvent->pos());
        IDOSRenderObject* object = renderView->scene()->object(objectId);
        if (object == nullptr || !object->isVisible())
        {
            showPrompt(tr("Click a visible 3D object to place the clipping plane."));
            return true;
        }
        m_clipObjectId = objectId;
        if (startClippingPlane())
        {
            showPrompt(tr("Drag the plane to clip the target object. Press Escape to restore it."));
        }
        else
        {
            m_clipObjectId.clear();
            showPrompt(tr("The target object cannot be clipped in this view."));
        }
        return true;
    }
    if (m_mode == Mode::SliceByPoints)
    {
        if (m_sliceObjectId.isEmpty())
        {
            const QString objectId = renderView->objectIdAt(mouseEvent->pos());
            IDOSRenderObject* object = renderView->scene()->object(objectId);
            if (object == nullptr || !object->isVisible())
            {
                showPrompt(tr("Click a visible 3D object to start the slice path."));
                return true;
            }
            m_sliceObjectId = objectId;
        }
        double worldPosition[3];
        if (pickSelectedObjectWorldPosition(mouseEvent->pos(), worldPosition))
        {
            addSlicePoint(worldPosition);
        }
        else
        {
            showPrompt(tr("Click the target object to add a path point."));
        }
        return true;
    }
    if (m_mode == Mode::Selection)
    {
        const QString objectId = renderView->objectIdAt(mouseEvent->pos());
        if (!objectId.isEmpty())
        {
            renderView->setSelection(QList<QString>{objectId});
            emit renderView->activated(objectId);
        }
        else
        {
            renderView->setSelection(QList<QString>());
        }
        return true;
    }
    if (m_mode == Mode::PointProbe)
    {
        performProbe(mouseEvent->pos());
        return true;
    }
    double worldPosition[3];
    if (!renderView->pickWorldPosition(mouseEvent->pos(), worldPosition))
    {
        showPrompt(tr("No model point was picked."));
        return true;
    }
    handlePickedPoint(worldPosition);
    return true;
}

void IDOSRenderInteractionTool::onActivated()
{
    m_hasFirstPoint = false;
    if (view() != nullptr)
    {
        view()->vtkWidget()->setFocus(Qt::OtherFocusReason);
    }
    if (m_mode == Mode::ClippingPlane)
    {
        m_clipObjectId.clear();
        showPrompt(tr("Click a visible 3D object to place the clipping plane."));
    }
    else if (m_mode == Mode::SliceByPoints)
    {
        m_slicePoints.clear();
        m_sliceObjectId.clear();
        clearSlicePreview();
        clearSliceProfileWindow();
        showPrompt(tr("Click a visible 3D object to start the slice path."));
    }
    else if (m_mode == Mode::PointZoom)
    {
        showPrompt(tr("Click a model point to center and zoom the view."));
    }
    else if (m_mode == Mode::PointProbe)
    {
        showPrompt(tr("Click a model cell to inspect its coordinates and attributes."));
    }
    else if (m_mode == Mode::Selection)
    {
        showPrompt(tr("Click a model object to select it."));
    }
    else
    {
        showPrompt(tr("Click two model points to measure their distance."));
    }
}

void IDOSRenderInteractionTool::onDeactivated()
{
    clearMeasurement();
    clearProbe();
    clearClippingPlane();
    clearSlicePreview();
    m_slicePoints.clear();
    m_hasFirstPoint = false;
    if (m_mode == Mode::Selection && view() != nullptr)
    {
        view()->setSelection(QList<QString>());
    }
    m_promptLabel->hide();
    QToolTip::hideText();
}

void IDOSRenderInteractionTool::showPrompt(const QString& text) const
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        if (m_mode == Mode::DistanceMeasurement || m_mode == Mode::PointProbe ||
            m_mode == Mode::ClippingPlane || m_mode == Mode::SliceByPoints)
        {
            m_promptLabel->setText(text);
            m_promptLabel->adjustSize();
            m_promptLabel->move(8, qMax(8, renderView->height() - m_promptLabel->height() - 8));
            m_promptLabel->show();
            m_promptLabel->raise();
            return;
        }
        QToolTip::showText(QCursor::pos(), text, renderView);
    }
}

void IDOSRenderInteractionTool::onClipPlaneInteraction(vtkObject* caller, unsigned long eventId,
                                                        void* clientData, void* callData)
{
    Q_UNUSED(caller)
    Q_UNUSED(eventId)
    Q_UNUSED(callData)
    IDOSRenderInteractionTool* tool = static_cast<IDOSRenderInteractionTool*>(clientData);
    if (tool != nullptr)
    {
        tool->updateClippingPlane();
    }
}

void IDOSRenderInteractionTool::onSceneChanged()
{
    if (!isActive())
    {
        return;
    }
    if (m_mode == Mode::ClippingPlane && !m_clipObjectId.isEmpty())
    {
        updateClippingPlane();
        if (m_clipWidget->GetEnabled() != 0)
        {
            m_clipWidget->SetEnabled(0);
            m_clipWidget->SetEnabled(1);
        }
    }
    else if (m_mode == Mode::SliceByPoints && !m_slicePoints.isEmpty())
    {
        updateSlicePreview();
    }
}

void IDOSRenderInteractionTool::onSceneObjectAboutToBeRemoved(const QString& id)
{
    if (id == m_clipObjectId)
    {
        clearClippingPlane();
        QTimer::singleShot(0, this, &IDOSRenderInteractionTool::onDeactivateRequested);
    }
    if (id == m_sliceObjectId)
    {
        clearSlicePreview();
        m_slicePoints.clear();
        m_sliceObjectId.clear();
        QTimer::singleShot(0, this, &IDOSRenderInteractionTool::onDeactivateRequested);
    }
}

bool IDOSRenderInteractionTool::startClippingPlane()
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr || renderView->isTwoDimensional())
    {
        return false;
    }
    if (m_clipObjectId.isEmpty() && renderView->selection().size() == 1)
    {
        m_clipObjectId = renderView->selection().first();
    }
    IDOSRenderObject* object = renderView->scene()->object(m_clipObjectId);
    if (object == nullptr || !object->isVisible() || object->provider() == nullptr)
    {
        return false;
    }
    const QList<vtkActor*> actorList = actors(object->provider());
    double bounds[6] = {std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    bool hasBounds = false;
    for (vtkActor* actor : actorList)
    {
        if (actor == nullptr || actor->GetVisibility() == 0)
        {
            continue;
        }
        double actorBounds[6];
        actor->GetBounds(actorBounds);
        for (int index = 0; index < 3; ++index)
        {
            bounds[index * 2] = qMin(bounds[index * 2], actorBounds[index * 2]);
            bounds[index * 2 + 1] = qMax(bounds[index * 2 + 1], actorBounds[index * 2 + 1]);
        }
        hasBounds = true;
    }
    if (!hasBounds)
    {
        return false;
    }

    m_clipObjectId = object->id();
    m_clipRepresentation->PlaceWidget(bounds);
    m_clipRepresentation->SetOrigin((bounds[0] + bounds[1]) * 0.5,
                                     (bounds[2] + bounds[3]) * 0.5,
                                     (bounds[4] + bounds[5]) * 0.5);
    m_clipRepresentation->SetNormal(0.0, 0.0, 1.0);
    m_clipWidget->SetCurrentRenderer(renderView->vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer());
    m_clipWidget->SetInteractor(renderView->vtkWidget()->interactor());
    m_clipObserverTag = m_clipWidget->AddObserver(vtkCommand::InteractionEvent, m_clipCallback);
    m_clipRepresentation->GetPlane(m_clipPlane);
    m_clipWidget->SetEnabled(1);
    updateClippingPlane();
    return true;
}

void IDOSRenderInteractionTool::updateClippingPlane()
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr || m_clipObjectId.isEmpty())
    {
        return;
    }
    if (m_clipRepresentation != nullptr)
    {
        m_clipRepresentation->GetPlane(m_clipPlane);
    }
    for (vtkSmartPointer<vtkMapper>& mapper : m_clippedMappers)
    {
        if (mapper != nullptr)
        {
            mapper->RemoveClippingPlane(m_clipPlane);
            mapper->Modified();
        }
    }
    m_clippedMappers.clear();
    IDOSRenderObject* object = renderView->scene()->object(m_clipObjectId);
    if (object != nullptr && object->provider() != nullptr)
    {
        const QList<vtkActor*> actorList = actors(object->provider());
        for (vtkActor* actor : actorList)
        {
            vtkMapper* mapper = actor != nullptr ? actor->GetMapper() : nullptr;
            if (mapper != nullptr && !m_clippedMappers.contains(mapper))
            {
                mapper->AddClippingPlane(m_clipPlane);
                mapper->Modified();
                m_clippedMappers.append(vtkSmartPointer<vtkMapper>(mapper));
            }
        }
    }
    renderView->render();
}

void IDOSRenderInteractionTool::clearClippingPlane()
{
    if (m_clipWidget != nullptr)
    {
        m_clipWidget->SetEnabled(0);
        if (m_clipObserverTag != 0)
        {
            m_clipWidget->RemoveObserver(m_clipObserverTag);
            m_clipObserverTag = 0;
        }
    }
    for (vtkSmartPointer<vtkMapper>& mapper : m_clippedMappers)
    {
        if (mapper != nullptr)
        {
            mapper->RemoveClippingPlane(m_clipPlane);
            mapper->Modified();
        }
    }
    m_clippedMappers.clear();
    m_clipObjectId.clear();
    if (view() != nullptr)
    {
        view()->render();
    }
}

bool IDOSRenderInteractionTool::pickSelectedObjectWorldPosition(const QPoint& position,
                                                                 double worldPosition[3]) const
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr || worldPosition == nullptr || m_sliceObjectId.isEmpty() ||
        renderView->vtkWidget()->width() <= 0 || renderView->vtkWidget()->height() <= 0)
    {
        return false;
    }
    IDOSRenderObject* object = renderView->scene()->object(m_sliceObjectId);
    if (object == nullptr || !object->isVisible() || object->provider() == nullptr)
    {
        return false;
    }
    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);
    picker->PickFromListOn();
    for (vtkActor* actor : actors(object->provider()))
    {
        if (actor != nullptr && actor->GetVisibility() != 0)
        {
            picker->AddPickList(actor);
        }
    }
    vtkRenderWindow* window = renderView->vtkWidget()->renderWindow();
    const int* size = window->GetSize();
    const double x = static_cast<double>(position.x()) * size[0] / renderView->vtkWidget()->width();
    const double y = size[1] - 1.0 - static_cast<double>(position.y()) * size[1] /
        renderView->vtkWidget()->height();
    vtkRenderer* renderer = window->GetRenderers()->GetFirstRenderer();
    if (renderer == nullptr || picker->Pick(x, y, 0.0, renderer) == 0)
    {
        return false;
    }
    picker->GetPickPosition(worldPosition);
    return true;
}

void IDOSRenderInteractionTool::addSlicePoint(const double worldPosition[3])
{
    if (worldPosition == nullptr)
    {
        return;
    }
    if (!m_slicePoints.isEmpty())
    {
        const std::array<double, 3>& previous = m_slicePoints.last();
        const double dx = worldPosition[0] - previous[0];
        const double dy = worldPosition[1] - previous[1];
        if (dx * dx + dy * dy < 1.0e-8)
        {
            return;
        }
    }
    m_slicePoints.append({worldPosition[0], worldPosition[1], worldPosition[2]});
    updateSlicePreview();
    showPrompt(tr("Path points: %1. Click more points or press Enter to create the slice.")
                   .arg(m_slicePoints.size()));
}

void IDOSRenderInteractionTool::updateSlicePreview()
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr || m_slicePoints.isEmpty())
    {
        clearSlicePreview();
        return;
    }
    vtkRenderer* renderer = renderView->vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer();
    if (renderer == nullptr)
    {
        return;
    }
    if (m_slicePreviewActor != nullptr)
    {
        renderer->RemoveActor(m_slicePreviewActor);
    }
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    for (int index = 0; index < m_slicePoints.size(); ++index)
    {
        const std::array<double, 3>& point = m_slicePoints.at(index);
        points->InsertNextPoint(point.data());
        vertices->InsertNextCell(1);
        vertices->InsertCellPoint(index);
        if (index > 0)
        {
            const vtkIdType ids[2] = {index - 1, index};
            lines->InsertNextCell(2, ids);
        }
    }
    vtkSmartPointer<vtkPolyData> path = vtkSmartPointer<vtkPolyData>::New();
    path->SetPoints(points);
    path->SetVerts(vertices);
    path->SetLines(lines);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(path);
    m_slicePreviewActor = vtkSmartPointer<vtkActor>::New();
    m_slicePreviewActor->SetMapper(mapper);
    m_slicePreviewActor->GetProperty()->SetColor(1.0, 0.8, 0.1);
    m_slicePreviewActor->GetProperty()->SetLineWidth(2.0);
    m_slicePreviewActor->GetProperty()->SetPointSize(9.0);
    m_slicePreviewActor->PickableOff();
    renderer->AddActor(m_slicePreviewActor);
    renderView->render();
}

void IDOSRenderInteractionTool::clearSlicePreview()
{
    if (m_slicePreviewActor != nullptr && view() != nullptr)
    {
        vtkRenderer* renderer = view()->vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer();
        if (renderer != nullptr)
        {
            renderer->RemoveActor(m_slicePreviewActor);
        }
        view()->render();
    }
    m_slicePreviewActor = nullptr;
}

void IDOSRenderInteractionTool::executeSlice()
{
    IDOSRenderView* renderView = view();
    IDOSRenderObject* object = renderView != nullptr ? renderView->scene()->object(m_sliceObjectId) : nullptr;
    if (renderView == nullptr || object == nullptr || object->provider() == nullptr || m_slicePoints.size() < 2)
    {
        showPrompt(tr("Add at least two path points on the selected object."));
        return;
    }

    vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
    const QList<vtkActor*> actorList = actors(object->provider());
    for (vtkActor* actor : actorList)
    {
        vtkMapper* mapper = actor != nullptr ? actor->GetMapper() : nullptr;
        vtkDataSet* source = mapper != nullptr
            ? vtkDataSet::SafeDownCast(mapper->GetInputDataObject(0, 0)) : nullptr;
        if (source == nullptr)
        {
            continue;
        }
        vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
        transform->SetMatrix(actor->GetMatrix());
        vtkSmartPointer<vtkTransformFilter> transformed = vtkSmartPointer<vtkTransformFilter>::New();
        transformed->SetInputData(source);
        transformed->SetTransform(transform);
        transformed->Update();
        vtkDataSet* worldData = vtkDataSet::SafeDownCast(transformed->GetOutputDataObject(0));
        if (worldData == nullptr)
        {
            continue;
        }

        double accumulatedDistance = 0.0;
        for (int segmentIndex = 0; segmentIndex + 1 < m_slicePoints.size(); ++segmentIndex)
        {
            const std::array<double, 3>& first = m_slicePoints.at(segmentIndex);
            const std::array<double, 3>& second = m_slicePoints.at(segmentIndex + 1);
            const double dx = second[0] - first[0];
            const double dy = second[1] - first[1];
            const double length = std::sqrt(dx * dx + dy * dy);
            if (length <= 1.0e-6)
            {
                continue;
            }
            const double axis[3] = {dx / length, dy / length, 0.0};
            const double normal[3] = {-axis[1], axis[0], 0.0};
            vtkSmartPointer<vtkPlane> cutPlane = vtkSmartPointer<vtkPlane>::New();
            cutPlane->SetOrigin(first.data());
            cutPlane->SetNormal(normal);
            vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();
            cutter->SetInputData(worldData);
            cutter->SetCutFunction(cutPlane);
            cutter->GenerateCutScalarsOff();
            cutter->Update();

            vtkSmartPointer<vtkPlane> startPlane = vtkSmartPointer<vtkPlane>::New();
            startPlane->SetOrigin(first.data());
            startPlane->SetNormal(axis);
            vtkSmartPointer<vtkClipPolyData> startClip = vtkSmartPointer<vtkClipPolyData>::New();
            startClip->SetInputConnection(cutter->GetOutputPort());
            startClip->SetClipFunction(startPlane);
            startClip->InsideOutOff();
            vtkSmartPointer<vtkPlane> endPlane = vtkSmartPointer<vtkPlane>::New();
            endPlane->SetOrigin(second.data());
            endPlane->SetNormal(axis);
            vtkSmartPointer<vtkClipPolyData> endClip = vtkSmartPointer<vtkClipPolyData>::New();
            endClip->SetInputConnection(startClip->GetOutputPort());
            endClip->SetClipFunction(endPlane);
            endClip->InsideOutOn();
            endClip->Update();
            vtkPolyData* clipped = endClip->GetOutput();
            if (clipped == nullptr || clipped->GetNumberOfPoints() == 0 || clipped->GetNumberOfCells() == 0)
            {
                accumulatedDistance += length;
                continue;
            }

            vtkSmartPointer<vtkPolyData> flattened = vtkSmartPointer<vtkPolyData>::New();
            flattened->DeepCopy(clipped);
            vtkSmartPointer<vtkPoints> flattenedPoints = vtkSmartPointer<vtkPoints>::New();
            flattenedPoints->SetDataTypeToDouble();
            for (vtkIdType pointIndex = 0; pointIndex < clipped->GetNumberOfPoints(); ++pointIndex)
            {
                double point[3];
                clipped->GetPoint(pointIndex, point);
                const double distance = accumulatedDistance +
                    (point[0] - first[0]) * axis[0] + (point[1] - first[1]) * axis[1];
                flattenedPoints->InsertNextPoint(distance, point[2], 0.0);
            }
            flattened->SetPoints(flattenedPoints);
            append->AddInputData(flattened);
            accumulatedDistance += length;
        }
    }
    append->Update();
    vtkSmartPointer<vtkPolyData> profile = vtkSmartPointer<vtkPolyData>::New();
    profile->DeepCopy(append->GetOutput());
    if (profile->GetNumberOfPoints() == 0 || profile->GetNumberOfCells() == 0)
    {
        showPrompt(tr("No slice geometry was found along the selected path."));
        return;
    }
    clearSlicePreview();
    showSliceProfile(profile);
    showPrompt(tr("Slice profile created for %1.").arg(object->name()));
    QTimer::singleShot(0, this, &IDOSRenderInteractionTool::onDeactivateRequested);
}

void IDOSRenderInteractionTool::showSliceProfile(vtkPolyData* profile)
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr || profile == nullptr)
    {
        return;
    }
    clearSliceProfileWindow();
    QDialog* dialog = new QDialog(renderView, Qt::Window);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QObject::destroyed,
            this, &IDOSRenderInteractionTool::onSliceProfileWindowDestroyed);
    dialog->setObjectName(QStringLiteral("renderSliceProfileWindow"));
    dialog->setWindowTitle(tr("Slice Profile"));
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    QVTKOpenGLNativeWidget* widget = new QVTKOpenGLNativeWidget(dialog);
    widget->setObjectName(QStringLiteral("renderSliceProfileView"));
    m_sliceRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.12, 0.16, 0.21);
    m_sliceRenderWindow->AddRenderer(renderer);
    widget->setRenderWindow(m_sliceRenderWindow);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(profile);
    mapper->ScalarVisibilityOff();
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.96, 0.72, 0.2);
    actor->GetProperty()->SetLineWidth(2.0);
    renderer->AddActor(actor);
    renderer->GetActiveCamera()->ParallelProjectionOn();
    renderer->ResetCamera();
    renderer->ResetCameraClippingRange();
    layout->addWidget(widget);
    dialog->resize(1000, 700);
    m_sliceDialog = dialog;
    dialog->show();
    dialog->raise();
    m_sliceRenderWindow->Render();
}

void IDOSRenderInteractionTool::clearSliceProfileWindow()
{
    if (m_sliceDialog != nullptr)
    {
        m_sliceDialog->close();
    }
    m_sliceDialog = nullptr;
    m_sliceRenderWindow = nullptr;
}

void IDOSRenderInteractionTool::onSliceProfileWindowDestroyed(QObject* object)
{
    Q_UNUSED(object)
    m_sliceDialog = nullptr;
    m_sliceRenderWindow = nullptr;
}

void IDOSRenderInteractionTool::handlePickedPoint(const double worldPosition[3])
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr)
    {
        return;
    }
    if (m_mode == Mode::PointZoom)
    {
        renderView->zoomToWorldPosition(worldPosition);
        showPrompt(tr("View centered on the picked point."));
        QTimer::singleShot(0, this, &IDOSRenderInteractionTool::onDeactivateRequested);
        return;
    }
    if (!m_hasFirstPoint)
    {
        clearMeasurement();
        m_firstPoint[0] = worldPosition[0];
        m_firstPoint[1] = worldPosition[1];
        m_firstPoint[2] = worldPosition[2];
        m_hasFirstPoint = true;
        updateMeasurement(worldPosition);
        showPrompt(tr("Pick the second point."));
        return;
    }
    const double dx = worldPosition[0] - m_firstPoint[0];
    const double dy = worldPosition[1] - m_firstPoint[1];
    const double dz = worldPosition[2] - m_firstPoint[2];
    const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    updateMeasurement(worldPosition);
    m_hasFirstPoint = false;
    showPrompt(tr("Distance: %1").arg(distance, 0, 'f', 2));
}

void IDOSRenderInteractionTool::clearMeasurement()
{
    const bool wasEnabled = m_measurementWidget->GetEnabled() != 0;
    m_measurementWidget->SetEnabled(0);
    m_measurementWidget->SetInteractor(nullptr);
    if (wasEnabled && view() != nullptr)
    {
        view()->render();
    }
}

void IDOSRenderInteractionTool::updateMeasurement(const double worldPosition[3])
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr)
    {
        return;
    }
    double secondPoint[3] = {worldPosition[0], worldPosition[1], worldPosition[2]};
    m_measurementWidget->SetInteractor(renderView->vtkWidget()->interactor());
    m_measurementWidget->SetDefaultRenderer(renderView->vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer());
    m_measurementRepresentation->SetRenderer(renderView->vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer());
    m_measurementRepresentation->SetPoint1WorldPosition(m_firstPoint);
    m_measurementRepresentation->SetPoint2WorldPosition(secondPoint);
    m_measurementWidget->SetWidgetStateToManipulate();
    m_measurementWidget->SetEnabled(1);
    m_measurementWidget->ProcessEventsOff();
    m_measurementRepresentation->BuildRepresentation();
    renderView->render();
}

QString IDOSRenderInteractionTool::probeAttributes(vtkDataSetAttributes* attributes, vtkIdType tupleId) const
{
    QStringList lines;
    if (attributes == nullptr || tupleId < 0)
    {
        return QString();
    }
    for (int index = 0; index < attributes->GetNumberOfArrays() && lines.size() < 12; ++index)
    {
        vtkDataArray* array = attributes->GetArray(index);
        if (array == nullptr || tupleId >= array->GetNumberOfTuples() || array->GetName() == nullptr)
        {
            continue;
        }
        QStringList values;
        for (int component = 0; component < array->GetNumberOfComponents(); ++component)
        {
            values.append(QString::number(array->GetComponent(tupleId, component), 'g', 8));
        }
        lines.append(QString::fromUtf8(array->GetName()) + QStringLiteral(": ") + values.join(QStringLiteral(", ")));
    }
    return lines.join(QStringLiteral("\n"));
}

void IDOSRenderInteractionTool::clearProbe()
{
    if (m_probeActor != nullptr && view() != nullptr)
    {
        vtkRenderer* renderer = view()->vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer();
        renderer->RemoveViewProp(m_probeActor);
        view()->render();
    }
    m_probeActor = nullptr;
}

void IDOSRenderInteractionTool::performProbe(const QPoint& position)
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr || renderView->vtkWidget()->width() <= 0 || renderView->vtkWidget()->height() <= 0)
    {
        return;
    }
    clearProbe();
    m_promptLabel->hide();
    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);
    picker->PickFromListOn();
    for (IDOSRenderObject* object : renderView->scene()->objects())
    {
        if (object->isVisible() && object->provider() != nullptr)
        {
            for (vtkActor* actor : actors(object->provider()))
            {
                picker->AddPickList(actor);
            }
        }
    }
    vtkRenderWindow* window = renderView->vtkWidget()->renderWindow();
    vtkRenderer* renderer = window->GetRenderers()->GetFirstRenderer();
    const int* size = window->GetSize();
    const double x = static_cast<double>(position.x()) * size[0] / renderView->vtkWidget()->width();
    const double y = size[1] - 1.0 - static_cast<double>(position.y()) * size[1] / renderView->vtkWidget()->height();
    picker->Pick(x, y, 0.0, renderer);
    vtkDataSet* data = picker->GetDataSet();
    vtkActor* pickedActor = picker->GetActor();
    vtkIdType cellId = picker->GetCellId();
    vtkIdType pointId = picker->GetPointId();
    double world[3];
    picker->GetPickPosition(world);
    if (data == nullptr || (cellId < 0 && pointId < 0))
    {
        vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
        pointPicker->SetTolerance(0.01);
        pointPicker->PickFromListOn();
        for (IDOSRenderObject* object : renderView->scene()->objects())
        {
            if (object->isVisible() && object->provider() != nullptr)
            {
                for (vtkActor* actor : actors(object->provider()))
                {
                    pointPicker->AddPickList(actor);
                }
            }
        }
        if (!pointPicker->Pick(x, y, 0.0, renderer) || pointPicker->GetDataSet() == nullptr || pointPicker->GetPointId() < 0)
        {
            showPrompt(tr("No model point or cell was picked."));
            return;
        }
        data = pointPicker->GetDataSet();
        pickedActor = pointPicker->GetActor();
        cellId = -1;
        pointId = pointPicker->GetPointId();
        pointPicker->GetPickPosition(world);
    }
    QStringList lines;
    lines.append(tr("Point Probe"));
    for (IDOSRenderObject* object : renderView->scene()->objects())
    {
        if (object->provider() != nullptr && actors(object->provider()).contains(pickedActor))
        {
            lines.append(tr("Data: %1").arg(object->name()));
            break;
        }
    }
    if (cellId >= 0)
    {
        lines.append(tr("Cell: %1  Point: %2").arg(static_cast<qlonglong>(cellId)).arg(static_cast<qlonglong>(pointId)));
    }
    else
    {
        lines.append(tr("Point: %1").arg(static_cast<qlonglong>(pointId)));
    }
    lines.append(tr("X: %1, Y: %2, Z: %3").arg(world[0], 0, 'g', 7).arg(world[1], 0, 'g', 7).arg(world[2], 0, 'g', 7));
    const QString cellAttributes = probeAttributes(data->GetCellData(), cellId);
    const QString pointAttributes = probeAttributes(data->GetPointData(), pointId);
    if (!cellAttributes.isEmpty())
    {
        lines.append(tr("Cell attributes:"));
        lines.append(cellAttributes);
    }
    if (!pointAttributes.isEmpty())
    {
        lines.append(tr("Point attributes:"));
        lines.append(pointAttributes);
    }
    vtkSmartPointer<vtkPolyData> outline;
    if (cellId >= 0)
    {
        vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
        ids->InsertNextId(cellId);
        vtkSmartPointer<vtkExtractCells> extract = vtkSmartPointer<vtkExtractCells>::New();
        extract->SetInputData(data);
        extract->SetCellList(ids);
        extract->Update();
        vtkSmartPointer<vtkExtractEdges> edges = vtkSmartPointer<vtkExtractEdges>::New();
        edges->SetInputData(extract->GetOutput());
        edges->Update();
        outline = edges->GetOutput();
    }
    else
    {
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        points->InsertNextPoint(data->GetPoint(pointId));
        outline = vtkSmartPointer<vtkPolyData>::New();
        outline->SetPoints(points);
        const vtkIdType id = 0;
        vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
        vertices->InsertNextCell(1, &id);
        outline->SetVerts(vertices);
    }
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(pickedActor->GetMatrix());
    vtkSmartPointer<vtkTransformPolyDataFilter> transformed = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformed->SetInputData(outline);
    transformed->SetTransform(transform);
    transformed->Update();
    vtkSmartPointer<vtkPolyData> visual = vtkSmartPointer<vtkPolyData>::New();
    visual->DeepCopy(transformed->GetOutput());
    vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetCoordinateSystemToWorld();
    vtkSmartPointer<vtkPolyDataMapper2D> mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    mapper->SetInputData(visual);
    mapper->SetTransformCoordinate(coordinate);
    mapper->TransformCoordinateUseDoubleOn();
    mapper->ScalarVisibilityOff();
    m_probeActor = vtkSmartPointer<vtkActor2D>::New();
    m_probeActor->SetMapper(mapper);
    m_probeActor->GetProperty()->SetColor(1.0, 0.85, 0.1);
    m_probeActor->GetProperty()->SetLineWidth(3.0);
    m_probeActor->GetProperty()->SetPointSize(10.0);
    m_probeActor->GetProperty()->SetDisplayLocationToForeground();
    m_probeActor->PickableOff();
    renderer->AddViewProp(m_probeActor);
    showPrompt(lines.join(QStringLiteral("\n")));
    const QPoint anchor = renderView->vtkWidget()->mapTo(renderView, position);
    m_promptLabel->move(qBound(8, anchor.x() + 14, qMax(8, renderView->width() - m_promptLabel->width() - 8)),
                        qBound(8, anchor.y() + 14, qMax(8, renderView->height() - m_promptLabel->height() - 8)));
    renderView->render();
}

void IDOSRenderInteractionTool::onDeactivateRequested()
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr && isActive())
    {
        renderView->disableTool(this);
    }
}

QList<vtkActor*> IDOSRenderInteractionTool::actors(const IDOSRenderProvider* provider)
{
    QList<vtkActor*> result;
    if (provider == nullptr)
    {
        return result;
    }
    for (vtkProp* prop : provider->props())
    {
        if (prop != nullptr && vtkActor::SafeDownCast(prop) != nullptr)
        {
            result.append(vtkActor::SafeDownCast(prop));
        }
    }
    return result;
}
