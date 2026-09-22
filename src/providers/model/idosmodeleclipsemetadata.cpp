#include "idosmodeleclipsemetadata.h"
#include "idosmodeleclipseprovider.h"

std::unique_ptr<IDOSDataProvider> IDOSModelEclipseMetadata::createProvider() const
{
    return std::make_unique<IDOSModelEclipseProvider>();
}
