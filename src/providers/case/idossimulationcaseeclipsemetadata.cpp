#include "idossimulationcaseeclipsemetadata.h"
#include "idossimulationcaseeclipseprovider.h"

std::unique_ptr<IDOSDataProvider> IDOSSimulationCaseEclipseMetadata::createProvider() const
{
    return std::make_unique<IDOSSimulationCaseEclipseProvider>();
}

QString IDOSSimulationCaseEclipseMetadata::id() const
{
    return QStringLiteral("idos.case.simulation.eclipse");
}

QString IDOSSimulationCaseEclipseMetadata::displayName() const
{
    return QObject::tr("ECLIPSE Simulation Case");
}

QStringList IDOSSimulationCaseEclipseMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.DATA"), QStringLiteral("*.data")};
}
