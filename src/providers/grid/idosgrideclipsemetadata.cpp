#include "idosgrideclipsemetadata.h"
#include "idosgrideclipseprovider.h"

std::unique_ptr<IDOSDataProvider> IDOSGridEclipseMetadata::createProvider() const
{
    return std::make_unique<IDOSGridEclipseProvider>();
}
