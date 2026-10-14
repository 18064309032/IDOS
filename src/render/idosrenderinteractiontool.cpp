#include <cmath>

#include <QCursor>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QStringList>
#include <QTimer>
#include <QToolTip>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkAxisActor2D.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellPicker.h>
#include <vtkCoordinate.h>
#include <vtkCursor2D.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkDistanceWidget.h>
#include <vtkExtractCells.h>
#include <vtkExtractEdges.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPointPicker.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkUnstructuredGrid.h>

#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "providers/idosrenderprovider.h"

#include "idosrenderinteractiontool.h"

IDOSRenderInteractionTool::IDOSRenderInteractionTool(IDOSRenderView* view, Mode mode)
    :
    IDOSTool(view)
    , m_mode(mode)
    , m_promptLabel(new QLabel(view))
    , m_measurementRepresentation(vtkSmartPointer<vtkDistanceRepresentation2D>::New())
    , m_measurementWidget(vtkSmartPointer<vtkDistanceWidget>::New())
    , m_probeActor()
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
}

IDOSRenderInteractionTool::~IDOSRenderInteractionTool()
{
    clearMeasurement();
    clearProbe();
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
    }
    if (event->type() == QEvent::MouseButtonRelease)
    {
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
    if (m_mode == Mode::PointZoom)
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
    if (m_mode == Mode::Selection && view() != nullptr)
    {
        view()->setSelection(QList<QString>());
    }
    clearMeasurement();
    clearProbe();
    m_hasFirstPoint = false;
    m_promptLabel->hide();
    QToolTip::hideText();
}

void IDOSRenderInteractionTool::showPrompt(const QString& text) const
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        if (m_mode == Mode::DistanceMeasurement || m_mode == Mode::PointProbe)
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
        if (object->visible() && object->provider() != nullptr)
        {
            for (vtkActor* actor : object->provider()->actors())
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
            if (object->visible() && object->provider() != nullptr)
            {
                for (vtkActor* actor : object->provider()->actors())
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
        if (object->provider() != nullptr && object->provider()->actors().contains(pickedActor))
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
