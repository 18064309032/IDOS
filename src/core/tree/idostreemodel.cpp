#include <QScopedValueRollback>
#include <QVector>

#include "idosdataobject.h"
#include "idostreeitem.h"
#include "idostreeitemprovider.h"
#include "idostreeitemprovidermetadata.h"
#include "idostreeitemproviderregistry.h"

#include "idostreemodel.h"

IDOSTreeModel::IDOSTreeModel(IDOSTreeItemProviderRegistry* registry, QObject* parent)
    :
    QAbstractItemModel(parent)
    , m_root(new IDOSTreeItem(new IDOSTreeItemProvider()))
    , m_registry(registry)
    , m_changing(false)
    , m_committing(false)
    , m_resetting(false)
    , m_refreshing(false)
    , m_inserting(false)
    , m_changeParent(nullptr)
    , m_changeItem(nullptr)
    , m_changeRow(-1)
{
    m_root->setModel(this);
    if (registry != nullptr)
    {
        connect(registry, &IDOSTreeItemProviderRegistry::metadataRegistered, this, &IDOSTreeModel::onMetadataRegistered);
        connect(registry, &IDOSTreeItemProviderRegistry::metadataUnregistering, this, &IDOSTreeModel::onMetadataRemoving);
        connect(registry, &IDOSTreeItemProviderRegistry::metadataUnregistered, this, &IDOSTreeModel::onMetadataRegistered);
    }
}

IDOSTreeModel::~IDOSTreeModel()
{
    delete m_root;
}

IDOSTreeItem* IDOSTreeModel::root() const
{
    return m_root;
}

void IDOSTreeModel::setRootProvider(IDOSTreeItemProvider* provider)
{
    if (provider == nullptr || provider->item() != nullptr || m_changing || m_resetting || m_refreshing)
    {
        return;
    }
    m_resetting = true;
    beginResetModel();
    IDOSTreeItem* previousRoot = m_root;
    m_root = new IDOSTreeItem(provider);
    m_root->setModel(this);
    delete previousRoot;
    m_root->provider()->refresh();
    m_resetting = false;
    endResetModel();
}

void IDOSTreeModel::refreshCheckStates()
{
    refreshCheckStates(m_root);
}

void IDOSTreeModel::refreshCheckStates(IDOSTreeItem* item)
{
    for (IDOSTreeItem* child : item->children())
    {
        refreshCheckStates(child);
    }
    const QModelIndex index = this->index(item);
    if (index.isValid())
    {
        emit dataChanged(index, index, {Qt::CheckStateRole});
    }
}

void IDOSTreeModel::refresh()
{
    if (m_changing || m_resetting || m_refreshing)
    {
        return;
    }
    m_resetting = true;
    beginResetModel();
    clear();
    m_root->provider()->refresh();
    m_resetting = false;
    endResetModel();
}

IDOSTreeItemProviderRegistry* IDOSTreeModel::registry() const
{
    return m_registry.data();
}

QString IDOSTreeModel::name() const
{
    return m_name;
}

void IDOSTreeModel::setName(const QString& name)
{
    m_name = name;
}

bool IDOSTreeModel::insertItem(IDOSTreeItem* parent, IDOSTreeItem* item, int row)
{
    if (m_changing || parent == nullptr || parent->model() != this || item == nullptr ||
        item->parent() != nullptr || item->model() != nullptr)
    {
        return false;
    }
    for (IDOSTreeItem* ancestor = parent; ancestor != nullptr; ancestor = ancestor->parent())
    {
        if (ancestor == item)
        {
            return false;
        }
    }
    row = row < 0 ? parent->childCount() : row;
    if (row > parent->childCount())
    {
        return false;
    }
    QScopedValueRollback<bool> changing(m_changing, true);
    m_changeParent = parent;
    m_changeItem = item;
    m_changeRow = row;
    m_inserting = true;
    if (!m_resetting)
    {
        beginInsertRows(this->index(parent), row, row);
    }
    m_committing = true;
    const bool inserted = parent->insertChild(row, item);
    m_committing = false;
    if (!m_resetting)
    {
        endInsertRows();
    }
    m_changeParent = nullptr;
    m_changeItem = nullptr;
    m_changeRow = -1;
    return inserted;
}

bool IDOSTreeModel::removeItem(IDOSTreeItem* item)
{
    if (m_changing || item == nullptr || item == m_root || item->model() != this || item->parent() == nullptr)
    {
        return false;
    }
    IDOSTreeItem* parent = item->parent();
    const int row = item->row();
    forgetRefreshItems(item);
    QScopedValueRollback<bool> changing(m_changing, true);
    m_changeParent = parent;
    m_changeItem = item;
    m_changeRow = row;
    m_inserting = false;
    if (!m_resetting)
    {
        beginRemoveRows(this->index(parent), row, row);
    }
    m_committing = true;
    const bool removed = parent->removeChild(item);
    m_committing = false;
    if (!m_resetting)
    {
        endRemoveRows();
    }
    m_changeParent = nullptr;
    m_changeItem = nullptr;
    m_changeRow = -1;
    return removed;
}

void IDOSTreeModel::clear()
{
    while (m_root->childCount() > 0)
    {
        if (!removeItem(m_root->child(m_root->childCount() - 1)))
        {
            break;
        }
    }
}

IDOSTreeItem* IDOSTreeModel::lookup(IDOSTreeItem* branch, const QString& value, bool byId) const
{
    if ((byId ? branch->provider()->id() : branch->provider()->key()) == value)
    {
        return branch;
    }
    for (IDOSTreeItem* child : branch->children())
    {
        IDOSTreeItem* found = lookup(child, value, byId);
        if (found != nullptr)
        {
            return found;
        }
    }
    return nullptr;
}

IDOSTreeItem* IDOSTreeModel::item(const QString& id) const
{
    return id.isEmpty() ? nullptr : lookup(m_root, id, true);
}

IDOSTreeItem* IDOSTreeModel::find(const QString& key) const
{
    return key.isEmpty() ? nullptr : lookup(m_root, key, false);
}

bool IDOSTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    IDOSTreeItem* item = this->item(index);
    if (item == nullptr || item->model() != this || !item->provider()->setData(value, role))
    {
        return false;
    }
    if (role == Qt::CheckStateRole)
    {
        emit checkStateChanged(item, static_cast<Qt::CheckState>(value.toInt()));
        refreshCheckStates();
    }
    else
    {
        QVector<int> roles;
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            roles << Qt::DisplayRole << Qt::EditRole << Qt::ToolTipRole;
        }
        else
        {
            roles.append(role);
        }
        emit dataChanged(index, index, roles);
    }
    return true;
}

bool IDOSTreeModel::setChecked(IDOSTreeItem* item, Qt::CheckState state)
{
    return setData(this->index(item), state, Qt::CheckStateRole);
}

void IDOSTreeModel::addObject(IDOSDataObject* data)
{
    if (m_registry == nullptr || data == nullptr)
    {
        return;
    }
    IDOSTreeItem* old = find(QStringLiteral("object:") + data->objectId());
    const IDOSTreeItemProviderMetadata* metadata = nullptr;
    for (const IDOSTreeItemProviderMetadata* candidate : m_registry->metadata())
    {
        if (candidate->treeName() == m_name && candidate->supports(data))
        {
            metadata = candidate;
            break;
        }
    }
    if (metadata == nullptr)
    {
        if (old != nullptr)
        {
            removeItem(old);
        }
        return;
    }
    IDOSTreeItem* parent = find(metadata->groupKey());
    if (parent == nullptr)
    {
        parent = m_root;
    }
    if (old != nullptr && old->parent() == parent && old->provider() != nullptr &&
        old->provider()->object() == data && old->provider()->metadataId() == metadata->id())
    {
        old->provider()->setName(data->name());
        refreshItem(old);
        return;
    }
    const int row = old != nullptr && old->parent() == parent ? old->row() : parent->childCount();
    IDOSTreeItemProvider* provider = metadata->create(data);
    if (provider == nullptr)
    {
        return;
    }
    if (!provider->setMetadataId(metadata->id()))
    {
        delete provider;
        return;
    }
    IDOSTreeItem* item = new IDOSTreeItem(provider);
    provider->setKey(QStringLiteral("object:") + data->objectId());
    provider->setDataId(data->objectId());
    provider->setName(data->name());
    provider->refresh();
    if (old != nullptr)
    {
        removeItem(old);
    }
    if (!insertItem(parent, item, row))
    {
        delete item;
    }
}

void IDOSTreeModel::onMetadataRegistered(const QString& typeId)
{
    Q_UNUSED(typeId);
    emit registryChanged();
}

void IDOSTreeModel::removeMetadataItems(const QString& typeId)
{
    QList<IDOSTreeItem*> pending;
    pending.append(m_root);
    while (!pending.isEmpty())
    {
        IDOSTreeItem* item = pending.takeLast();
        IDOSTreeItemProvider* provider = item->provider();
        if (provider != nullptr && provider->metadataId() == typeId)
        {
            removeItem(item);
            continue;
        }
        for (IDOSTreeItem* child : item->children())
        {
            pending.append(child);
        }
    }
}

void IDOSTreeModel::onMetadataRemoving(const QString& typeId)
{
    removeMetadataItems(typeId);
}

QString IDOSTreeModel::itemPath(const IDOSTreeItem* item)
{
    QStringList path;
    for (const IDOSTreeItem* current = item;
         current != nullptr && current->parent() != nullptr;
         current = current->parent())
    {
        path.prepend(QString::fromLatin1(current->provider()->key().toUtf8().toHex()));
    }
    return path.join(QLatin1Char('/'));
}

IDOSTreeItem* IDOSTreeModel::itemFromPath(IDOSTreeModel* model, const QString& path)
{
    if (model == nullptr || path.isEmpty())
    {
        return nullptr;
    }
    IDOSTreeItem* current = model->root();
    const QStringList parts = path.split(QLatin1Char('/'));
    for (const QString& part : parts)
    {
        IDOSTreeItem* found = nullptr;
        for (IDOSTreeItem* child : current->children())
        {
            if (QString::fromLatin1(child->provider()->key().toUtf8().toHex()) == part)
            {
                found = child;
                break;
            }
        }
        if (found == nullptr)
        {
            return nullptr;
        }
        current = found;
    }
    return current;
}

QModelIndex IDOSTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0 || row < 0 || parent.column() > 0 ||
        (parent.isValid() && parent.model() != this))
    {
        return QModelIndex();
    }
    IDOSTreeItem* parentItem = parent.isValid() ? this->item(parent) : m_root;
    IDOSTreeItem* item = parentItem->child(row);
    return item != nullptr ? createIndex(row, column, item) : QModelIndex();
}

QModelIndex IDOSTreeModel::parent(const QModelIndex& child) const
{
    IDOSTreeItem* item = this->item(child);
    return item != nullptr ? index(item->parent()) : QModelIndex();
}

int IDOSTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0 || (parent.isValid() && parent.model() != this))
    {
        return 0;
    }
    IDOSTreeItem* item = parent.isValid() ? this->item(parent) : m_root;
    return item->childCount();
}

int IDOSTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant IDOSTreeModel::data(const QModelIndex& index, int role) const
{
    IDOSTreeItem* item = this->item(index);
    if (item == nullptr)
    {
        return QVariant();
    }
    return item->provider()->data(role);
}

Qt::ItemFlags IDOSTreeModel::flags(const QModelIndex& index) const
{
    IDOSTreeItem* item = this->item(index);
    if (item == nullptr)
    {
        return Qt::ItemIsDropEnabled;
    }
    return item->provider()->flags();
}

IDOSTreeItem* IDOSTreeModel::item(const QModelIndex& index) const
{
    return index.isValid() && index.model() == this ? static_cast<IDOSTreeItem*>(index.internalPointer()) : nullptr;
}

QModelIndex IDOSTreeModel::index(const IDOSTreeItem* item) const
{
    if (m_resetting || item == nullptr || item == m_root || item->model() != this)
    {
        return QModelIndex();
    }
    return createIndex(item->row(), 0, const_cast<IDOSTreeItem*>(item));
}

IDOSTreeItem* IDOSTreeModel::findData(IDOSTreeItem* branch, const QString& id) const
{
    if (branch->provider()->dataId() == id && !branch->provider()->isGroup())
    {
        return branch;
    }
    for (IDOSTreeItem* child : branch->children())
    {
        IDOSTreeItem* result = findData(child, id);
        if (result != nullptr)
        {
            return result;
        }
    }
    return nullptr;
}

QModelIndex IDOSTreeModel::index(const QString& id) const
{
    return !id.isEmpty() ? this->index(findData(m_root, id)) : QModelIndex();
}

IDOSDataObject* IDOSTreeModel::object(const QModelIndex& index) const
{
    IDOSTreeItem* item = this->item(index);
    if (item == nullptr)
    {
        return nullptr;
    }
    const QString dataId = item->provider()->dataId();
    for (IDOSTreeItem* current = item; current != nullptr; current = current->parent())
    {
        IDOSDataObject* data = current->provider()->object();
        if (data != nullptr && data->objectId() == dataId)
        {
            return data;
        }
    }
    return nullptr;
}

bool IDOSTreeModel::isInserting(const IDOSTreeItem* parent, const IDOSTreeItem* item, int row) const
{
    return m_changing && m_committing && m_inserting && m_changeParent == parent &&
        m_changeItem == item && m_changeRow == row;
}

bool IDOSTreeModel::isRemoving(const IDOSTreeItem* parent, const IDOSTreeItem* item) const
{
    return m_changing && m_committing && !m_inserting && m_changeParent == parent && m_changeItem == item;
}

void IDOSTreeModel::forgetRefreshItems(IDOSTreeItem* item)
{
    m_refreshItems.remove(item);
    for (IDOSTreeItem* child : item->children())
    {
        forgetRefreshItems(child);
    }
}

void IDOSTreeModel::collectRefreshItems(IDOSTreeItem* item, QList<IDOSTreeItem*>& items)
{
    for (IDOSTreeItem* child : item->children())
    {
        collectRefreshItems(child, items);
        items.append(child);
        m_refreshItems.insert(child);
    }
}

void IDOSTreeModel::retainItem(IDOSTreeItem* item)
{
    m_refreshItems.remove(item);
}

void IDOSTreeModel::refreshItem(IDOSTreeItem* item)
{
    if (item == nullptr || item->model() != this || m_refreshing)
    {
        return;
    }
    QScopedValueRollback<bool> refreshing(m_refreshing, true);
    QList<IDOSTreeItem*> previous;
    collectRefreshItems(item, previous);
    item->provider()->refresh();
    for (IDOSTreeItem* child : previous)
    {
        if (m_refreshItems.remove(child))
        {
            child->parent()->removeChild(child);
        }
    }
    m_refreshItems.clear();
    QList<IDOSTreeItem*> pending;
    pending.append(item);
    while (!pending.isEmpty())
    {
        IDOSTreeItem* current = pending.takeLast();
        updateItem(current);
        pending.append(current->children());
    }
}

void IDOSTreeModel::updateItem(IDOSTreeItem* item, const QList<int>& roles)
{
    const QModelIndex index = this->index(item);
    if (index.isValid())
    {
        emit dataChanged(index, index, roles.toVector());
    }
}

void IDOSTreeModel::setCheckState(IDOSTreeItem* item, Qt::CheckState state, bool checkable)
{
    if (item == nullptr || item->model() != this || item->provider()->isGroup())
    {
        return;
    }
    item->provider()->setCheckEnabled(checkable);
    item->provider()->setChecked(state);
}

QString IDOSTreeModel::sessionId() const
{
    return m_sessionId;
}

void IDOSTreeModel::setSessionId(const QString& sessionId)
{
    m_sessionId = sessionId;
}

QList<IDOSDataObject*> IDOSTreeModel::objects() const
{
    QList<IDOSDataObject*> objects;
    QList<IDOSTreeItem*> pending;
    pending.append(m_root);
    while (!pending.isEmpty())
    {
        IDOSTreeItem* item = pending.takeLast();
        IDOSDataObject* data = item->provider()->object();
        if (data != nullptr && !objects.contains(data))
        {
            objects.append(data);
        }
        pending.append(item->children());
    }
    return objects;
}

IDOSDataObject* IDOSTreeModel::object(const QString& dataId) const
{
    for (IDOSDataObject* data : objects())
    {
        if (data->objectId() == dataId)
        {
            return data;
        }
    }
    return nullptr;
}

bool IDOSTreeModel::acceptsContext(const QString& session, const QString& itemId) const
{
    return !m_sessionId.isEmpty() && session == m_sessionId &&
        (itemId.isEmpty() || item(itemId) != nullptr);
}

bool IDOSTreeModel::transferItems(const QVariantMap& arguments, bool validateOnly)
{
    const QString targetId = arguments.value(QStringLiteral("targetItemId")).toString();
    if (m_sessionId.isEmpty() || (!targetId.isEmpty() && item(targetId) == nullptr))
    {
        return false;
    }
    bool accepted = false;
    emit transferRequested(arguments, validateOnly, accepted);
    return accepted;
}
