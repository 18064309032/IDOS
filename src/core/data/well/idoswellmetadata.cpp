#include <QObject>

#include "idoswellobject.h"

#include "idoswellmetadata.h"

IDOSWellMetadata::IDOSWellMetadata()
{
}

IDOSWellMetadata::~IDOSWellMetadata()
{
}

QString IDOSWellMetadata::typeId() const
{
    return QStringLiteral("idos.well");
}

QString IDOSWellMetadata::displayName() const
{
    return QObject::tr("Well");
}

IDOSDataObject* IDOSWellMetadata::create() const
{
    return new IDOSWellObject();
}
