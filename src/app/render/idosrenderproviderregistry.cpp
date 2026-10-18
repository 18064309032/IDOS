#include "idosrenderprovidermetadata.h"

#include "idosrenderproviderregistry.h"

IDOSRenderProviderRegistry::IDOSRenderProviderRegistry(QObject* parent)
    :
    QObject(parent)
    , m_metadataByTypeId()
    , m_changing(false)
{
}

IDOSRenderProviderRegistry::~IDOSRenderProviderRegistry()
{
    clear();
}

bool IDOSRenderProviderRegistry::registerMetadata(IDOSRenderProviderMetadata* metadata)
{
    if (m_changing || metadata == nullptr)
    {
        return false;
    }

    const QString dataTypeId = metadata->typeId();
    if (dataTypeId.isEmpty() || m_metadataByTypeId.contains(dataTypeId))
    {
        return false;
    }

    m_changing = true;
    m_metadataByTypeId.insert(dataTypeId, metadata);
    emit metadataRegistered(dataTypeId);
    m_changing = false;
    return true;
}

bool IDOSRenderProviderRegistry::unregisterMetadata(const QString& dataTypeId)
{
    IDOSRenderProviderMetadata* metadata = m_metadataByTypeId.value(dataTypeId, nullptr);
    if (m_changing || metadata == nullptr)
    {
        return false;
    }

    m_changing = true;
    m_unregisteringId = dataTypeId;
    emit metadataUnregistering(dataTypeId);
    m_metadataByTypeId.remove(dataTypeId);
    delete metadata;
    m_unregisteringId.clear();
    emit metadataUnregistered(dataTypeId);
    m_changing = false;
    return true;
}

IDOSRenderProviderMetadata* IDOSRenderProviderRegistry::metadata(const QString& dataTypeId) const
{
    if (dataTypeId == m_unregisteringId)
    {
        return nullptr;
    }
    return m_metadataByTypeId.value(dataTypeId, nullptr);
}

QList<IDOSRenderProviderMetadata*> IDOSRenderProviderRegistry::metadata() const
{
    QList<IDOSRenderProviderMetadata*> result;
    for (QMap<QString, IDOSRenderProviderMetadata*>::const_iterator entry = m_metadataByTypeId.constBegin();
         entry != m_metadataByTypeId.constEnd(); ++entry)
    {
        if (entry.key() != m_unregisteringId)
        {
            result.append(entry.value());
        }
    }
    return result;
}

void IDOSRenderProviderRegistry::clear()
{
    const QList<QString> types = m_metadataByTypeId.keys();
    for (const QString& type : types)
    {
        unregisterMetadata(type);
    }
}
