#include "idosrenderview.h"
#include "idosrendermesh.h"
#include "idosrenderscene.h"
#include "idoswellrenderobject.h"

#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
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
#include <vtkRenderer.h>
#include <vtkUnstructuredGrid.h>

class IDOSRenderViewPrivate
{
  public:
    IDOSRenderViewPrivate();
    ~IDOSRenderViewPrivate();

    vtkSmartPointer<vtkActor> createMeshActor(const IDOSRenderMesh* mesh);
    vtkSmartPointer<vtkActor> createWellActor(const IDOSWellRenderObject* well);
    void applyCellScalars(vtkUnstructuredGrid* grid, vtkDataSetMapper* mapper, const IDOSRenderMesh* mesh) const;
    void clearActivePipeline();

    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkAxesActor> axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarker;
    vtkSmartPointer<vtkActor> activeActor;
    vtkSmartPointer<vtkUnstructuredGrid> activeGrid;
    vtkSmartPointer<vtkDataSetMapper> activeMapper;
    IDOSRenderScene* scene;
    bool ownsScene;
};

IDOSRenderViewPrivate::IDOSRenderViewPrivate()
    : vtkWidget(nullptr)
    , renderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New())
    , renderer(vtkSmartPointer<vtkRenderer>::New())
    , axesActor(vtkSmartPointer<vtkAxesActor>::New())
    , orientationMarker(vtkSmartPointer<vtkOrientationMarkerWidget>::New())
    , activeActor(nullptr)
    , activeGrid(nullptr)
    , activeMapper(nullptr)
    , scene(new IDOSRenderScene())
    , ownsScene(true)
{
}

IDOSRenderViewPrivate::~IDOSRenderViewPrivate()
{
    if (ownsScene)
    {
        delete scene;
    }
}

vtkSmartPointer<vtkActor> IDOSRenderViewPrivate::createMeshActor(const IDOSRenderMesh* mesh)
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
    activeActor = actor;
    activeGrid = grid;
    activeMapper = mapper;
    return actor;
}

void IDOSRenderViewPrivate::applyCellScalars(vtkUnstructuredGrid* grid, vtkDataSetMapper* mapper,
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

vtkSmartPointer<vtkActor> IDOSRenderViewPrivate::createWellActor(const IDOSWellRenderObject* well)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    if (well->hasWellHead())
    {
        const QVector3D wellHeadPosition = well->wellHeadPosition();
        const vtkIdType wellHeadId = points->InsertNextPoint(
            wellHeadPosition.x(), wellHeadPosition.y(), wellHeadPosition.z());
        vertices->InsertNextCell(1, &wellHeadId);
    }

    const QVector<QVector3D>& wellPoints = well->points();
    for (int pointIndex = 0; pointIndex < wellPoints.size(); ++pointIndex)
    {
        const QVector3D& point = wellPoints.at(pointIndex);
        points->InsertNextPoint(point.x(), point.y(), point.z());
    }

    if (wellPoints.size() >= 2)
    {
        // 井口顶点单独存储，因此轨迹折线从当前 points 的尾部索引开始。
        vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
        const vtkIdType firstTrajectoryPointId = well->hasWellHead() ? 1 : 0;
        for (int pointIndex = 0; pointIndex < wellPoints.size(); ++pointIndex)
        {
            pointIds->InsertNextId(firstTrajectoryPointId + pointIndex);
        }
        lines->InsertNextCell(pointIds);
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetVerts(vertices);
    polyData->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    // 井口与轨迹使用同一颜色和可见性；井口即使无轨迹也应可见。
    actor->GetProperty()->SetColor(1.0, 0.55, 0.0);
    actor->GetProperty()->SetLineWidth(2.0);
    actor->GetProperty()->SetPointSize(10.0);
    actor->SetVisibility(well->visible());
    return actor;
}

void IDOSRenderViewPrivate::clearActivePipeline()
{
    activeActor = nullptr;
    activeGrid = nullptr;
    activeMapper = nullptr;
}

IDOSRenderView::IDOSRenderView(QWidget* parent)
    : QWidget(parent)
    , m_privateData(new IDOSRenderViewPrivate())
{
    m_privateData->vtkWidget = new QVTKOpenGLNativeWidget(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_privateData->vtkWidget);

    m_privateData->vtkWidget->setRenderWindow(m_privateData->renderWindow);
    m_privateData->renderWindow->AddRenderer(m_privateData->renderer);
    m_privateData->renderer->SetBackground(0.08, 0.09, 0.11);

    m_privateData->orientationMarker->SetOrientationMarker(m_privateData->axesActor);
    m_privateData->orientationMarker->SetInteractor(m_privateData->vtkWidget->interactor());
    m_privateData->orientationMarker->SetViewport(0.0, 0.0, 0.18, 0.18);
    m_privateData->orientationMarker->SetEnabled(1);
    m_privateData->orientationMarker->InteractiveOff();
}

IDOSRenderView::~IDOSRenderView()
{
    delete m_privateData;
}

IDOSRenderScene* IDOSRenderView::scene() const
{
    return m_privateData->scene;
}

void IDOSRenderView::setScene(IDOSRenderScene* scene)
{
    if (m_privateData->scene == scene)
    {
        return;
    }
    if (m_privateData->ownsScene)
    {
        delete m_privateData->scene;
    }
    m_privateData->scene = scene;
    m_privateData->ownsScene = false;
    rebuildActors();
    resetCamera();
}

void IDOSRenderView::addObject(IDOSRenderObject* object)
{
    if (m_privateData->scene == nullptr)
    {
        return;
    }
    m_privateData->scene->addObject(object);
    rebuildActors();
}

void IDOSRenderView::setObject(IDOSRenderObject* object)
{
    if (m_privateData->scene == nullptr)
    {
        return;
    }
    m_privateData->scene->setObject(object);
    rebuildActors();
    resetCamera();
}

QString IDOSRenderView::currentObjectId() const
{
    if (m_privateData->scene == nullptr)
    {
        return QString();
    }
    const QList<IDOSRenderObject*>& objects = m_privateData->scene->objects();
    if (objects.size() != 1 || objects.first() == nullptr)
    {
        return QString();
    }
    return objects.first()->id();
}

bool IDOSRenderView::setCellScalars(const QString& name, const QVector<double>& values)
{
    if (m_privateData->scene == nullptr)
    {
        return false;
    }
    const QList<IDOSRenderObject*>& objects = m_privateData->scene->objects();
    if (objects.size() != 1)
    {
        return false;
    }

    IDOSRenderMesh* mesh = dynamic_cast<IDOSRenderMesh*>(objects.first());
    if (mesh == nullptr || m_privateData->activeGrid == nullptr || m_privateData->activeMapper == nullptr)
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
    m_privateData->applyCellScalars(m_privateData->activeGrid, m_privateData->activeMapper, mesh);
    m_privateData->activeGrid->Modified();
    m_privateData->activeMapper->Modified();
    m_privateData->renderWindow->Render();
    return true;
}

void IDOSRenderView::clear()
{
    if (m_privateData->scene != nullptr)
    {
        m_privateData->scene->clear();
    }
    rebuildActors();
}

void IDOSRenderView::refresh()
{
    rebuildActors();
}

void IDOSRenderView::resetCamera()
{
    m_privateData->renderer->ResetCamera();
    m_privateData->renderWindow->Render();
}

void IDOSRenderView::rebuildActors()
{
    m_privateData->renderer->RemoveAllViewProps();
    m_privateData->clearActivePipeline();

    if (m_privateData->scene == nullptr)
    {
        m_privateData->renderWindow->Render();
        return;
    }

    const QList<IDOSRenderObject*>& objects = m_privateData->scene->objects();
    for (int index = 0; index < objects.size(); ++index)
    {
        IDOSRenderObject* object = objects.at(index);
        IDOSRenderMesh* mesh = dynamic_cast<IDOSRenderMesh*>(object);
        if (mesh != nullptr)
        {
            m_privateData->renderer->AddActor(m_privateData->createMeshActor(mesh));
            continue;
        }
        IDOSWellRenderObject* well = dynamic_cast<IDOSWellRenderObject*>(object);
        if (well != nullptr)
        {
            m_privateData->renderer->AddActor(m_privateData->createWellActor(well));
        }
    }

    m_privateData->renderWindow->Render();
}


