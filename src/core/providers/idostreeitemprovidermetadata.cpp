#include "idosdataobject.h"

#include "idostreeitemprovidermetadata.h"

IDOSTreeItemProviderMetadata::IDOSTreeItemProviderMetadata()
{
}

IDOSTreeItemProviderMetadata::~IDOSTreeItemProviderMetadata()
{
}

QString IDOSTreeItemProviderMetadata::groupKey() const
{
    return QString();
}

bool IDOSTreeItemProviderMetadata::supports(const IDOSDataObject* data) const
{
    return data != nullptr && data->typeId() == id();
}
