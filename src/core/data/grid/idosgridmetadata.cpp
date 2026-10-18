#include <QObject>

#include "idosgridobject.h"

#include "idosgridmetadata.h"

IDOSGridMetadata::IDOSGridMetadata()
{
}

IDOSGridMetadata::~IDOSGridMetadata()
{
}

QString IDOSGridMetadata::typeId() const
{
    return QStringLiteral("idos.grid");
}

QString IDOSGridMetadata::displayName() const
{
    return QObject::tr("Grid");
}

IDOSDataObject* IDOSGridMetadata::create() const
{
    return new IDOSGridObject();
}
