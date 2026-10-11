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

bool IDOSDataRegistry::add(IDOSDataMetadata* metadata)
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
    emit added(typeId);
    m_changing = false;
    return true;
}

bool IDOSDataRegistry::remove(const QString& typeId)
{
    if (m_changing || !m_metadata.contains(typeId))
    {
        return false;
    }
    m_changing = true;
    emit aboutToBeRemoved(typeId);
    delete m_metadata.take(typeId);
    emit removed(typeId);
    m_changing = false;
    return true;
}

const IDOSDataMetadata* IDOSDataRegistry::metadata(const QString& typeId) const
{
    return m_metadata.value(typeId, nullptr);
}
