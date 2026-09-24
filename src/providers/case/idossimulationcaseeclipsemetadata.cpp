#include "idossimulationcaseeclipsemetadata.h"
#include "idossimulationcaseeclipseprovider.h"

std::unique_ptr<IDOSDataProvider> IDOSSimulationCaseEclipseMetadata::createProvider() const
{
    return std::make_unique<IDOSSimulationCaseEclipseProvider>();
}
