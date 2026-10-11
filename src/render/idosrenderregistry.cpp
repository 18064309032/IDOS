#include "idosrendermetadata.h"
#include "idosrenderguiprovider.h"

#include "idosrenderregistry.h"

IDOSRenderRegistry::IDOSRenderRegistry(QObject* parent)
    :
    QObject(parent)
    , m_metadataByTypeId()
    , m_changing(false)
{
}

IDOSRenderRegistry::~IDOSRenderRegistry()
{
    clear();
    qDeleteAll(m_guiProviders);
}

bool IDOSRenderRegistry::add(IDOSRenderMetadata* metadata)
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
    emit metadataAdded(dataTypeId);
    m_changing = false;
    return true;
}

bool IDOSRenderRegistry::remove(const QString& dataTypeId)
{
    IDOSRenderMetadata* metadata = m_metadataByTypeId.value(dataTypeId, nullptr);
    if (m_changing || metadata == nullptr)
    {
        return false;
    }

    m_changing = true;
    emit metadataAboutToBeRemoved(dataTypeId);
    m_metadataByTypeId.remove(dataTypeId);
    delete metadata;
    m_changing = false;
    return true;
}

IDOSRenderMetadata* IDOSRenderRegistry::metadata(const QString& dataTypeId) const
{
    return m_metadataByTypeId.value(dataTypeId, nullptr);
}

QList<IDOSRenderMetadata*> IDOSRenderRegistry::metadata() const
{
    return m_metadataByTypeId.values();
}

void IDOSRenderRegistry::clear()
{
    const QList<QString> types = m_metadataByTypeId.keys();
    for (const QString& type : types)
    {
        remove(type);
    }
}

bool IDOSRenderRegistry::addGuiProvider(IDOSRenderGuiProvider* provider)
{
    if (m_changing || provider == nullptr || provider->name().isEmpty())
    {
        return false;
    }
    for (const IDOSRenderGuiProvider* existing : m_guiProviders)
    {
        if (existing->name() == provider->name())
        {
            return false;
        }
    }
    m_guiProviders.append(provider);
    return true;
}

bool IDOSRenderRegistry::removeGuiProvider(const QString& name)
{
    if (m_changing)
    {
        return false;
    }
    for (int index = 0; index < m_guiProviders.size(); ++index)
    {
        IDOSRenderGuiProvider* provider = m_guiProviders.at(index);
        if (provider->name() == name)
        {
            m_guiProviders.removeAt(index);
            delete provider;
            return true;
        }
    }
    return false;
}

QList<IDOSRenderGuiProvider*> IDOSRenderRegistry::guiProviders() const
{
    return m_guiProviders;
}
