#include "data/grid/idosgridsurface.h"
#include "idosgridsurfacerenderprovider.h"

#include "idosgridsurfacerendermetadata.h"

QString IDOSGridSurfaceRenderMetadata::typeId() const
{
    return QStringLiteral("idos.grid.surface");
}

IDOSRenderProvider* IDOSGridSurfaceRenderMetadata::create(IDOSDataObject* data) const
{
    IDOSGridSurface* surface = qobject_cast<IDOSGridSurface*>(data);
    if (surface == nullptr)
    {
        return nullptr;
    }
    return new IDOSGridSurfaceRenderProvider(surface);
}
