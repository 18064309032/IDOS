#include <QObject>
#include <QUuid>
#include <QVector3D>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#endif

#include "idosrenderobject.h"
#include "idoswell.h"
#include "idoswellgeometryresolver.h"
#include "idoswellhead.h"
#include "idoswellpath.h"
#include "idoswellpathpoint.h"
#include "idoswellrenderobject.h"
#include "idosrenderprovider.h"

#include "idoswellrenderobjectprovider.h"

IDOSWellRenderObjectProvider::IDOSWellRenderObjectProvider()
{
}

IDOSWellRenderObjectProvider::~IDOSWellRenderObjectProvider()
{
}

QString IDOSWellRenderObjectProvider::dataTypeId() const
{
    return QStringLiteral("idos.well");
}

IDOSRenderObject* IDOSWellRenderObjectProvider::createRenderObject(IDOSDataObject* object) const
{
    IDOSWell* well = qobject_cast<IDOSWell*>(object);
    if (well == nullptr || (!well->hasWellHead() && !well->hasPath()))
    {
        return nullptr;
    }

    IDOSWellRenderObject* renderObject = new IDOSWellRenderObject();
    renderObject->setRenderProvider(createRenderProvider(object, renderObject));
    renderObject->setId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    renderObject->setName(well->name());
    renderObject->setInjector(well->type() == IDOSWell::Type::Injector);

    const IDOSWellGeometryResolver resolver;
    const IDOSWellGeometryResolution resolution = resolver.resolve(well->wellHead(), well->path());
    if (resolution.hasStartPoint())
    {
        renderObject->setWellHeadPosition(resolution.startPoint());
    }

    if (well->hasPath())
    {
        const QVector<IDOSWellPathPoint> points = well->path().points();
        for (const IDOSWellPathPoint& point : points)
        {
            renderObject->appendPoint(QVector3D(point.x(), point.y(), point.z()));
        }
    }

    if (!resolution.hasStartPoint() && renderObject->pointCount() < 2)
    {
        delete renderObject;
        return nullptr;
    }

    return renderObject;
}

bool IDOSWellRenderObjectProvider::canConvert(const IDOSRenderObject* object) const
{
    return dynamic_cast<const IDOSWellRenderObject*>(object) != nullptr;
}

QList<vtkActor*> IDOSWellRenderObjectProvider::toVtk(const IDOSRenderObject* object,
                                                     bool highlighted) const
{
    QList<vtkActor*> actors;
#ifdef IDOS_ENABLE_RENDER
    const IDOSWellRenderObject* well = dynamic_cast<const IDOSWellRenderObject*>(object);
    const IDOSRenderProvider* renderProvider = well != nullptr ? well->renderProvider() : nullptr;
    if (well == nullptr || renderProvider == nullptr || !renderProvider->visible())
    {
        return actors;
    }

    const double red = highlighted ? 1.0 : (well->isInjector() ? 0.2 : 0.95);
    const double green = highlighted ? 0.95 : (well->isInjector() ? 0.9 : 0.2);
    const double blue = highlighted ? 0.15 : (well->isInjector() ? 0.25 : 0.15);
    if (well->hasWellHead())
    {
        vtkSmartPointer<vtkPoints> headPoints = vtkSmartPointer<vtkPoints>::New();
        const QVector3D position = well->wellHeadPosition();
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
        headActor->GetProperty()->SetPointSize(highlighted ? 18.0 : 12.0);
        headActor->GetProperty()->SetOpacity(renderProvider->opacity());
        actors.append(headActor.GetPointer());
    }

    if (well->pointCount() >= 2)
    {
        vtkSmartPointer<vtkPoints> trajectoryPoints = vtkSmartPointer<vtkPoints>::New();
        const QVector<QVector3D>& points = well->points();
        for (int pointIndex = 0; pointIndex < points.size(); ++pointIndex)
        {
            const QVector3D& point = points.at(pointIndex);
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
        trajectoryActor->GetProperty()->SetLineWidth(highlighted ? 5.0 : 2.5);
        trajectoryActor->GetProperty()->SetOpacity(renderProvider->opacity());
        actors.append(trajectoryActor.GetPointer());
    }
#else
    Q_UNUSED(object);
    Q_UNUSED(highlighted);
#endif
    return actors;
}

QList<vtkSmartPointer<vtkProp>> IDOSWellRenderObjectProvider::toVtkLegends(
    const IDOSRenderObject* object) const
{
    Q_UNUSED(object);
    return QList<vtkSmartPointer<vtkProp>>();
}
