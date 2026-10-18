#include <QtGlobal>

#include "data/grid/idosgridboundaryobject.h"
#include "idosgridboundaryrenderprovider.h"

#include "idosgridboundaryrendermetadata.h"

QString IDOSGridBoundaryRenderMetadata::typeId() const
{
    return QStringLiteral("idos.grid.boundary");
}

IDOSRenderProvider* IDOSGridBoundaryRenderMetadata::create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const
{
    Q_UNUSED(references)
    IDOSGridBoundaryObject* boundary = qobject_cast<IDOSGridBoundaryObject*>(data);
    if (boundary == nullptr)
    {
        return nullptr;
    }
    return new IDOSGridBoundaryRenderProvider(boundary);
}
