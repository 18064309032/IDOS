#include "idosdataobject.h"
#include "idosproject.h"
#include "idostree.h"
#include "idostreeitem.h"
#include "idostreemetadata.h"
#include "idostreeprovider.h"
#include "idostreeregistry.h"

#include "idostreeserver.h"

IDOSTreeServer::IDOSTreeServer(IDOSTreeRegistry* registry, QObject* parent)
    :
    QObject(parent)
    , m_registry(registry)
{
    if (registry != nullptr)
    {
        connect(registry, &IDOSTreeRegistry::added, this, &IDOSTreeServer::onMetadataAdded);
        connect(registry, &IDOSTreeRegistry::aboutToRemove, this, &IDOSTreeServer::onMetadataRemoving);
        connect(registry, &IDOSTreeRegistry::removed, this, &IDOSTreeServer::onMetadataAdded);
        connect(registry, &IDOSTreeRegistry::groupsChanged, this, &IDOSTreeServer::onGroupsChanged);
    }
}

IDOSTreeServer::~IDOSTreeServer()
{
    setProject(nullptr);
}

IDOSProject* IDOSTreeServer::project() const
{
    return m_project.data();
}

void IDOSTreeServer::setProject(IDOSProject* project)
{
    if (m_project != nullptr)
    {
        disconnect(m_project, nullptr, this, nullptr);
    }
    m_project = project;
    for (const QPointer<IDOSTree>& tree : m_trees)
    {
        if (tree != nullptr)
        {
            tree->clear();
            tree->setProject(project);
            populate(tree);
        }
    }
    if (project != nullptr)
    {
        connect(project, &IDOSProject::objectAdded, this, &IDOSTreeServer::onObjectAdded);
        connect(project, &IDOSProject::objectDataChanged, this, &IDOSTreeServer::onObjectAdded);
        connect(project, &IDOSProject::objectAboutToBeRemoved, this, &IDOSTreeServer::onObjectAboutToRemove);
        connect(project, &IDOSProject::objectRemoved, this, &IDOSTreeServer::onObjectRemoved);
        connect(project, &IDOSProject::objectsAdded, this, &IDOSTreeServer::onObjectsChanged);
        connect(project, &IDOSProject::objectsDataChanged, this, &IDOSTreeServer::onObjectsChanged);
        connect(project, &IDOSProject::objectsRemoved, this, &IDOSTreeServer::onObjectsChanged);
        connect(project, &IDOSProject::objectVisibilityChanged, this, &IDOSTreeServer::onVisibilityChanged);
        connect(project, &IDOSProject::objectsVisibilityChanged, this, &IDOSTreeServer::onObjectsVisibilityChanged);
        connect(project, &QObject::destroyed, this, &IDOSTreeServer::onProjectDestroyed);
    }
}

bool IDOSTreeServer::addTree(IDOSTree* tree)
{
    if (tree == nullptr || m_trees.contains(QPointer<IDOSTree>(tree)))
    {
        return false;
    }
    m_trees.append(tree);
    tree->clear();
    tree->setProject(m_project);
    populate(tree);
    return true;
}

bool IDOSTreeServer::removeTree(IDOSTree* tree)
{
    if (!m_trees.removeOne(QPointer<IDOSTree>(tree)))
    {
        return false;
    }
    if (tree != nullptr)
    {
        tree->clear();
        tree->setProject(nullptr);
    }
    return true;
}

IDOSTreeItem* IDOSTreeServer::copyGroup(const IDOSTreeItem* source) const
{
    IDOSTreeItem* result = new IDOSTreeItem();
    result->setKey(source->key());
    result->setName(source->name());
    result->setIcon(source->icon());
    result->setGroup(true);
    for (const IDOSTreeItem* child : source->children())
    {
        result->appendChild(copyGroup(child));
    }
    return result;
}

void IDOSTreeServer::populateGroups(IDOSTree* tree)
{
    if (m_registry == nullptr)
    {
        return;
    }
    for (const IDOSTreeItem* group : m_registry->groups(tree->name()))
    {
        if (tree->find(group->key()) == nullptr)
        {
            tree->insert(tree->root(), copyGroup(group));
        }
    }
}

void IDOSTreeServer::populate(IDOSTree* tree)
{
    if (m_project == nullptr)
    {
        return;
    }
    populateGroups(tree);
    if (m_project != nullptr)
    {
        for (IDOSDataObject* data : m_project->objects())
        {
            addObject(tree, data);
        }
    }
}

void IDOSTreeServer::addObject(IDOSTree* tree, IDOSDataObject* data)
{
    if (m_registry == nullptr || data == nullptr)
    {
        return;
    }
    IDOSTreeItem* old = tree->find(QStringLiteral("object:") + data->objectId());
    const IDOSTreeMetadata* metadata = m_registry->metadata(data->typeId());
    if (metadata == nullptr || metadata->treeName() != tree->name() || !metadata->supports(data))
    {
        if (old != nullptr)
        {
            tree->remove(old);
        }
        return;
    }
    IDOSTreeItem* parent = tree->find(metadata->groupKey());
    if (parent == nullptr)
    {
        parent = tree->root();
    }
    if (old != nullptr && old->parent() == parent && old->provider() != nullptr &&
        old->provider()->dataSource() == data)
    {
        old->setName(data->name());
        old->provider()->update(old);
        return;
    }
    const int row = old != nullptr && old->parent() == parent ? old->row() : parent->childCount();
    IDOSTreeProvider* provider = metadata->create(data);
    if (provider == nullptr)
    {
        return;
    }
    IDOSTreeItem* item = new IDOSTreeItem();
    item->setKey(QStringLiteral("object:") + data->objectId());
    item->setDataId(data->objectId());
    item->setName(data->name());
    item->setProvider(provider);
    provider->populate(item);
    if (old != nullptr)
    {
        tree->remove(old);
    }
    if (!tree->insert(parent, item, row))
    {
        delete item;
    }
}

void IDOSTreeServer::update(const QStringList& ids)
{
    if (m_project == nullptr)
    {
        return;
    }
    for (const QPointer<IDOSTree>& tree : m_trees)
    {
        if (tree == nullptr)
        {
            continue;
        }
        QStringList affected;
        QList<IDOSTreeItem*> pending;
        pending.append(tree->root());
        while (!pending.isEmpty())
        {
            IDOSTreeItem* item = pending.takeLast();
            IDOSTreeProvider* provider = item->provider();
            if (provider != nullptr && item->key() == QStringLiteral("object:") + item->dataId())
            {
                if (provider->dependsOn(ids))
                {
                    affected.append(item->dataId());
                }
            }
            else
            {
                for (IDOSTreeItem* child : item->children())
                {
                    pending.append(child);
                }
            }
        }
        for (const QString& id : ids)
        {
            if (!affected.contains(id))
            {
                affected.append(id);
            }
        }
        for (const QString& id : affected)
        {
            addObject(tree, m_project->object(id));
        }
    }
}

void IDOSTreeServer::onObjectAdded(const QString& id)
{
    update(QStringList(id));
}

void IDOSTreeServer::onObjectRemoved(const QString& id)
{
    update(QStringList(id));
}

void IDOSTreeServer::onObjectsChanged(const QStringList& ids)
{
    update(ids);
}

void IDOSTreeServer::onObjectAboutToRemove(const QString& id)
{
    for (const QPointer<IDOSTree>& tree : m_trees)
    {
        if (tree != nullptr)
        {
            tree->remove(tree->find(QStringLiteral("object:") + id));
        }
    }
}

void IDOSTreeServer::updateChecks(IDOSTreeItem* item, const QStringList& ids)
{
    if (ids.contains(item->dataId()) && m_project != nullptr)
    {
        IDOSDataObject* data = m_project->object(item->dataId());
        if (data != nullptr)
        {
            item->setChecked(data->isVisible());
        }
    }
    for (IDOSTreeItem* child : item->children())
    {
        updateChecks(child, ids);
    }
}

void IDOSTreeServer::onVisibilityChanged(const QString& id, bool visible)
{
    Q_UNUSED(visible);
    onObjectsVisibilityChanged(QStringList(id));
}

void IDOSTreeServer::onObjectsVisibilityChanged(const QStringList& ids)
{
    for (const QPointer<IDOSTree>& tree : m_trees)
    {
        if (tree != nullptr)
        {
            updateChecks(tree->root(), ids);
        }
    }
}

void IDOSTreeServer::onMetadataAdded(const QString& typeId)
{
    if (m_project == nullptr)
    {
        return;
    }
    QStringList ids;
    for (IDOSDataObject* data : m_project->objects())
    {
        if (data->typeId() == typeId)
        {
            ids.append(data->objectId());
        }
    }
    update(ids);
}

void IDOSTreeServer::onMetadataRemoving(const QString& typeId)
{
    for (const QPointer<IDOSTree>& tree : m_trees)
    {
        if (tree == nullptr)
        {
            continue;
        }
        QList<IDOSTreeItem*> pending;
        pending.append(tree->root());
        while (!pending.isEmpty())
        {
            IDOSTreeItem* item = pending.takeLast();
            IDOSTreeProvider* provider = item->provider();
            if (provider != nullptr && provider->dataSource()->typeId() == typeId)
            {
                tree->remove(item);
                continue;
            }
            for (IDOSTreeItem* child : item->children())
            {
                pending.append(child);
            }
        }
    }
}

void IDOSTreeServer::onGroupsChanged(const QString& name)
{
    for (const QPointer<IDOSTree>& tree : m_trees)
    {
        if (tree != nullptr && tree->name() == name)
        {
            populateGroups(tree);
            if (m_project != nullptr)
            {
                for (IDOSDataObject* data : m_project->objects())
                {
                    addObject(tree, data);
                }
            }
        }
    }
}

void IDOSTreeServer::onProjectDestroyed()
{
    setProject(nullptr);
}
