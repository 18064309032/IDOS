#include "idosdataobject.h"
#include "idosproject.h"
#include "idostree.h"
#include "idostreeitem.h"

#include "idostreemodel.h"

IDOSTreeModel::IDOSTreeModel(IDOSTree* tree, QObject* parent)
    :
    QAbstractItemModel(parent)
    , m_tree(tree)
{
    if (tree != nullptr)
    {
        connect(tree, &IDOSTree::aboutToInsert, this, &IDOSTreeModel::onAboutToInsert);
        connect(tree, &IDOSTree::inserted, this, &IDOSTreeModel::onInserted);
        connect(tree, &IDOSTree::aboutToRemove, this, &IDOSTreeModel::onAboutToRemove);
        connect(tree, &IDOSTree::removed, this, &IDOSTreeModel::onRemoved);
        connect(tree, &IDOSTree::itemChanged, this, &IDOSTreeModel::onItemChanged);
        connect(tree, &QObject::destroyed, this, &IDOSTreeModel::onTreeDestroyed);
        connect(tree, &IDOSTree::projectChanged, this, &IDOSTreeModel::onProjectChanged);
    }
}

IDOSTreeModel::~IDOSTreeModel()
{
}

IDOSTree* IDOSTreeModel::tree() const
{
    return m_tree.data();
}

IDOSProject* IDOSTreeModel::project() const
{
    return m_tree != nullptr ? m_tree->project() : nullptr;
}

QModelIndex IDOSTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (m_tree == nullptr || column != 0 || row < 0 || parent.column() > 0 ||
        (parent.isValid() && parent.model() != this))
    {
        return QModelIndex();
    }
    IDOSTreeItem* parentItem = parent.isValid() ? nodeFromIndex(parent) : m_tree->root();
    IDOSTreeItem* item = parentItem->child(row);
    return item != nullptr ? createIndex(row, column, item) : QModelIndex();
}

QModelIndex IDOSTreeModel::parent(const QModelIndex& child) const
{
    IDOSTreeItem* item = nodeFromIndex(child);
    return item != nullptr ? indexFromNode(item->parent()) : QModelIndex();
}

int IDOSTreeModel::rowCount(const QModelIndex& parent) const
{
    if (m_tree == nullptr || parent.column() > 0 || (parent.isValid() && parent.model() != this))
    {
        return 0;
    }
    IDOSTreeItem* item = parent.isValid() ? nodeFromIndex(parent) : m_tree->root();
    return item->childCount();
}

int IDOSTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant IDOSTreeModel::data(const QModelIndex& index, int role) const
{
    IDOSTreeItem* item = nodeFromIndex(index);
    if (item == nullptr)
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
    {
        return item->name();
    }
    if (role == Qt::DecorationRole)
    {
        return item->icon();
    }
    if (role == Qt::CheckStateRole && item->isCheckable())
    {
        return item->isChecked() ? Qt::Checked : Qt::Unchecked;
    }
    return QVariant();
}

bool IDOSTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    IDOSTreeItem* item = nodeFromIndex(index);
    if (item == nullptr || role != Qt::CheckStateRole || !item->isCheckable())
    {
        return false;
    }
    bool accepted = false;
    emit checkRequested(item, value.toInt() == Qt::Checked, &accepted);
    return accepted;
}

Qt::ItemFlags IDOSTreeModel::flags(const QModelIndex& index) const
{
    IDOSTreeItem* item = nodeFromIndex(index);
    if (item == nullptr)
    {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (item->isCheckable())
    {
        result |= Qt::ItemIsUserCheckable;
    }
    return result;
}

IDOSTreeItem* IDOSTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    return index.isValid() && index.model() == this ? static_cast<IDOSTreeItem*>(index.internalPointer()) : nullptr;
}

QModelIndex IDOSTreeModel::indexFromNode(const IDOSTreeItem* item) const
{
    if (m_tree == nullptr || item == nullptr || item == m_tree->root() || item->tree() != m_tree)
    {
        return QModelIndex();
    }
    return createIndex(item->row(), 0, const_cast<IDOSTreeItem*>(item));
}

IDOSTreeItem* IDOSTreeModel::findData(IDOSTreeItem* branch, const QString& id) const
{
    if (branch->dataId() == id && !branch->isGroup())
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

QModelIndex IDOSTreeModel::indexFromDataId(const QString& id) const
{
    return m_tree != nullptr && !id.isEmpty() ? indexFromNode(findData(m_tree->root(), id)) : QModelIndex();
}

IDOSDataObject* IDOSTreeModel::objectFromIndex(const QModelIndex& index) const
{
    IDOSTreeItem* item = nodeFromIndex(index);
    return item != nullptr && project() != nullptr ? project()->object(item->dataId()) : nullptr;
}

void IDOSTreeModel::onAboutToInsert(IDOSTreeItem* parent, int row)
{
    beginInsertRows(indexFromNode(parent), row, row);
}

void IDOSTreeModel::onInserted()
{
    endInsertRows();
}

void IDOSTreeModel::onAboutToRemove(IDOSTreeItem* parent, int row)
{
    beginRemoveRows(indexFromNode(parent), row, row);
}

void IDOSTreeModel::onRemoved()
{
    endRemoveRows();
}

void IDOSTreeModel::onItemChanged(IDOSTreeItem* item)
{
    const QModelIndex index = indexFromNode(item);
    if (index.isValid())
    {
        emit dataChanged(index, index);
    }
}

void IDOSTreeModel::onTreeDestroyed()
{
    beginResetModel();
    m_tree = nullptr;
    endResetModel();
}

void IDOSTreeModel::onProjectChanged()
{
    beginResetModel();
    endResetModel();
}
