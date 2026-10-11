#include "idospointset.h"
#include "idospointsetrenderprovider.h"

#include "idospointsetrendermetadata.h"

QString IDOSPointSetRenderMetadata::typeId() const
{
    return QString::fromLatin1("idos.example.pointset");
}

IDOSRenderProvider* IDOSPointSetRenderMetadata::create(IDOSDataObject* data) const
{
    IDOSPointSet* points = qobject_cast<IDOSPointSet*>(data);
    return points != nullptr ? new IDOSPointSetRenderProvider(points) : nullptr;
}
