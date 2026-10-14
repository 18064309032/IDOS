#include <cmath>

#include <QVector>
#include <QtGlobal>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#endif

#include "idosgridsurface.h"

#include "idosgridsurfacerenderprovider.h"

IDOSGridSurfaceRenderProvider::IDOSGridSurfaceRenderProvider(IDOSGridSurface* surface)
    :
    m_surface(surface)
    , m_actor()
    , m_visible(true)
    , m_highlighted(false)
    , m_opacity(1.0)
{
    update();
}

IDOSGridSurfaceRenderProvider::~IDOSGridSurfaceRenderProvider() = default;

IDOSDataObject* IDOSGridSurfaceRenderProvider::dataSource() const
{
    return m_surface.data();
}

void IDOSGridSurfaceRenderProvider::update()
{
#ifdef IDOS_ENABLE_RENDER
    m_actor = nullptr;
    if (m_surface.isNull() || m_surface->nx() < 2 || m_surface->ny() < 2)
    {
        return;
    }
    const QVector<double> values = m_surface->values();
    if (values.size() != m_surface->nx() * m_surface->ny())
    {
        return;
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkDoubleArray> elevations = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();
    elevations->SetName("Elevation");
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(values.size());
    elevations->SetNumberOfValues(values.size());
    for (int row = 0; row < m_surface->ny(); ++row)
    {
        for (int column = 0; column < m_surface->nx(); ++column)
        {
            const vtkIdType pointId = static_cast<vtkIdType>(row) * m_surface->nx() + column;
            const double elevation = values.at(static_cast<int>(pointId));
            const double z = std::isfinite(elevation) ? elevation : 0.0;
            points->SetPoint(pointId, m_surface->originX() + column * m_surface->spacingX(),
                             m_surface->originY() + row * m_surface->spacingY(), z);
            elevations->SetValue(pointId, elevation);
        }
    }
    for (int row = 0; row < m_surface->ny() - 1; ++row)
    {
        for (int column = 0; column < m_surface->nx() - 1; ++column)
        {
            const vtkIdType first = static_cast<vtkIdType>(row) * m_surface->nx() + column;
            const vtkIdType second = first + 1;
            const vtkIdType third = first + m_surface->nx();
            const vtkIdType fourth = third + 1;
            if (!std::isfinite(values.at(static_cast<int>(first))) ||
                !std::isfinite(values.at(static_cast<int>(second))) ||
                !std::isfinite(values.at(static_cast<int>(third))) ||
                !std::isfinite(values.at(static_cast<int>(fourth))))
            {
                continue;
            }
            polygons->InsertNextCell(4);
            polygons->InsertCellPoint(first);
            polygons->InsertCellPoint(second);
            polygons->InsertCellPoint(fourth);
            polygons->InsertCellPoint(third);
        }
    }
    vtkSmartPointer<vtkPolyData> surfaceData = vtkSmartPointer<vtkPolyData>::New();
    surfaceData->SetPoints(points);
    surfaceData->SetPolys(polygons);
    surfaceData->GetPointData()->SetScalars(elevations);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(surfaceData);
    mapper->SetScalarRange(surfaceData->GetScalarRange());
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(mapper);
    applyActorProperties();
#endif
}

QList<vtkActor*> IDOSGridSurfaceRenderProvider::actors() const
{
    QList<vtkActor*> result;
#ifdef IDOS_ENABLE_RENDER
    if (m_actor != nullptr)
    {
        result.append(m_actor.GetPointer());
    }
#endif
    return result;
}

QList<vtkSmartPointer<vtkProp>> IDOSGridSurfaceRenderProvider::legends() const
{
    return {};
}

bool IDOSGridSurfaceRenderProvider::visible() const
{
    return m_visible;
}

void IDOSGridSurfaceRenderProvider::setVisible(bool visible)
{
    m_visible = visible;
    applyActorProperties();
}

bool IDOSGridSurfaceRenderProvider::highlighted() const
{
    return m_highlighted;
}

void IDOSGridSurfaceRenderProvider::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    applyActorProperties();
}

double IDOSGridSurfaceRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSGridSurfaceRenderProvider::setOpacity(double opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    applyActorProperties();
}

void IDOSGridSurfaceRenderProvider::applyActorProperties()
{
#ifdef IDOS_ENABLE_RENDER
    if (m_actor == nullptr)
    {
        return;
    }
    m_actor->SetVisibility(m_visible ? 1 : 0);
    m_actor->GetProperty()->SetOpacity(m_opacity);
    m_actor->GetProperty()->SetEdgeVisibility(m_highlighted ? 1 : 0);
    m_actor->GetProperty()->SetEdgeColor(1.0, 0.85, 0.0);
#endif
}
