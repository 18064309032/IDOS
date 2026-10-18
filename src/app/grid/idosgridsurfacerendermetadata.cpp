#include <QtGlobal>

#include "data/grid/idosgridsurfaceobject.h"
#include "idosgridsurfacerenderprovider.h"

#include "idosgridsurfacerendermetadata.h"

QString IDOSGridSurfaceRenderMetadata::typeId() const
{
    return QStringLiteral("idos.grid.surface");
}

IDOSRenderProvider* IDOSGridSurfaceRenderMetadata::create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const
{
    Q_UNUSED(references)
    IDOSGridSurfaceObject* surface = qobject_cast<IDOSGridSurfaceObject*>(data);
    if (surface == nullptr)
    {
        return nullptr;
    }
    return new IDOSGridSurfaceRenderProvider(surface);
}
