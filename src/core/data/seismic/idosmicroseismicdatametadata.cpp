#include <QObject>

#include "idosmicroseismicset.h"

#include "idosmicroseismicdatametadata.h"

QString IDOSMicroseismicDataMetadata::typeId() const
{
    return QStringLiteral("idos.microseismic.set");
}

QString IDOSMicroseismicDataMetadata::displayName() const
{
    return QObject::tr("Microseismic Event Set");
}

IDOSDataObject* IDOSMicroseismicDataMetadata::create() const
{
    return new IDOSMicroseismicSet();
}
