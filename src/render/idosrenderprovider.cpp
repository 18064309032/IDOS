#include "idosrenderprovider.h"

IDOSRenderProvider::~IDOSRenderProvider()
{
}

bool IDOSRenderProvider::supportsDisplayMode() const
{
    return false;
}

IDOSDisplayMode IDOSRenderProvider::displayMode() const
{
    return IDOSDisplayMode::Surface;
}

bool IDOSRenderProvider::setDisplayMode(IDOSDisplayMode mode)
{
    Q_UNUSED(mode);
    return false;
}
