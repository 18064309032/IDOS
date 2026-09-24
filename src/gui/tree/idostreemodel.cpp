#include "tree/idostreemodel.h"
#include "tree/idosobjecttreenode.h"
#include "idosproject.h"
#include "data/idosdataobject.h"

IDOSTreeModel::IDOSTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_project(nullptr)
    , m_rootNode(new IDOSTreeNode())
{
}

IDOSTreeModel::~IDOSTreeModel()
{
    delete m_rootNode;
}

void IDOSTreeModel::setProject(IDOSProject* project)
{
    if (m_project != nullptr)
    {
        disconnect(m_project, &IDOSProject::objectAdded, this, &IDOSTreeModel::onObjectAdded);
        disconnect(m_project, &IDOSProject::objectRemoved, this, &IDOSTreeModel::onObjectRemoved);
        disconnect(m_project, &IDOSProject::objectChanged, this, &IDOSTreeModel::onObjectChanged);
    }

    m_project = project;

    if (m_project != nullptr)
    {
        connect(m_project, &IDOSProject::objectAdded, this, &IDOSTreeModel::onObjectAdded);
        connect(m_project, &IDOSProject::objectRemoved, this, &IDOSTreeModel::onObjectRemoved);
        connect(m_project, &IDOSProject::objectChanged, this, &IDOSTreeModel::onObjectChanged);
    }

    beginResetModel();
    rebuildTree();
    endResetModel();
}

IDOSProject* IDOSTreeModel::project() const
{
    return m_project;
}

QModelIndex IDOSTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();
    auto* parentNode = parent.isValid() ? static_cast<IDOSTreeNode*>(parent.internalPointer()) : m_rootNode;
    auto* childNode = parentNode->child(row);
    return childNode != nullptr ? createIndex(row, column, childNode) : QModelIndex();
}

QModelIndex IDOSTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return QModelIndex();
    auto* childNode = static_cast<IDOSTreeNode*>(child.internalPointer());
    IDOSTreeNode* parentNode = childNode->parent();
    if (parentNode == nullptr || parentNode == m_rootNode) return QModelIndex();
    return createIndex(parentNode->row(), 0, parentNode);
}

int IDOSTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0) return 0;
    auto* parentNode = parent.isValid() ? static_cast<IDOSTreeNode*>(parent.internalPointer()) : m_rootNode;
    return parentNode->childCount();
}

int IDOSTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant IDOSTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    auto* node = static_cast<IDOSTreeNode*>(index.internalPointer());
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) return node->name();
    if (role == Qt::DecorationRole) return node->icon();
    return QVariant();
}

QVariant IDOSTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

Qt::ItemFlags IDOSTreeModel::flags(const QModelIndex& index) const
{
    return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}

IDOSTreeNode* IDOSTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    return index.isValid() ? static_cast<IDOSTreeNode*>(index.internalPointer()) : nullptr;
}

IDOSDataObject* IDOSTreeModel::objectFromIndex(const QModelIndex& index) const
{
    if (m_project == nullptr) return nullptr;
    auto* objectNode = dynamic_cast<IDOSObjectTreeNode*>(nodeFromIndex(index));
    return objectNode != nullptr ? m_project->objectById(objectNode->objectId()) : nullptr;
}

void IDOSTreeModel::rebuildTree()
{
    m_rootNode->clearChildren();
    if (m_project == nullptr) return;
    for (auto* object : m_project->objects())
    {
        if (shouldShowObject(object)) buildObjectTree(m_rootNode, object);
    }
}

void IDOSTreeModel::onObjectAdded(const QString& objectId)
{
    Q_UNUSED(objectId);
    beginResetModel();
    rebuildTree();
    endResetModel();
}

void IDOSTreeModel::onObjectRemoved(const QString& objectId)
{
    Q_UNUSED(objectId);
    beginResetModel();
    rebuildTree();
    endResetModel();
}

void IDOSTreeModel::onObjectChanged(const QString& objectId)
{
    Q_UNUSED(objectId);
    beginResetModel();
    rebuildTree();
    endResetModel();
}
