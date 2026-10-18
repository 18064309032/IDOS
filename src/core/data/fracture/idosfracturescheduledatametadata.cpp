#include <QObject>

#include "idosfracturescheduleobject.h"

#include "idosfracturescheduledatametadata.h"

QString IDOSFractureScheduleDataMetadata::typeId() const
{
    return QStringLiteral("idos.fracture.schedule");
}

QString IDOSFractureScheduleDataMetadata::displayName() const
{
    return QObject::tr("Hydraulic Fracture Schedule");
}

IDOSDataObject* IDOSFractureScheduleDataMetadata::create() const
{
    return new IDOSFractureScheduleObject();
}
