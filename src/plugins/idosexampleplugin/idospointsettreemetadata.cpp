#include "idospointsettreeprovider.h"

#include "idospointsettreemetadata.h"

QString IDOSPointSetTreeMetadata::typeId() const
{
    return QString::fromLatin1("idos.example.pointset");
}

QString IDOSPointSetTreeMetadata::treeName() const
{
    return QString::fromLatin1("data");
}

IDOSTreeProvider* IDOSPointSetTreeMetadata::create(IDOSDataObject* data) const
{
    return new IDOSPointSetTreeProvider(data);
}
