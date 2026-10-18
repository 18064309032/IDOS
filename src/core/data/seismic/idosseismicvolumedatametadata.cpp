#include <QObject>

#include "idosseismicvolumeobject.h"

#include "idosseismicvolumedatametadata.h"

QString IDOSSeismicVolumeDataMetadata::typeId() const
{
    return QStringLiteral("idos.seismic.volume");
}

QString IDOSSeismicVolumeDataMetadata::displayName() const
{
    return QObject::tr("Seismic Volume");
}

IDOSDataObject* IDOSSeismicVolumeDataMetadata::create() const
{
    return new IDOSSeismicVolumeObject();
}
