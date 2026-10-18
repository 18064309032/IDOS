#include "idosdataobject.h"
#include "idoswelltreeprovider.h"

#include "idoswelltreemetadata.h"

QString IDOSWellTreeMetadata::id() const
{
    return QStringLiteral("idos.well");
}

QString IDOSWellTreeMetadata::treeName() const
{
    return QStringLiteral("data");
}

QString IDOSWellTreeMetadata::groupKey() const
{
    return QStringLiteral("data.wells");
}

IDOSTreeItemProvider* IDOSWellTreeMetadata::create(IDOSDataObject* data) const
{
    return new IDOSWellTreeProvider(data);
}
