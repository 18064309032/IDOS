#include "idosdataobject.h"
#include "idosgridproperty.h"
#include "idosobjecttreenode.h"
#include "idosproject.h"
#include "idostreereferencenode.h"

#include "idostreemodel.h"

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
        disconnect(m_project, &IDOSProject::objectDataChanged, this, &IDOSTreeModel::onObjectDataChanged);
        disconnect(m_project, &IDOSProject::objectsAdded, this, &IDOSTreeModel::onObjectsAdded);
        disconnect(m_project, &IDOSProject::objectsRemoved, this, &IDOSTreeModel::onObjectsRemoved);
        disconnect(m_project, &IDOSProject::objectsDataChanged, this, &IDOSTreeModel::onObjectsDataChanged);
        disconnect(m_project, &IDOSProject::objectVisibilityChanged,
                   this, &IDOSTreeModel::onObjectVisibilityChanged);
        disconnect(m_project, &IDOSProject::objectsVisibilityChanged,
                   this, &IDOSTreeModel::onObjectsVisibilityChanged);
    }

    m_project = project;

    if (m_project != nullptr)
    {
        connect(m_project, &IDOSProject::objectAdded, this, &IDOSTreeModel::onObjectAdded);
        connect(m_project, &IDOSProject::objectRemoved, this, &IDOSTreeModel::onObjectRemoved);
        connect(m_project, &IDOSProject::objectDataChanged, this, &IDOSTreeModel::onObjectDataChanged);
        connect(m_project, &IDOSProject::objectsAdded, this, &IDOSTreeModel::onObjectsAdded);
        connect(m_project, &IDOSProject::objectsRemoved, this, &IDOSTreeModel::onObjectsRemoved);
        connect(m_project, &IDOSProject::objectsDataChanged, this, &IDOSTreeModel::onObjectsDataChanged);
        connect(m_project, &IDOSProject::objectVisibilityChanged,
                this, &IDOSTreeModel::onObjectVisibilityChanged);
        connect(m_project, &IDOSProject::objectsVisibilityChanged,
                this, &IDOSTreeModel::onObjectsVisibilityChanged);
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
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    IDOSTreeNode* parentNode = parent.isValid() ? static_cast<IDOSTreeNode*>(parent.internalPointer()) : m_rootNode;
    IDOSTreeNode* childNode = parentNode->child(row);
    return childNode != nullptr ? createIndex(row, column, childNode) : QModelIndex();
}

QModelIndex IDOSTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
    {
        return QModelIndex();
    }
    IDOSTreeNode* childNode = static_cast<IDOSTreeNode*>(child.internalPointer());
    IDOSTreeNode* parentNode = childNode->parent();
    if (parentNode == nullptr || parentNode == m_rootNode)
    {
        return QModelIndex();
    }
    return createIndex(parentNode->row(), 0, parentNode);
}

int IDOSTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }
    IDOSTreeNode* parentNode = parent.isValid() ? static_cast<IDOSTreeNode*>(parent.internalPointer()) : m_rootNode;
    return parentNode->childCount();
}

int IDOSTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant IDOSTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    IDOSTreeNode* node = static_cast<IDOSTreeNode*>(index.internalPointer());
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
    {
        return node->name();
    }
    if (role == Qt::DecorationRole)
    {
        return node->icon();
    }
    if (role == Qt::CheckStateRole && node->isCheckable())
    {
        IDOSDataObject* object = objectOfNode(node);
        const bool checked = object != nullptr ? object->isVisible() : node->isChecked();
        return checked ? Qt::Checked : Qt::Unchecked;
    }
    return QVariant();
}

bool IDOSTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::CheckStateRole)
    {
        return false;
    }

    IDOSTreeNode* node = static_cast<IDOSTreeNode*>(index.internalPointer());
    if (node == nullptr || !node->isCheckable())
    {
        return false;
    }

    if (m_project != nullptr)
    {
        m_project->beginUpdate();
    }
    const bool checked = value.toInt() == Qt::Checked;
    IDOSDataObject* currentObject = objectOfNode(node);
    node->setChecked(checked);
    emit dataChanged(index, index, QVector<int>() << Qt::CheckStateRole);
    if (currentObject != nullptr && currentObject->isVisible() != checked)
    {
        currentObject->setVisible(checked);
    }

    // 勾选网格属性节点时，取消其他已勾选属性（单选互斥），
    // 避免渲染窗口多个属性颜色映射互相覆盖。
    if (checked)
    {
        if (qobject_cast<const IDOSGridProperty*>(currentObject) != nullptr)
        {
            uncheckOtherProperties(node);
        }
    }
    if (m_project != nullptr)
    {
        m_project->endUpdate();
    }
    return true;
}

void IDOSTreeModel::uncheckOtherProperties(IDOSTreeNode* exceptNode)
{
    if (m_rootNode == nullptr)
    {
        return;
    }

    QList<IDOSTreeNode*> toUncheck;
    collectCheckedProperties(m_rootNode, exceptNode, toUncheck);

    for (IDOSTreeNode* node : toUncheck)
    {
        node->setChecked(false);
        QModelIndex idx = indexOfNode(node);
        if (idx.isValid())
        {
            emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
        }
        IDOSDataObject* object = objectOfNode(node);
        if (object != nullptr && object->isVisible())
        {
            object->setVisible(false);
        }
    }
}

void IDOSTreeModel::collectCheckedProperties(IDOSTreeNode* branch,
                                                     IDOSTreeNode* exceptNode,
                                                     QList<IDOSTreeNode*>& out) const
{
    if (branch == nullptr)
    {
        return;
    }

    for (int i = 0; i < branch->childCount(); ++i)
    {
        IDOSTreeNode* child = branch->child(i);
        if (child == nullptr)
        {
            continue;
        }

        // 命中已勾选、非当前节点、且对应网格属性的节点
        if (child != exceptNode && child->isCheckable() && child->isChecked())
        {
            IDOSDataObject* obj = objectOfNode(child);
            if (qobject_cast<const IDOSGridProperty*>(obj) != nullptr)
            {
                out.append(child);
            }
        }

        collectCheckedProperties(child, exceptNode, out);
    }
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
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    IDOSTreeNode* node = static_cast<IDOSTreeNode*>(index.internalPointer());
    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (node != nullptr && node->isCheckable())
    {
        itemFlags |= Qt::ItemIsUserCheckable;
    }
    return itemFlags;
}

IDOSTreeNode* IDOSTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    return index.isValid() ? static_cast<IDOSTreeNode*>(index.internalPointer()) : nullptr;
}

IDOSDataObject* IDOSTreeModel::objectFromIndex(const QModelIndex& index) const
{
    return objectOfNode(nodeFromIndex(index));
}

QModelIndex IDOSTreeModel::indexFromObjectId(const QString& objectId) const
{
    IDOSObjectTreeNode* objectNode = findObjectNode(objectId);
    return indexOfNode(objectNode);
}

IDOSDataObject* IDOSTreeModel::objectOfNode(IDOSTreeNode* node) const
{
    if (m_project == nullptr || node == nullptr)
    {
        return nullptr;
    }
    // 数据本体树：ObjectTreeNode 直接持 objectId
    IDOSObjectTreeNode* objectNode = dynamic_cast<IDOSObjectTreeNode*>(node);
    if (objectNode != nullptr)
    {
        return m_project->objectById(objectNode->objectId());
    }
    // 工况引用树：ReferenceNode 通过 IDOSCaseItemRef 间接引用 objectId
    IDOSTreeReferenceNode* refNode = dynamic_cast<IDOSTreeReferenceNode*>(node);
    if (refNode != nullptr)
    {
        return m_project->objectById(refNode->itemRef().objectId());
    }
    return nullptr;
}

void IDOSTreeModel::rebuildTree()
{
    m_rootNode->clearChildren();
    if (m_project == nullptr)
    {
        return;
    }
    buildDefaultTree(m_rootNode);
    for (IDOSDataObject* object : m_project->objects())
    {
        if (shouldShowObject(object))
        {
            IDOSTreeNode* parent = parentNodeForNewObject(object);
            buildObjectTree(parent != nullptr ? parent : m_rootNode, object);
        }
    }
}

void IDOSTreeModel::buildDefaultTree(IDOSTreeNode* rootNode)
{
    Q_UNUSED(rootNode);
}

IDOSTreeNode* IDOSTreeModel::parentNodeForNewObject(const IDOSDataObject* object) const
{
    Q_UNUSED(object);
    return m_rootNode;
}

void IDOSTreeModel::refreshReferencingBranches(const QString& objectId, bool objectRemoved)
{
    Q_UNUSED(objectId);
    Q_UNUSED(objectRemoved);
}

void IDOSTreeModel::refreshReferencingBranchesBatch(const QStringList& changedIds,
                                                    const QStringList& removedIds)
{
    // 默认退化为逐个处理，子类（工况树）重写为聚合算法
    for (const QString& id : removedIds)
    {
        refreshReferencingBranches(id, true);
    }
    for (const QString& id : changedIds)
    {
        refreshReferencingBranches(id, false);
    }
}

IDOSObjectTreeNode* IDOSTreeModel::findObjectNode(const QString& objectId) const
{
    if (objectId.isEmpty())
    {
        return nullptr;
    }
    return findObjectNodeRecursive(m_rootNode, objectId);
}

IDOSObjectTreeNode* IDOSTreeModel::findObjectNodeRecursive(IDOSTreeNode* branch,
                                                           const QString& objectId) const
{
    if (branch == nullptr)
    {
        return nullptr;
    }
    for (int row = 0; row < branch->childCount(); ++row)
    {
        IDOSTreeNode* child = branch->child(row);
        IDOSObjectTreeNode* objectNode = dynamic_cast<IDOSObjectTreeNode*>(child);
        if (objectNode != nullptr && objectNode->objectId() == objectId)
        {
            return objectNode;
        }
        IDOSObjectTreeNode* nested = findObjectNodeRecursive(child, objectId);
        if (nested != nullptr)
        {
            return nested;
        }
    }
    return nullptr;
}

QModelIndex IDOSTreeModel::indexOfNode(IDOSTreeNode* node) const
{
    if (node == nullptr || node->parent() == nullptr)
    {
        return QModelIndex();
    }
    return createIndex(node->row(), 0, node);
}

QString IDOSTreeModel::checkKeyOf(const IDOSTreeNode* node) const
{
    // 节点稳定键由节点体系自身给出（对象/引用/分组/Part 各带类型前缀）
    return node->nodeKey();
}

void IDOSTreeModel::collectCheckedKeys(IDOSTreeNode* branch, QSet<QString>& checkedKeys) const
{
    if (branch == nullptr)
    {
        return;
    }
    if (branch->isCheckable() && branch->isChecked())
    {
        const QString key = checkKeyOf(branch);
        if (!key.isEmpty())
        {
            checkedKeys.insert(key);
        }
    }
    for (int row = 0; row < branch->childCount(); ++row)
    {
        collectCheckedKeys(branch->child(row), checkedKeys);
    }
}

void IDOSTreeModel::applyCheckedKeys(IDOSTreeNode* branch, const QSet<QString>& checkedKeys) const
{
    if (branch == nullptr)
    {
        return;
    }
    if (branch->isCheckable())
    {
        const QString key = checkKeyOf(branch);
        if (!key.isEmpty() && checkedKeys.contains(key))
        {
            branch->setChecked(true);
        }
    }
    for (int row = 0; row < branch->childCount(); ++row)
    {
        applyCheckedKeys(branch->child(row), checkedKeys);
    }
}

void IDOSTreeModel::rebuildObjectBranch(IDOSObjectTreeNode* objectNode, const IDOSDataObject* object)
{
    if (objectNode == nullptr || object == nullptr)
    {
        return;
    }
    IDOSTreeNode* parent = objectNode->parent();
    if (parent == nullptr)
    {
        return;
    }
    const int row = objectNode->row();
    const QModelIndex parentIndex = parent == m_rootNode ? QModelIndex() : indexOfNode(parent);

    // 快照旧分支的勾选状态，重建后恢复，避免结构刷新清空用户勾选
    QSet<QString> checkedKeys;
    collectCheckedKeys(objectNode, checkedKeys);

    beginRemoveRows(parentIndex, row, row);
    parent->removeChild(row);
    endRemoveRows();

    // 在游离的暂存节点上经 provider 重建，再插回原行以保持分支顺序。
    // 契约：provider 构建一个对象分支时只能在传入节点下挂唯一一个顶层对象节点
    IDOSTreeNode staging;
    buildObjectTree(&staging, object);
    Q_ASSERT(staging.childCount() == 1);
    IDOSTreeNode* freshNode = staging.childCount() > 0 ? staging.takeChild(0) : nullptr;
    if (freshNode == nullptr)
    {
        return;
    }
    applyCheckedKeys(freshNode, checkedKeys);

    beginInsertRows(parentIndex, row, row);
    parent->insertChild(row, freshNode);
    endInsertRows();
}

void IDOSTreeModel::processAddedObject(const QString& objectId)
{
    if (m_project == nullptr)
    {
        return;
    }
    IDOSDataObject* object = m_project->objectById(objectId);
    if (object == nullptr || !shouldShowObject(object))
    {
        return;
    }

    // Project::addObject 支持同 objectId 替换（删旧对象后发 objectAdded）。
    // 树上若已存在同 id 节点，不能再次插入（否则出现重复节点），转为原地重建
    IDOSObjectTreeNode* existingNode = findObjectNode(objectId);
    if (existingNode != nullptr)
    {
        rebuildObjectBranch(existingNode, object);
        return;
    }

    IDOSTreeNode* parent = parentNodeForNewObject(object);
    if (parent == nullptr)
    {
        parent = m_rootNode;
    }
    const int row = parent->childCount();
    const QModelIndex parentIndex = parent == m_rootNode ? QModelIndex() : indexOfNode(parent);

    // provider 只会在 parent 下追加一个对象节点（及其子树），恰好新增一行
    beginInsertRows(parentIndex, row, row);
    buildObjectTree(parent, object);
    Q_ASSERT(parent->childCount() == row + 1);
    endInsertRows();
}

void IDOSTreeModel::processRemovedObject(const QString& objectId)
{
    IDOSObjectTreeNode* objectNode = findObjectNode(objectId);
    if (objectNode == nullptr)
    {
        return;
    }
    IDOSTreeNode* parent = objectNode->parent();
    const int row = objectNode->row();
    const QModelIndex parentIndex = parent == m_rootNode ? QModelIndex() : indexOfNode(parent);
    beginRemoveRows(parentIndex, row, row);
    parent->removeChild(row);
    endRemoveRows();
}

void IDOSTreeModel::processChangedObject(const QString& objectId)
{
    if (m_project == nullptr)
    {
        return;
    }
    IDOSDataObject* object = m_project->objectById(objectId);
    if (object == nullptr)
    {
        return;
    }
    IDOSObjectTreeNode* objectNode = findObjectNode(objectId);
    if (objectNode != nullptr && shouldShowObject(object))
    {
        rebuildObjectBranch(objectNode, object);
    }
}

void IDOSTreeModel::onObjectAdded(const QString& objectId)
{
    processAddedObject(objectId);
    refreshReferencingBranches(objectId, false);
}

void IDOSTreeModel::onObjectRemoved(const QString& objectId)
{
    processRemovedObject(objectId);
    refreshReferencingBranches(objectId, true);
}

void IDOSTreeModel::onObjectDataChanged(const QString& objectId)
{
    processChangedObject(objectId);
    refreshReferencingBranches(objectId, false);
}

void IDOSTreeModel::onObjectsAdded(const QStringList& objectIds)
{
    for (const QString& id : objectIds)
    {
        processAddedObject(id);
    }
    // 引用分支只在所有自身分支落位后统一刷新一次（工况树按工况聚合）
    refreshReferencingBranchesBatch(objectIds, QStringList());
}

void IDOSTreeModel::onObjectsRemoved(const QStringList& objectIds)
{
    for (const QString& id : objectIds)
    {
        processRemovedObject(id);
    }
    refreshReferencingBranchesBatch(QStringList(), objectIds);
}

void IDOSTreeModel::onObjectsDataChanged(const QStringList& objectIds)
{
    for (const QString& id : objectIds)
    {
        processChangedObject(id);
    }
    refreshReferencingBranchesBatch(objectIds, QStringList());
}

void IDOSTreeModel::onObjectVisibilityChanged(const QString& objectId, bool visible)
{
    Q_UNUSED(visible)
    updateCheckStateForObject(objectId);
}

void IDOSTreeModel::onObjectsVisibilityChanged(const QStringList& objectIds)
{
    for (const QString& objectId : objectIds)
    {
        updateCheckStateForObject(objectId);
    }
}

void IDOSTreeModel::updateCheckStateForObject(const QString& objectId)
{
    updateCheckStateForObject(m_rootNode, objectId);
}

void IDOSTreeModel::updateCheckStateForObject(IDOSTreeNode* branch, const QString& objectId)
{
    if (branch == nullptr || objectId.isEmpty())
    {
        return;
    }

    IDOSDataObject* object = objectOfNode(branch);
    if (object != nullptr && object->objectId() == objectId)
    {
        branch->setChecked(object->isVisible());
        const QModelIndex index = indexOfNode(branch);
        if (index.isValid())
        {
            emit dataChanged(index, index, QVector<int>() << Qt::CheckStateRole);
        }
    }

    for (int childIndex = 0; childIndex < branch->childCount(); ++childIndex)
    {
        updateCheckStateForObject(branch->child(childIndex), objectId);
    }
}
