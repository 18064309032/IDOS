#include "idoswelllasmetadata.h"
#include "idoswelllasprovider.h"

std::unique_ptr<IDOSDataProvider> IDOSWellLasMetadata::createProvider() const
{
    return std::make_unique<IDOSWellLasProvider>();
}
