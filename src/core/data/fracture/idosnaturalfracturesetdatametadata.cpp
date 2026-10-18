#include <QObject>

#include "idosnaturalfracturesetobject.h"

#include "idosnaturalfracturesetdatametadata.h"

QString IDOSNaturalFractureSetDataMetadata::typeId() const
{
    return QStringLiteral("idos.naturalFracture.set");
}

QString IDOSNaturalFractureSetDataMetadata::displayName() const
{
    return QObject::tr("Natural Fracture Set");
}

IDOSDataObject* IDOSNaturalFractureSetDataMetadata::create() const
{
    return new IDOSNaturalFractureSetObject();
}
