#include "idoswellrenderobjectprovider.h"
#include "idoswell.h"
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

    if (well->hasWellHead())
    {
        const IDOSWellHead head = well->wellHead();
        renderObject->setWellHeadPosition(
            QVector3D(head.surfaceX(), head.surfaceY(), head.surfaceElevation()));
    }

    if (well->hasPath())
    {
        const QVector<IDOSWellPathPoint> points = well->path().points();
        for (const IDOSWellPathPoint& point : points)
        {
            renderObject->appendPoint(QVector3D(point.x(), point.y(), point.z()));
        }
    }
    renderObject->setVisible(true);
    return renderObject;
}
