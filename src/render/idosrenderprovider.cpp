#include <QtGlobal>

#include "idosrenderprovider.h"

IDOSRenderProvider::~IDOSRenderProvider()
{
}

IDOSDisplayMode IDOSRenderProvider::displayMode() const
{
    return IDOSDisplayMode::Surface;
}

bool IDOSRenderProvider::setDisplayMode(IDOSDisplayMode mode)
{
    Q_UNUSED(mode)
    return false;
}

bool IDOSRenderProvider::dependsOn(const QStringList& dataIds) const
{
    Q_UNUSED(dataIds)
    return false;
}

bool IDOSRenderProvider::removeDataReference(const QString& dataId)
{
    Q_UNUSED(dataId)
    return false;
}
