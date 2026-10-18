#include <QtGlobal>

#include "idospointsetobject.h"
#include "idospointsetrenderprovider.h"

#include "idospointsetrendermetadata.h"

QString IDOSPointSetRenderMetadata::typeId() const
{
    return QString::fromLatin1("idos.example.pointset");
}

IDOSRenderProvider* IDOSPointSetRenderMetadata::create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const
{
    Q_UNUSED(references)
    IDOSPointSetObject* points = qobject_cast<IDOSPointSetObject*>(data);
    return points != nullptr ? new IDOSPointSetRenderProvider(points) : nullptr;
}
