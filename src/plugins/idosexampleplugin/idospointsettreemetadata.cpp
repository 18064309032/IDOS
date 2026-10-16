#include "idospointsettreeprovider.h"

#include "idospointsettreemetadata.h"

QString IDOSPointSetTreeMetadata::id() const
{
    return QString::fromLatin1("idos.example.pointset");
}

QString IDOSPointSetTreeMetadata::treeName() const
{
    return QString::fromLatin1("data");
}

IDOSTreeItemProvider* IDOSPointSetTreeMetadata::createProvider(IDOSDataObject* data) const
{
    return new IDOSPointSetTreeProvider(data);
}
