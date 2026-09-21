#include "idosmodeltreemodel.h"
#include "idosproject.h"
#include "data/idosdataobject.h"
#include "data/grid/idosgrid.h"

IDOSModelTreeModel::IDOSModelTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_project(nullptr)
    , m_rootNode(new IDOSModelTreeNode())
{
}

IDOSModelTreeModel::~IDOSModelTreeModel()
{
    delete m_rootNode;
}

void IDOSModelTreeModel::setProject(IDOSProject* project)
{
    if (m_project != nullptr)
    {
        disconnect(m_project, &IDOSProject::objectAdded,
                   this, &IDOSModelTreeModel::onObjectAdded);
        disconnect(m_project, &IDOSProject::objectRemoved,
                   this, &IDOSModelTreeModel::onObjectRemoved);
        disconnect(m_project, &IDOSProject::objectChanged,
                   this, &IDOSModelTreeModel::onObjectChanged);
    }

    m_project = project;

    if (m_project != nullptr)
    {
        connect(m_project, &IDOSProject::objectAdded,
                this, &IDOSModelTreeModel::onObjectAdded);
        connect(m_project, &IDOSProject::objectRemoved,
                this, &IDOSModelTreeModel::onObjectRemoved);
        connect(m_project, &IDOSProject::objectChanged,
                this, &IDOSModelTreeModel::onObjectChanged);
    }

    beginResetModel();
    rebuildTree();
    endResetModel();
}

// ===== QAbstractItemModel =====

QModelIndex IDOSModelTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();

    IDOSTreeNode* parentNode = parent.isValid()
        ? static_cast<IDOSTreeNode*>(parent.internalPointer())
        : m_rootNode;

    auto* childNode = parentNode->child(row);
    if (childNode != nullptr)
    {
        return createIndex(row, column, childNode);
    }
    return QModelIndex();
}

QModelIndex IDOSModelTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) return QModelIndex();

    auto* childNode = static_cast<IDOSTreeNode*>(child.internalPointer());
    IDOSTreeNode* parentNode = childNode->parent();
    if (parentNode == nullptr || parentNode == m_rootNode) return QModelIndex();

    return createIndex(parentNode->row(), 0, parentNode);
}

int IDOSModelTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0) return 0;

    IDOSTreeNode* parentNode = parent.isValid()
        ? static_cast<IDOSTreeNode*>(parent.internalPointer())
        : m_rootNode;

    return parentNode->childCount();
}

int IDOSModelTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant IDOSModelTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();

    auto* node = static_cast<IDOSTreeNode*>(index.internalPointer());

    switch (role)
    {
    case Qt::DisplayRole:
        return node->name();
    case Qt::DecorationRole:
        return node->icon();
    case Qt::CheckStateRole:
    {
        auto* modelNode = dynamic_cast<IDOSModelTreeNode*>(node);
        if (modelNode != nullptr && !modelNode->objectId().isEmpty() && m_project != nullptr)
        {
            auto* obj = m_project->objectById(modelNode->objectId());
            if (obj != nullptr)
            {
                return obj->isVisible() ? Qt::Checked : Qt::Unchecked;
            }
        }
        break;
    }
    case Qt::ToolTipRole:
        return node->name();
    default:
        break;
    }
    return QVariant();
}

QVariant IDOSModelTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section); Q_UNUSED(orientation); Q_UNUSED(role);
    return QVariant();
}

Qt::ItemFlags IDOSModelTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    auto* node = static_cast<IDOSTreeNode*>(index.internalPointer());
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // 非分组节点带复选框
    auto* modelNode = dynamic_cast<IDOSModelTreeNode*>(node);
    if (modelNode != nullptr && !modelNode->objectId().isEmpty())
    {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

IDOSModelTreeNode* IDOSModelTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) return nullptr;
    return static_cast<IDOSModelTreeNode*>(index.internalPointer());
}

IDOSDataObject* IDOSModelTreeModel::objectFromIndex(const QModelIndex& index) const
{
    auto* node = nodeFromIndex(index);
    if (node == nullptr || m_project == nullptr) return nullptr;
    if (node->objectId().isEmpty()) return nullptr;
    return m_project->objectById(node->objectId());
}

// ===== 内部 =====

void IDOSModelTreeModel::rebuildTree()
{
    m_rootNode->clearChildren();

    if (m_project == nullptr) return;

    // 只收集 IDOSGrid
    for (auto* obj : m_project->objects())
    {
        if (qobject_cast<IDOSGrid*>(obj) == nullptr) continue;

        auto* node = new IDOSModelTreeNode(m_rootNode);
        node->setObjectId(obj->objectId());
        node->setName(obj->name());
        m_rootNode->appendChild(node);
    }
}

IDOSModelTreeNode* IDOSModelTreeModel::findNodeByObjectId(const QString& objectId) const
{
    for (int i = 0; i < m_rootNode->childCount(); ++i)
    {
        auto* node = dynamic_cast<IDOSModelTreeNode*>(m_rootNode->child(i));
        if (node != nullptr && node->objectId() == objectId)
        {
            return node;
        }
    }
    return nullptr;
}

// ===== Project 信号处理 =====

void IDOSModelTreeModel::onObjectAdded(const QString& objectId)
{
    if (m_project == nullptr) return;
    auto* obj = m_project->objectById(objectId);
    if (obj == nullptr) return;
    // 只关心 Grid
    if (qobject_cast<IDOSGrid*>(obj) == nullptr) return;

    int row = m_rootNode->childCount();
    beginInsertRows(QModelIndex(), row, row);

    auto* node = new IDOSModelTreeNode(m_rootNode);
    node->setObjectId(objectId);
    node->setName(obj->name());
    m_rootNode->appendChild(node);

    endInsertRows();
}

void IDOSModelTreeModel::onObjectRemoved(const QString& objectId)
{
    auto* node = findNodeByObjectId(objectId);
    if (node == nullptr) return;

    int row = node->row();
    beginRemoveRows(QModelIndex(), row, row);
    m_rootNode->removeChild(row);
    endRemoveRows();
}

void IDOSModelTreeModel::onObjectChanged(const QString& objectId)
{
    if (m_project == nullptr) return;

    auto* node = findNodeByObjectId(objectId);
    if (node == nullptr) return;

    auto* obj = m_project->objectById(objectId);
    if (obj != nullptr)
    {
        node->setName(obj->name());
    }

    QModelIndex index = createIndex(node->row(), 0, node);
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::CheckStateRole, Qt::ToolTipRole});
}
