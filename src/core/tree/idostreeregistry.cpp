#include "idostreeitem.h"
#include "idostreemetadata.h"

#include "idosdatametadata.h"
#include "idosdataregistry.h"

#include "idostreeregistry.h"

IDOSTreeRegistry::IDOSTreeRegistry(QObject* parent)
    :
    QObject(parent)
    , m_changing(false)
{
}

IDOSTreeRegistry::~IDOSTreeRegistry()
{
    qDeleteAll(m_metadata);
    qDeleteAll(m_groups);
}

bool IDOSTreeRegistry::add(IDOSTreeMetadata* value)
{
    if (m_changing || value == nullptr || value->typeId().isEmpty() ||
        metadata(value->typeId()) != nullptr)
    {
        return false;
    }
    m_changing = true;
    m_metadata.append(value);
    emit added(value->typeId());
    m_changing = false;
    return true;
}

bool IDOSTreeRegistry::remove(const QString& typeId)
{
    if (m_changing)
    {
        return false;
    }
    for (int index = 0; index < m_metadata.size(); ++index)
    {
        IDOSTreeMetadata* value = m_metadata.at(index);
        if (value->typeId() == typeId)
        {
            m_changing = true;
            emit aboutToRemove(typeId);
            m_metadata.removeAt(index);
            delete value;
            emit removed(typeId);
            m_changing = false;
            return true;
        }
    }
    return false;
}

const IDOSTreeMetadata* IDOSTreeRegistry::metadata(const QString& typeId) const
{
    for (const IDOSTreeMetadata* value : m_metadata)
    {
        if (value->typeId() == typeId)
        {
            return value;
        }
    }
    return nullptr;
}

QList<const IDOSTreeMetadata*> IDOSTreeRegistry::metadata() const
{
    QList<const IDOSTreeMetadata*> result;
    for (const IDOSTreeMetadata* value : m_metadata)
    {
        result.append(value);
    }
    return result;
}

QList<const IDOSDataMetadata*> IDOSTreeRegistry::dataMetadata(const IDOSDataRegistry* dataRegistry) const
{
    QList<const IDOSDataMetadata*> result;
    if (dataRegistry == nullptr)
    {
        return result;
    }
    for (const IDOSTreeMetadata* treeMetadata : m_metadata)
    {
        const IDOSDataMetadata* dataValue = dataRegistry->metadata(treeMetadata->typeId());
        if (dataValue != nullptr)
        {
            result.append(dataValue);
        }
    }
    return result;
}

bool IDOSTreeRegistry::addGuiProvider(const std::shared_ptr<IDOSTreeGuiProvider>& provider)
{
    if (m_changing || !provider || m_guiProviders.contains(provider))
    {
        return false;
    }
    m_changing = true;
    m_guiProviders.append(provider);
    emit guiProvidersChanged();
    m_changing = false;
    return true;
}

bool IDOSTreeRegistry::removeGuiProvider(IDOSTreeGuiProvider* provider)
{
    if (m_changing)
    {
        return false;
    }
    for (int index = 0; index < m_guiProviders.size(); ++index)
    {
        if (m_guiProviders.at(index).get() == provider)
        {
            m_changing = true;
            m_guiProviders.removeAt(index);
            emit guiProvidersChanged();
            m_changing = false;
            return true;
        }
    }
    return false;
}

QList<std::shared_ptr<IDOSTreeGuiProvider>> IDOSTreeRegistry::guiProviders() const
{
    return m_guiProviders;
}

bool IDOSTreeRegistry::addGroup(const QString& treeName, IDOSTreeItem* group)
{
    if (m_changing || treeName.isEmpty() || group == nullptr || group->parent() != nullptr ||
        group->tree() != nullptr || group->key().isEmpty())
    {
        return false;
    }
    for (int index = 0; index < m_groups.size(); ++index)
    {
        if (m_groupTrees.at(index) == treeName && m_groups.at(index)->key() == group->key())
        {
            return false;
        }
    }
    m_changing = true;
    m_groups.append(group);
    m_groupTrees.append(treeName);
    emit groupsChanged(treeName);
    m_changing = false;
    return true;
}

QList<const IDOSTreeItem*> IDOSTreeRegistry::groups(const QString& treeName) const
{
    QList<const IDOSTreeItem*> result;
    for (int index = 0; index < m_groups.size(); ++index)
    {
        if (m_groupTrees.at(index) == treeName)
        {
            result.append(m_groups.at(index));
        }
    }
    return result;
}
