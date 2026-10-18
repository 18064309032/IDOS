#include <QtGlobal>

#include "idosrenderprovider.h"

#include "idosrenderprovidermetadata.h"

IDOSRenderProviderMetadata::~IDOSRenderProviderMetadata()
{
}

QStringList IDOSRenderProviderMetadata::dependencies(const IDOSDataObject* object, const QVariantMap& state) const
{
    Q_UNUSED(object)
    Q_UNUSED(state)
    return {};
}

bool IDOSRenderProviderMetadata::update(IDOSRenderProvider* provider, const QList<IDOSDataObject*>& references) const
{
    Q_UNUSED(references)
    if (provider == nullptr)
    {
        return false;
    }
    provider->update();
    return true;
}
