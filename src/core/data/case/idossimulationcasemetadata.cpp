#include <QObject>

#include "idossimulationcaseobject.h"

#include "idossimulationcasemetadata.h"

IDOSSimulationCaseMetadata::IDOSSimulationCaseMetadata()
{
}

IDOSSimulationCaseMetadata::~IDOSSimulationCaseMetadata()
{
}

QString IDOSSimulationCaseMetadata::typeId() const
{
    return QStringLiteral("idos.case.simulation");
}

QString IDOSSimulationCaseMetadata::displayName() const
{
    return QObject::tr("Case");
}

IDOSDataObject* IDOSSimulationCaseMetadata::create() const
{
    return new IDOSSimulationCaseObject();
}
