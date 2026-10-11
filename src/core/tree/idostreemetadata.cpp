#include "idosdataobject.h"

#include "idostreemetadata.h"

IDOSTreeMetadata::IDOSTreeMetadata()
{
}

IDOSTreeMetadata::~IDOSTreeMetadata()
{
}

QString IDOSTreeMetadata::groupKey() const
{
    return QString();
}

bool IDOSTreeMetadata::supports(const IDOSDataObject* data) const
{
    return data != nullptr && data->typeId() == typeId();
}
