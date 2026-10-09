#include <QEvent>
#include <QHash>
#include <QList>
#include <QMouseEvent>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QVBoxLayout>

#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkHexahedron.h>
#include <vtkIdList.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPropPicker.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStringOutputWindow.h>
#include <vtkUnstructuredGrid.h>

#include "idosrendermesh.h"
#include "idosrenderscene.h"
#include "idoswellrenderobject.h"

#include "idosrenderview.h"

vtkSmartPointer<vtkActor> IDOSRenderView::createMeshActor(const IDOSRenderMesh* mesh)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    const QVector<QVector3D>& renderPoints = mesh->points();
    for (int pointIndex = 0; pointIndex < renderPoints.size(); ++pointIndex)
    {
        const QVector3D& point = renderPoints.at(pointIndex);
        points->InsertNextPoint(point.x(), point.y(), point.z());
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);

    const QVector<int>& hexahedra = mesh->hexahedra();
    for (int cellIndex = 0; cellIndex + 7 < hexahedra.size(); cellIndex += 8)
    {
        vtkIdType ids[8];
        for (int corner = 0; corner < 8; ++corner)
        {
            ids[corner] = static_cast<vtkIdType>(hexahedra.at(cellIndex + corner));
        }
        grid->InsertNextCell(VTK_HEXAHEDRON, 8, ids);
    }

    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(grid);
    applyCellScalars(grid, mapper, mesh);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    if (mesh->hasCellScalars())
    {
        actor->GetProperty()->SetRepresentationToSurface();
        actor->GetProperty()->EdgeVisibilityOff();
        actor->GetProperty()->SetInterpolationToFlat();
    }
    else
    {
        actor->GetProperty()->SetRepresentationToSurface();
        actor->GetProperty()->SetColor(0.35, 0.72, 1.0);
        actor->GetProperty()->SetInterpolationToFlat();
    }
    actor->GetProperty()->SetLineWidth(1.0);
    actor->SetVisibility(mesh->visible());
    m_activeActor = actor;
    m_activeGrid = grid;
    m_activeMapper = mapper;
    return actor;
}

void IDOSRenderView::applyCellScalars(vtkUnstructuredGrid* grid, vtkDataSetMapper* mapper,
                                      const IDOSRenderMesh* mesh) const
{
    if (!mesh->hasCellScalars())
    {
        mapper->ScalarVisibilityOff();
        return;
    }

    vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
    scalars->SetName(mesh->cellScalarName().toUtf8().constData());
    const QVector<double>& values = mesh->cellScalars();
    double minimumValue = values.first();
    double maximumValue = values.first();
    for (int index = 0; index < values.size(); ++index)
    {
        const double value = values.at(index);
        scalars->InsertNextValue(value);
        if (value < minimumValue)
        {
            minimumValue = value;
        }
        if (value > maximumValue)
        {
            maximumValue = value;
        }
    }
    if (minimumValue == maximumValue)
    {
        maximumValue = minimumValue + 1.0;
    }

    grid->GetCellData()->SetScalars(scalars);

    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetHueRange(0.667, 0.0);
    lookupTable->SetSaturationRange(0.85, 0.85);
    lookupTable->SetValueRange(0.95, 0.95);
    lookupTable->SetRange(minimumValue, maximumValue);
    lookupTable->Build();

    mapper->SetLookupTable(lookupTable);
    mapper->SetScalarRange(minimumValue, maximumValue);
    mapper->SetScalarModeToUseCellData();
    mapper->SetInterpolateScalarsBeforeMapping(false);
    mapper->ScalarVisibilityOn();
}

vtkSmartPointer<vtkActor> IDOSRenderView::createWellHeadActor(const IDOSWellRenderObject* well)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    const QVector3D wellHeadPosition = well->wellHeadPosition();
    const vtkIdType wellHeadId = points->InsertNextPoint(
        wellHeadPosition.x(), wellHeadPosition.y(), wellHeadPosition.z());

    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vertices->InsertNextCell(1, &wellHeadId);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetVerts(vertices);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    applyWellStyle(actor, well);
    actor->GetProperty()->SetPointSize(12.0);
    actor->SetVisibility(well->visible());
    return actor;
}

vtkSmartPointer<vtkActor> IDOSRenderView::createWellTrajectoryActor(const IDOSWellRenderObject* well)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    const QVector<QVector3D>& wellPoints = well->points();
    for (int pointIndex = 0; pointIndex < wellPoints.size(); ++pointIndex)
    {
        const QVector3D& point = wellPoints.at(pointIndex);
        points->InsertNextPoint(point.x(), point.y(), point.z());
    }

    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    for (int pointIndex = 0; pointIndex < wellPoints.size(); ++pointIndex)
    {
        pointIds->InsertNextId(pointIndex);
    }

    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(pointIds);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    applyWellStyle(actor, well);
    actor->GetProperty()->SetLineWidth(2.5);
    actor->SetVisibility(well->visible());
    return actor;
}

void IDOSRenderView::applyWellStyle(vtkActor* actor, const IDOSWellRenderObject* well) const
{
    if (well->isInjector())
    {
        actor->GetProperty()->SetColor(0.2, 0.9, 0.25);
        return;
    }

    actor->GetProperty()->SetColor(0.95, 0.2, 0.15);
}

void IDOSRenderView::configureVtkOutputWindow()
{
    vtkStringOutputWindow* outputWindow = vtkStringOutputWindow::New();
    vtkOutputWindow::SetInstance(outputWindow);
    outputWindow->Delete();
}

void IDOSRenderView::clearActivePipeline()
{
    m_activeActor = nullptr;
    m_activeGrid = nullptr;
    m_activeMapper = nullptr;
}

IDOSRenderView::IDOSRenderView(QWidget* parent)
    :
    QWidget(parent)
    , m_vtkWidget(nullptr)
    , m_renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New())
    , m_renderer(vtkSmartPointer<vtkRenderer>::New())
    , m_axesActor(vtkSmartPointer<vtkAxesActor>::New())
    , m_orientationMarker(vtkSmartPointer<vtkOrientationMarkerWidget>::New())
    , m_activeActor(nullptr)
    , m_activeGrid(nullptr)
    , m_activeMapper(nullptr)
    , m_scene(new IDOSRenderScene())
    , m_actorObjectIds()
    , m_pressedPosition()
    , m_highlightedObjectId()
    , m_ownsScene(true)
{
    configureVtkOutputWindow();
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    m_vtkWidget->setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_vtkWidget);

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

bool IDOSRenderView::setCellScalars(const QString& name, const QVector<double>& values)
{
    if (m_scene == nullptr)
    {
        return false;
    }
    const QList<IDOSRenderObject*>& objects = m_scene->objects();
    if (objects.size() != 1)
    {
        return false;
    }

    IDOSRenderMesh* mesh = dynamic_cast<IDOSRenderMesh*>(objects.first());
    if (mesh == nullptr || m_activeGrid == nullptr || m_activeMapper == nullptr)
    {
        return false;
    }

    QVector<double> cellScalars;
    const QVector<int>& globalIndices = mesh->cellGlobalIndices();
    cellScalars.reserve(globalIndices.size());
    for (int index = 0; index < globalIndices.size(); ++index)
    {
        const int globalIndex = globalIndices.at(index);
        if (globalIndex >= 0 && globalIndex < values.size())
        {
            cellScalars.append(values.at(globalIndex));
        }
        else
        {
            cellScalars.append(0.0);
        }
    }

    mesh->setCellScalars(name, cellScalars);
    applyCellScalars(m_activeGrid, m_activeMapper, mesh);
    m_activeGrid->Modified();
    m_activeMapper->Modified();
    m_renderWindow->Render();
    return true;
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

void IDOSRenderView::resetCamera()
{
    m_renderer->ResetCamera();
    m_renderWindow->Render();
}

bool IDOSRenderView::setViewPreset(ViewPreset preset)
{
    double bounds[6];
    m_renderer->ComputeVisiblePropBounds(bounds);
    if (bounds[0] > bounds[1] || bounds[2] > bounds[3] || bounds[4] > bounds[5])
    {
        return false;
    }

    double direction[3] = {0.0, 0.0, 0.0};
    double viewUp[3] = {0.0, 0.0, 1.0};
    switch (preset)
    {
    case ViewPreset::Front:
        direction[1] = -1.0;
        break;
    case ViewPreset::Back:
        direction[1] = 1.0;
        break;
    case ViewPreset::Left:
        direction[0] = -1.0;
        break;
    case ViewPreset::Right:
        direction[0] = 1.0;
        break;
    case ViewPreset::Top:
        direction[2] = 1.0;
        viewUp[1] = 1.0;
        viewUp[2] = 0.0;
        break;
    case ViewPreset::Bottom:
        direction[2] = -1.0;
        viewUp[1] = -1.0;
        viewUp[2] = 0.0;
        break;
    case ViewPreset::Isometric:
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
    rebuildActors();
}

bool IDOSRenderView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_vtkWidget)
    {
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
    clearActivePipeline();
    m_actorObjectIds.clear();

    if (m_scene == nullptr)
    {
        m_renderWindow->Render();
        return;
    }

    const QList<IDOSRenderObject*>& objects = m_scene->objects();
    for (int index = 0; index < objects.size(); ++index)
    {
        IDOSRenderObject* object = objects.at(index);
        IDOSRenderMesh* mesh = dynamic_cast<IDOSRenderMesh*>(object);
        if (mesh != nullptr)
        {
            m_renderer->AddActor(createMeshActor(mesh));
            continue;
        }
        IDOSWellRenderObject* well = dynamic_cast<IDOSWellRenderObject*>(object);
        if (well != nullptr)
        {
            const bool highlighted = well->id() == m_highlightedObjectId;
            if (well->hasWellHead())
            {
                vtkSmartPointer<vtkActor> wellHeadActor = createWellHeadActor(well);
                if (highlighted)
                {
                    wellHeadActor->GetProperty()->SetColor(1.0, 0.95, 0.15);
                    wellHeadActor->GetProperty()->SetPointSize(18.0);
                }
                m_actorObjectIds.insert(wellHeadActor, well->id());
                m_renderer->AddActor(wellHeadActor);
            }
            if (well->pointCount() >= 2)
            {
                vtkSmartPointer<vtkActor> trajectoryActor = createWellTrajectoryActor(well);
                if (highlighted)
                {
                    trajectoryActor->GetProperty()->SetColor(1.0, 0.95, 0.15);
                    trajectoryActor->GetProperty()->SetLineWidth(5.0);
                }
                m_actorObjectIds.insert(trajectoryActor, well->id());
                m_renderer->AddActor(trajectoryActor);
            }
        }
    }

    m_renderWindow->Render();
}


