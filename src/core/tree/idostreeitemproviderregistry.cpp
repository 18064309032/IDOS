#include "idostreeitemprovider.h"
#include "idostreeitemprovidermetadata.h"

#include "idostreeitemproviderregistry.h"

IDOSTreeItemProviderRegistry::IDOSTreeItemProviderRegistry(QObject* parent)
    :
    QObject(parent)
    , m_changing(false)
{
}

IDOSTreeItemProviderRegistry::~IDOSTreeItemProviderRegistry()
{
    while (!m_metadata.isEmpty())
    {
        unregisterMetadata(m_metadata.first()->id());
    }
}

bool IDOSTreeItemProviderRegistry::registerMetadata(IDOSTreeItemProviderMetadata* value)
{
    if (m_changing || value == nullptr || value->id().isEmpty() ||
        metadata(value->id()) != nullptr)
    {
        return false;
    }
    m_changing = true;
    m_metadata.append(value);
    emit metadataRegistered(value->id());
    m_changing = false;
    return true;
}

bool IDOSTreeItemProviderRegistry::unregisterMetadata(const QString& metadataId)
{
    if (m_changing)
    {
        return false;
    }
    for (int index = 0; index < m_metadata.size(); ++index)
    {
        IDOSTreeItemProviderMetadata* value = m_metadata.at(index);
        if (value->id() == metadataId)
        {
            m_changing = true;
            m_unregisteringId = metadataId;
            emit metadataUnregistering(metadataId);
            m_metadata.removeAt(index);
            delete value;
            m_unregisteringId.clear();
            emit metadataUnregistered(metadataId);
            m_changing = false;
            return true;
        }
    }
    return false;
}

const IDOSTreeItemProviderMetadata* IDOSTreeItemProviderRegistry::metadata(const QString& metadataId) const
{
    if (metadataId == m_unregisteringId)
    {
        return nullptr;
    }
    for (const IDOSTreeItemProviderMetadata* value : m_metadata)
    {
        if (value->id() == metadataId)
        {
            return value;
        }
    }
    return nullptr;
}

QList<const IDOSTreeItemProviderMetadata*> IDOSTreeItemProviderRegistry::metadata() const
{
    QList<const IDOSTreeItemProviderMetadata*> result;
    for (const IDOSTreeItemProviderMetadata* value : m_metadata)
    {
        if (value->id() != m_unregisteringId)
        {
            result.append(value);
        }
    }
    return result;
}

IDOSTreeItemProvider* IDOSTreeItemProviderRegistry::create(IDOSDataObject* data) const
{
    if (data == nullptr)
    {
        return nullptr;
    }
    for (const IDOSTreeItemProviderMetadata* definition : m_metadata)
    {
        if (definition->id() != m_unregisteringId && definition->supports(data))
        {
            IDOSTreeItemProvider* provider = definition->create(data);
            if (provider != nullptr)
            {
                if (!provider->setMetadataId(definition->id()))
                {
                    delete provider;
                    return nullptr;
                }
            }
            return provider;
        }
    }
    return nullptr;
}
