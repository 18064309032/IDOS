#include <QtAlgorithms>

#include "idosrendermetadata.h"

#include "idosrenderregistry.h"

IDOSRenderRegistry::IDOSRenderRegistry()
    :
    m_metadataByTypeId()
{
}

IDOSRenderRegistry::~IDOSRenderRegistry()
{
    clear();
}

bool IDOSRenderRegistry::registerMetadata(IDOSRenderMetadata* metadata)
{
    if (metadata == nullptr)
    {
        return false;
    }

    const QString dataTypeId = metadata->dataTypeId();
    if (dataTypeId.isEmpty() || m_metadataByTypeId.contains(dataTypeId))
    {
        return false;
    }

    m_metadataByTypeId.insert(dataTypeId, metadata);
    return true;
}

bool IDOSRenderRegistry::unregisterMetadata(const QString& dataTypeId)
{
    IDOSRenderMetadata* metadata = m_metadataByTypeId.take(dataTypeId);
    if (metadata == nullptr)
    {
        return false;
    }

    delete metadata;
    return true;
}

IDOSRenderMetadata* IDOSRenderRegistry::metadata(const QString& dataTypeId)
{
    return m_metadataByTypeId.value(dataTypeId, nullptr);
}

QList<IDOSRenderMetadata*> IDOSRenderRegistry::metadataList()
{
    return m_metadataByTypeId.values();
}

void IDOSRenderRegistry::clear()
{
    qDeleteAll(m_metadataByTypeId);
    m_metadataByTypeId.clear();
}
