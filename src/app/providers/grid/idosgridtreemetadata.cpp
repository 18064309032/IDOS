#include "idosdataobject.h"
#include "idosgridtreeprovider.h"

#include "idosgridtreemetadata.h"

QString IDOSGridTreeMetadata::typeId() const
{
    return QStringLiteral("idos.grid");
}

QString IDOSGridTreeMetadata::treeName() const
{
    return QString();
}

QString IDOSGridTreeMetadata::groupKey() const
{
    return QStringLiteral("");
}

bool IDOSGridTreeMetadata::supports(const IDOSDataObject* data) const
{
    return IDOSTreeMetadata::supports(data) && data->containerId().isEmpty();
}

IDOSTreeProvider* IDOSGridTreeMetadata::create(IDOSDataObject* data) const
{
    return new IDOSGridTreeProvider(data);
}
