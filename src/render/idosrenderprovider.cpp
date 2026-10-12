#include <QtGlobal>

#include "idosrenderprovider.h"

IDOSRenderProvider::~IDOSRenderProvider()
{
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
