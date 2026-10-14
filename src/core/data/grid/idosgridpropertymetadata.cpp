#include <QObject>

#include "idosgridproperty.h"

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
    return new IDOSGridProperty();
}
