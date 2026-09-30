#include "idoswellrenderobjectprovider.h"
#include "idoswell.h"
#include "idoswellgeometryresolver.h"
#include "idoswellhead.h"
#include "idoswellpath.h"
#include "idoswellpathpoint.h"
#include "idoswellrenderobject.h"

#include <QObject>
#include <QVector3D>

IDOSWellRenderObjectProvider::IDOSWellRenderObjectProvider()
{
}

IDOSWellRenderObjectProvider::~IDOSWellRenderObjectProvider()
{
}

QString IDOSWellRenderObjectProvider::providerId() const
{
    return QStringLiteral("idos.render.well");
}

QString IDOSWellRenderObjectProvider::displayName() const
{
    return QObject::tr("Well Render Object Provider");
}

bool IDOSWellRenderObjectProvider::canCreate(const IDOSDataObject* object) const
{
    const IDOSWell* well = qobject_cast<const IDOSWell*>(object);
    return well != nullptr && (well->hasWellHead() || well->hasPath());
}

IDOSRenderObject* IDOSWellRenderObjectProvider::createObject(const IDOSDataObject* object) const
{
    const IDOSWell* well = qobject_cast<const IDOSWell*>(object);
    if (well == nullptr || !canCreate(well))
    {
        return nullptr;
    }

    IDOSWellRenderObject* renderObject = new IDOSWellRenderObject();
    renderObject->setId(well->objectId());
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

    renderObject->setVisible(true);
    return renderObject;
}
