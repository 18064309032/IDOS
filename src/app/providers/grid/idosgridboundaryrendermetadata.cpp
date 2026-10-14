#include "data/grid/idosgridboundary.h"
#include "idosgridboundaryrenderprovider.h"

#include "idosgridboundaryrendermetadata.h"

QString IDOSGridBoundaryRenderMetadata::typeId() const
{
    return QStringLiteral("idos.grid.boundary");
}

IDOSRenderProvider* IDOSGridBoundaryRenderMetadata::create(IDOSDataObject* data) const
{
    IDOSGridBoundary* boundary = qobject_cast<IDOSGridBoundary*>(data);
    if (boundary == nullptr)
    {
        return nullptr;
    }
    return new IDOSGridBoundaryRenderProvider(boundary);
}
