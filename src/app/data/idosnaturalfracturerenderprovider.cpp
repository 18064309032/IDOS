#include <cmath>

#include <QVariantList>
#include <QVariantMap>
#include <QVector>
#include <QtGlobal>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkTriangleFilter.h>
#endif

#include "data/fracture/idosnaturalfracturesetobject.h"

#include "idosnaturalfracturerenderprovider.h"

IDOSNaturalFractureRenderProvider::IDOSNaturalFractureRenderProvider(IDOSNaturalFractureSetObject* fractures)
    :
    m_fractures(fractures)
    , m_actor()
    , m_visible(true)
    , m_highlighted(false)
    , m_opacity(1.0)
    , m_displayMode(IDOSDisplayMode::SurfaceWithEdges)
{
    update();
}

IDOSNaturalFractureRenderProvider::~IDOSNaturalFractureRenderProvider() = default;

IDOSNaturalFractureSetObject* IDOSNaturalFractureRenderProvider::fractures() const
{
    return m_fractures.data();
}

void IDOSNaturalFractureRenderProvider::update()
{
#ifdef IDOS_ENABLE_RENDER
    m_actor = nullptr;
    if (m_fractures.isNull())
    {
        return;
    }
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIntArray> indices = vtkSmartPointer<vtkIntArray>::New();
    indices->SetName("FractureIndex");
    vtkSmartPointer<vtkIntArray> ids = vtkSmartPointer<vtkIntArray>::New();
    ids->SetName("FractureId");
    const QVariantList reports = m_fractures->geometryReport();
    for (const QVariant& reportValue : reports)
    {
        const QVariantMap report = reportValue.toMap();
        if (!report.value(QStringLiteral("valid")).toBool())
        {
            continue;
        }
        const QVector<double> coordinates = report.value(QStringLiteral("coordinates")).value<QVector<double>>();
        const vtkIdType firstPoint = points->GetNumberOfPoints();
        const int vertexCount = coordinates.size() / 3;
        polygons->InsertNextCell(vertexCount);
        for (int vertex = 0; vertex < vertexCount; ++vertex)
        {
            points->InsertNextPoint(coordinates.at(vertex * 3), coordinates.at(vertex * 3 + 1),
                                    coordinates.at(vertex * 3 + 2));
            polygons->InsertCellPoint(firstPoint + vertex);
        }
        indices->InsertNextValue(report.value(QStringLiteral("index")).toInt());
        ids->InsertNextValue(report.value(QStringLiteral("fractureId")).toInt());
    }
    if (polygons->GetNumberOfCells() == 0)
    {
        return;
    }
    vtkSmartPointer<vtkPolyData> geometry = vtkSmartPointer<vtkPolyData>::New();
    geometry->SetPoints(points);
    geometry->SetPolys(polygons);
    geometry->GetCellData()->AddArray(indices);
    geometry->GetCellData()->AddArray(ids);
    vtkSmartPointer<vtkTriangleFilter> triangles = vtkSmartPointer<vtkTriangleFilter>::New();
    triangles->SetInputData(geometry);
    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputConnection(triangles->GetOutputPort());
    normals->ComputeCellNormalsOn();
    normals->ComputePointNormalsOff();
    normals->SplittingOff();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(normals->GetOutputPort());
    mapper->ScalarVisibilityOff();
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(mapper);
    m_actor->GetProperty()->BackfaceCullingOff();
    applyActorProperties();
#endif
}

QList<vtkProp*> IDOSNaturalFractureRenderProvider::props() const
{
    QList<vtkProp*> result;
#ifdef IDOS_ENABLE_RENDER
    if (m_actor != nullptr)
    {
        result.append(m_actor.GetPointer());
    }
#endif
    return result;
}


bool IDOSNaturalFractureRenderProvider::isVisible() const
{
    return m_visible;
}

void IDOSNaturalFractureRenderProvider::setVisible(bool visible)
{
    m_visible = visible;
    applyActorProperties();
}

bool IDOSNaturalFractureRenderProvider::isSelected() const
{
    return m_highlighted;
}

void IDOSNaturalFractureRenderProvider::setSelected(bool highlighted)
{
    m_highlighted = highlighted;
    applyActorProperties();
}

double IDOSNaturalFractureRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSNaturalFractureRenderProvider::setOpacity(double opacity)
{
    if (!std::isfinite(opacity))
    {
        return;
    }
    m_opacity = qBound(0.0, opacity, 1.0);
    applyActorProperties();
}

IDOSDisplayMode IDOSNaturalFractureRenderProvider::displayMode() const
{
    return m_displayMode;
}

bool IDOSNaturalFractureRenderProvider::setDisplayMode(IDOSDisplayMode mode)
{
    if (mode < IDOSDisplayMode::Surface || mode > IDOSDisplayMode::Points)
    {
        return false;
    }
    m_displayMode = mode;
    applyActorProperties();
    return true;
}

void IDOSNaturalFractureRenderProvider::applyActorProperties()
{
#ifdef IDOS_ENABLE_RENDER
    if (m_actor == nullptr)
    {
        return;
    }
    m_actor->SetVisibility(m_visible ? 1 : 0);
    vtkProperty* property = m_actor->GetProperty();
    property->SetColor(m_highlighted ? 1.0 : 0.9, m_highlighted ? 0.85 : 0.45, 0.15);
    property->SetEdgeColor(m_highlighted ? 1.0 : 0.2, m_highlighted ? 0.85 : 0.15, 0.05);
    property->SetLineWidth(m_highlighted ? 3.0 : 1.0);
    property->SetOpacity(m_opacity);
    property->SetRepresentation(m_displayMode == IDOSDisplayMode::Wireframe ? VTK_WIREFRAME
        : (m_displayMode == IDOSDisplayMode::Points ? VTK_POINTS : VTK_SURFACE));
    property->SetEdgeVisibility(m_displayMode == IDOSDisplayMode::SurfaceWithEdges || m_highlighted);
#endif
}

QVariantMap IDOSNaturalFractureRenderProvider::saveState() const
{
    QVariantMap state;
    state.insert(QStringLiteral("visible"), isVisible());
    state.insert(QStringLiteral("highlighted"), isSelected());
    state.insert(QStringLiteral("opacity"), opacity());
    state.insert(QStringLiteral("displayMode"), static_cast<int>(displayMode()));
    return state;
}

bool IDOSNaturalFractureRenderProvider::restoreState(const QVariantMap& state)
{
    bool validOpacity = false;
    const double opacityValue = state.value(QStringLiteral("opacity")).toDouble(&validOpacity);
    if (!validOpacity || !(opacityValue >= 0.0 && opacityValue <= 1.0) ||
        state.value(QStringLiteral("visible")).type() != QVariant::Bool)
    {
        return false;
    }
    setVisible(state.value(QStringLiteral("visible")).toBool());
    setSelected(state.value(QStringLiteral("highlighted")).toBool());
    setOpacity(opacityValue);
    setDisplayMode(static_cast<IDOSDisplayMode>(state.value(QStringLiteral("displayMode")).toInt()));
    return true;
}
