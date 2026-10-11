#include <QObject>
#include <QtGlobal>
#include <QVector>
#include <QVector3D>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#endif

#include "idoswell.h"
#include "idoswellgeometryresolver.h"
#include "idoswellhead.h"
#include "idoswellpath.h"
#include "idoswellpathpoint.h"

#include "idoswellrenderprovider.h"

IDOSWellRenderProvider::IDOSWellRenderProvider(IDOSWell* dataObject)
    :
    m_dataObject(dataObject)
    , m_actors()
    , m_legends()
    , m_highlighted(false)
    , m_visible(dataObject != nullptr && dataObject->isVisible())
    , m_opacity(1.0)
{
}

IDOSWellRenderProvider::~IDOSWellRenderProvider()
{
}

IDOSDataObject* IDOSWellRenderProvider::dataSource() const
{
    return m_dataObject.data();
}

void IDOSWellRenderProvider::update()
{
    m_actors.clear();
    m_legends.clear();
    if (m_dataObject.isNull())
    {
        return;
    }
#ifdef IDOS_ENABLE_RENDER
    const IDOSWell* well = m_dataObject.data();
    const IDOSWellGeometryResolver resolver;
    const IDOSWellGeometryResolution resolution = resolver.resolve(well->wellHead(), well->path());
    const QVector<IDOSWellPathPoint> points = well->path().points();
    const double red = m_highlighted ? 1.0 : ((well->type() == IDOSWell::Type::Injector) ? 0.2 : 0.95);
    const double green = m_highlighted ? 0.95 : ((well->type() == IDOSWell::Type::Injector) ? 0.9 : 0.2);
    const double blue = m_highlighted ? 0.15 : ((well->type() == IDOSWell::Type::Injector) ? 0.25 : 0.15);
    if (resolution.hasStartPoint())
    {
        vtkSmartPointer<vtkPoints> headPoints = vtkSmartPointer<vtkPoints>::New();
        const QVector3D position = resolution.startPoint();
        const vtkIdType pointId = headPoints->InsertNextPoint(position.x(), position.y(), position.z());
        vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
        vertices->InsertNextCell(1, &pointId);
        vtkSmartPointer<vtkPolyData> headData = vtkSmartPointer<vtkPolyData>::New();
        headData->SetPoints(headPoints);
        headData->SetVerts(vertices);
        vtkSmartPointer<vtkPolyDataMapper> headMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        headMapper->SetInputData(headData);
        vtkSmartPointer<vtkActor> headActor = vtkSmartPointer<vtkActor>::New();
        headActor->SetMapper(headMapper);
        headActor->GetProperty()->SetColor(red, green, blue);
        headActor->GetProperty()->SetPointSize(m_highlighted ? 18.0 : 12.0);
        headActor->GetProperty()->SetOpacity(m_opacity);
        m_actors.append(headActor);
    }

    if (points.size() >= 2)
    {
        vtkSmartPointer<vtkPoints> trajectoryPoints = vtkSmartPointer<vtkPoints>::New();
        for (int pointIndex = 0; pointIndex < points.size(); ++pointIndex)
        {
            const IDOSWellPathPoint& point = points.at(pointIndex);
            trajectoryPoints->InsertNextPoint(point.x(), point.y(), point.z());
        }
        vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
        for (int pointIndex = 0; pointIndex < points.size(); ++pointIndex)
        {
            pointIds->InsertNextId(pointIndex);
        }
        vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
        lines->InsertNextCell(pointIds);
        vtkSmartPointer<vtkPolyData> trajectoryData = vtkSmartPointer<vtkPolyData>::New();
        trajectoryData->SetPoints(trajectoryPoints);
        trajectoryData->SetLines(lines);
        vtkSmartPointer<vtkPolyDataMapper> trajectoryMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        trajectoryMapper->SetInputData(trajectoryData);
        vtkSmartPointer<vtkActor> trajectoryActor = vtkSmartPointer<vtkActor>::New();
        trajectoryActor->SetMapper(trajectoryMapper);
        trajectoryActor->GetProperty()->SetColor(red, green, blue);
        trajectoryActor->GetProperty()->SetLineWidth(m_highlighted ? 5.0 : 2.5);
        trajectoryActor->GetProperty()->SetOpacity(m_opacity);
        m_actors.append(trajectoryActor);
    }
#endif
    applyActorProperties();
}

QList<vtkActor*> IDOSWellRenderProvider::actors() const
{
    QList<vtkActor*> result;
#ifdef IDOS_ENABLE_RENDER
    for (int index = 0; index < m_actors.size(); ++index)
    {
        result.append(m_actors.at(index).GetPointer());
    }
#endif
    return result;
}

QList<vtkSmartPointer<vtkProp>> IDOSWellRenderProvider::legends() const
{
    return m_visible ? m_legends : QList<vtkSmartPointer<vtkProp>>();
}

void IDOSWellRenderProvider::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    applyActorProperties();
}

bool IDOSWellRenderProvider::visible() const
{
    return m_visible;
}

void IDOSWellRenderProvider::setVisible(bool visible)
{
    m_visible = visible;
    applyActorProperties();
}

double IDOSWellRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSWellRenderProvider::setOpacity(double opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    applyActorProperties();
}

bool IDOSWellRenderProvider::highlighted() const
{
    return m_highlighted;
}

void IDOSWellRenderProvider::applyActorProperties()
{
#ifdef IDOS_ENABLE_RENDER
    const bool injector = !m_dataObject.isNull() && m_dataObject->type() == IDOSWell::Type::Injector;
    const double red = m_highlighted ? 1.0 : (injector ? 0.2 : 0.95);
    const double green = m_highlighted ? 0.95 : (injector ? 0.9 : 0.2);
    const double blue = m_highlighted ? 0.15 : (injector ? 0.25 : 0.15);
    for (int index = 0; index < m_actors.size(); ++index)
    {
        vtkActor* actor = m_actors.at(index).GetPointer();
        actor->GetProperty()->SetColor(red, green, blue);
        actor->GetProperty()->SetPointSize(m_highlighted ? 18.0 : 12.0);
        actor->GetProperty()->SetLineWidth(m_highlighted ? 5.0 : 2.5);
        actor->GetProperty()->SetOpacity(m_opacity);
        actor->SetVisibility(m_visible);
    }
    for (int index = 0; index < m_legends.size(); ++index)
    {
        m_legends.at(index)->SetVisibility(m_visible);
    }
#endif
}
