#include "idosdatametadata.h"

#include "idosdataregistry.h"

IDOSDataRegistry::IDOSDataRegistry(QObject* parent)
    :
    QObject(parent)
    , m_changing(false)
{
}

IDOSDataRegistry::~IDOSDataRegistry()
{
    qDeleteAll(m_metadata);
}

bool IDOSDataRegistry::registerMetadata(IDOSDataMetadata* metadata)
{
    if (metadata == nullptr || m_changing)
    {
        return false;
    }
    const QString typeId = metadata->typeId();
    if (typeId.isEmpty() || m_metadata.contains(typeId))
    {
        return false;
    }
    m_changing = true;
    m_metadata.insert(typeId, metadata);
    emit metadataRegistered(typeId);
    m_changing = false;
    return true;
}

bool IDOSDataRegistry::unregisterMetadata(const QString& typeId)
{
    if (m_changing || !m_metadata.contains(typeId))
    {
        return false;
    }
    m_changing = true;
    emit metadataUnregistering(typeId);
    delete m_metadata.take(typeId);
    emit metadataUnregistered(typeId);
    m_changing = false;
    return true;
}

const IDOSDataMetadata* IDOSDataRegistry::metadata(const QString& typeId) const
{
    return m_metadata.value(typeId, nullptr);
}

QList<const IDOSDataMetadata*> IDOSDataRegistry::metadata() const
{
    QList<const IDOSDataMetadata*> result;
    for (const IDOSDataMetadata* value : m_metadata)
    {
        result.append(value);
    }
    return result;
}
