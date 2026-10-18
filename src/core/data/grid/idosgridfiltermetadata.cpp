#include <QObject>

#include "idosgridfilterobject.h"

#include "idosgridfiltermetadata.h"

QString IDOSGridFilterMetadata::typeId() const
{
    return QStringLiteral("idos.grid.filter");
}

QString IDOSGridFilterMetadata::displayName() const
{
    return QObject::tr("Grid filter");
}

IDOSDataObject* IDOSGridFilterMetadata::create() const
{
    return new IDOSGridFilterObject();
}
