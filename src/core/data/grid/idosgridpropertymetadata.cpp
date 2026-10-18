#include <QObject>

#include "idosgridpropertyobject.h"

#include "idosgridpropertymetadata.h"

QString IDOSGridPropertyMetadata::typeId() const
{
    return QStringLiteral("idos.gridproperty");
}

QString IDOSGridPropertyMetadata::displayName() const
{
    return QObject::tr("Grid Property");
}

IDOSDataObject* IDOSGridPropertyMetadata::create() const
{
    return new IDOSGridPropertyObject();
}
