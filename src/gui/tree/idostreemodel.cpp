#include "idostreemodel.h"
#include "idostreeitemprovider.h"
#include "idosproject.h"
#include "idostyperegistry.h"
#include "data/idosdataobject.h"

IDOSTreeModel::IDOSTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_project(nullptr)
    , m_rootNode(new IDOSObjectTreeNode())
{
    // 机制层不注册任何具体类型的 provider——内置 provider 由
    // IDOSInputTreeModel / IDOSModelTreeModel 子类注册，插件运行时注册
}

IDOSTreeModel::~IDOSTreeModel()
{
    qDeleteAll(m_providers);
    delete m_rootNode;
}

void IDOSTreeModel::setProject(IDOSProject* project)
{
    if (m_project != nullptr)
    {
        disconnect(m_project, &IDOSProject::objectAdded,
                   this, &IDOSTreeModel::onObjectAdded);
        disconnect(m_project, &IDOSProject::objectRemoved,
                   this, &IDOSTreeModel::onObjectRemoved);
        disconnect(m_project, &IDOSProject::objectChanged,
                   this, &IDOSTreeModel::onObjectChanged);
    }

    m_project = project;

    if (m_project != nullptr)
    {
        connect(m_project, &IDOSProject::objectAdded,
                this, &IDOSTreeModel::onObjectAdded);
        connect(m_project, &IDOSProject::objectRemoved,
                this, &IDOSTreeModel::onObjectRemoved);
        connect(m_project, &IDOSProject::objectChanged,
                this, &IDOSTreeModel::onObjectChanged);
    }

    beginResetModel();
    rebuildTree();
    endResetModel();
}

void IDOSTreeModel::setTreeScope(TreeScope scope)
{
    if (m_treeScope == scope) return;

    m_treeScope = scope;
    beginResetModel();
    rebuildTree();
    endResetModel();
}

void IDOSTreeModel::registerItemProvider(IDOSTreeItemProvider* provider)
{
    if (provider == nullptr) return;
    m_providers.insert(provider->typeId(), provider);
}

IDOSTreeItemProvider* IDOSTreeModel::providerFor(const QString& typeId) const
{
    return m_providers.value(typeId, nullptr);
}

// ===== QAbstractItemModel =====

QModelIndex IDOSTreeModel::index(int row, int column, const QModelIndex& parent) const
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

    IDOSTreeNode* parentNode = parent.isValid()
        ? static_cast<IDOSTreeNode*>(parent.internalPointer())
        : m_rootNode;

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

    switch (role)
    {
    case Qt::DisplayRole:
        return node->name();
    case Qt::DecorationRole:
        return node->icon();
    case Qt::CheckStateRole:
    {
        // 仅对象节点（绑 objectId）带勾选框；值节点/分组节点无
        auto* objectNode = dynamic_cast<IDOSObjectTreeNode*>(node);
        if (objectNode != nullptr && !objectNode->objectId().isEmpty() && m_project != nullptr)
        {
            auto* obj = m_project->objectById(objectNode->objectId());
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

QVariant IDOSTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section); Q_UNUSED(orientation); Q_UNUSED(role);
    return QVariant();
}

Qt::ItemFlags IDOSTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    auto* node = static_cast<IDOSTreeNode*>(index.internalPointer());
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // 非分组节点带复选框：仅对象节点（绑 objectId）可勾选
    auto* objectNode = dynamic_cast<IDOSObjectTreeNode*>(node);
    if (objectNode != nullptr && !objectNode->objectId().isEmpty())
    {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

IDOSTreeNode* IDOSTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) return nullptr;
    return static_cast<IDOSTreeNode*>(index.internalPointer());
}

IDOSDataObject* IDOSTreeModel::objectFromIndex(const QModelIndex& index) const
{
    if (m_project == nullptr) return nullptr;

    // 仅对象节点对应领域对象；值节点/分组节点返回 nullptr
    auto* node = nodeFromIndex(index);
    auto* objectNode = dynamic_cast<IDOSObjectTreeNode*>(node);
    if (objectNode == nullptr || objectNode->objectId().isEmpty()) return nullptr;
    return m_project->objectById(objectNode->objectId());
}

// ===== 内部 =====

void IDOSTreeModel::rebuildTree()
{
    m_rootNode->clearChildren();
    m_groupNodes.clear();

    if (m_project == nullptr) return;

    // 按 TreeScope 过滤顶层对象，子结构由 provider 决定（静默重建，调用方包裹 reset）
    for (auto* obj : m_project->objects())
    {
        if (!shouldShow(obj)) continue;

        IDOSObjectTreeNode* container = m_rootNode;

        // Input 域按 inputGroup 归组（仿 tNavigator 输入树的固定分类骨架）
        const QString group = groupLabelOf(obj);
        if (!group.isEmpty())
        {
            auto* groupNode = new IDOSObjectTreeNode(m_rootNode);
            groupNode->setName(group);
            m_rootNode->appendChild(groupNode);
            m_groupNodes.insert(group, groupNode);
            container = groupNode;
        }

        auto* node = new IDOSObjectTreeNode(container);
        node->setObjectId(obj->objectId());
        node->setName(obj->name());
        container->appendChild(node);
        buildObjectSubtree(node, obj);
    }
}

bool IDOSTreeModel::shouldShow(IDOSDataObject* obj) const
{
    // 容器子对象（如网格属性）不进顶层，由容器的 provider 子树展示
    if (!obj->containerId().isEmpty()) return false;

    if (m_treeScope == TreeScope::All) return true;

    // 按类型注册表的类别分流；未知类型兜底按 Model 处理
    const auto category = IDOSTypeRegistry::instance().categoryOf(obj->typeId());
    return m_treeScope == TreeScope::Input
        ? category == IDOSObjectCategory::Input
        : category == IDOSObjectCategory::Model;
}

QString IDOSTreeModel::groupLabelOf(IDOSDataObject* obj) const
{
    if (m_treeScope != TreeScope::Input) return QString();

    auto* meta = IDOSTypeRegistry::instance().metadata(obj->typeId());
    return meta != nullptr ? meta->inputGroup() : QString();
}

IDOSObjectTreeNode* IDOSTreeModel::findNodeByObjectId(const QString& objectId) const
{
    return findNodeByObjectId(m_rootNode, objectId);
}

IDOSObjectTreeNode* IDOSTreeModel::findNodeByObjectId(IDOSTreeNode* branch,
                                                      const QString& objectId) const
{
    for (int i = 0; i < branch->childCount(); ++i)
    {
        IDOSTreeNode* child = branch->child(i);

        auto* objectNode = dynamic_cast<IDOSObjectTreeNode*>(child);
        if (objectNode != nullptr && objectNode->objectId() == objectId)
        {
            return objectNode;
        }

        // 递归搜子树（值节点通过 ownerObjectId 引用对象，但对象节点才是查找目标）
        auto* deeper = findNodeByObjectId(child, objectId);
        if (deeper != nullptr)
        {
            return deeper;
        }
    }
    return nullptr;
}

void IDOSTreeModel::buildObjectSubtree(IDOSObjectTreeNode* node, IDOSDataObject* obj)
{
    auto* provider = providerFor(obj->typeId());
    if (provider != nullptr)
    {
        provider->buildChildren(this, node, obj);
    }
}

void IDOSTreeModel::rebuildSubtree(IDOSObjectTreeNode* node, IDOSDataObject* obj)
{
    QModelIndex parentIndex = createIndex(node->row(), 0, node);

    if (node->childCount() > 0)
    {
        beginRemoveRows(parentIndex, 0, node->childCount() - 1);
        node->clearChildren();
        endRemoveRows();
    }

    auto* provider = providerFor(obj->typeId());
    if (provider == nullptr) return;

    // 先构建再声明插入范围，保证 endInsertRows 时 rowCount 与声明一致
    provider->buildChildren(this, node, obj);
    const int newCount = node->childCount();
    if (newCount > 0)
    {
        beginInsertRows(parentIndex, 0, newCount - 1);
        endInsertRows();
    }
}

// ===== Project 信号处理 =====

void IDOSTreeModel::rebuildContainerSubtree(const QString& containerObjectId)
{
    if (m_project == nullptr) return;

    auto* containerNode = findNodeByObjectId(containerObjectId);
    if (containerNode == nullptr) return;   // 容器不在树中（如异域或未加载）

    auto* containerObj = m_project->objectById(containerObjectId);
    if (containerObj == nullptr) return;

    rebuildSubtree(containerNode, containerObj);
}

void IDOSTreeModel::onObjectAdded(const QString& objectId)
{
    if (m_project == nullptr) return;
    auto* obj = m_project->objectById(objectId);
    if (obj == nullptr) return;

    // 容器子对象：不进顶层，重建所属容器的子树即可
    if (!obj->containerId().isEmpty())
    {
        rebuildContainerSubtree(obj->containerId());
        return;
    }

    if (!shouldShow(obj)) return;

    // 1. 需要新分组时，先声明根上的组节点插入
    const QString group = groupLabelOf(obj);
    if (!group.isEmpty() && !m_groupNodes.contains(group))
    {
        const int groupRow = m_rootNode->childCount();
        beginInsertRows(QModelIndex(), groupRow, groupRow);
        auto* groupNode = new IDOSObjectTreeNode(m_rootNode);
        groupNode->setName(group);
        m_rootNode->appendChild(groupNode);
        m_groupNodes.insert(group, groupNode);
        endInsertRows();
    }

    // 2. 对象节点插入（Input 域可能挂分组节点下）
    IDOSObjectTreeNode* container = group.isEmpty()
        ? m_rootNode
        : m_groupNodes.value(group, m_rootNode);
    const int row = container->childCount();
    const QModelIndex parentIndex = (container == m_rootNode)
        ? QModelIndex()
        : createIndex(container->row(), 0, container);
    beginInsertRows(parentIndex, row, row);

    auto* node = new IDOSObjectTreeNode(container);
    node->setObjectId(objectId);
    node->setName(obj->name());
    container->appendChild(node);

    buildObjectSubtree(node, obj);

    endInsertRows();
}

void IDOSTreeModel::onObjectRemoved(const QString& objectId)
{
    auto* node = findNodeByObjectId(objectId);
    if (node == nullptr) return;

    // 嵌套节点（如网格属性挂在网格子树内）按其父节点声明删除范围
    IDOSTreeNode* parentNode = node->parent();
    if (parentNode == nullptr) return;

    const int row = node->row();
    const QModelIndex parentIndex = (parentNode == m_rootNode)
        ? QModelIndex()
        : createIndex(parentNode->row(), 0, parentNode);
    beginRemoveRows(parentIndex, row, row);
    parentNode->removeChild(row);
    endRemoveRows();

    // 被删对象可能正被其他对象的子树引用（如工况引用井）：重查全部对象子树
    refreshReferencingSubtrees();
}

void IDOSTreeModel::refreshReferencingSubtrees()
{
    if (m_project == nullptr) return;
    refreshNeedsRebuild(m_rootNode);
}

void IDOSTreeModel::refreshNeedsRebuild(IDOSTreeNode* branch)
{
    for (int i = 0; i < branch->childCount(); ++i)
    {
        IDOSTreeNode* child = branch->child(i);

        auto* objectNode = dynamic_cast<IDOSObjectTreeNode*>(child);
        if (objectNode != nullptr && !objectNode->objectId().isEmpty())
        {
            auto* obj = m_project->objectById(objectNode->objectId());
            if (obj != nullptr)
            {
                auto* provider = providerFor(obj->typeId());
                if (provider != nullptr
                    && provider->needsRebuild(this, objectNode, obj))
                {
                    rebuildSubtree(objectNode, obj);
                }
            }
        }

        refreshNeedsRebuild(child);
    }
}

void IDOSTreeModel::onObjectChanged(const QString& objectId)
{
    if (m_project == nullptr) return;

    auto* node = findNodeByObjectId(objectId);
    if (node == nullptr) return;

    auto* obj = m_project->objectById(objectId);
    if (obj != nullptr)
    {
        // 显示名经 provider 裁决（provider 可能特化，如网格属性显示 keyword），
        // 与 buildChildren 构建时的取名规则保持同源
        auto* provider = providerFor(obj->typeId());
        node->setName(provider != nullptr ? provider->displayName(obj) : obj->name());
    }

    QModelIndex index = createIndex(node->row(), 0, node);
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::CheckStateRole, Qt::ToolTipRole});

    // 结构变化才重建子树（避免内容编辑时 QTreeView 展开状态折叠）
    auto* provider = providerFor(obj != nullptr ? obj->typeId() : QString());
    if (provider != nullptr && obj != nullptr && provider->needsRebuild(this, node, obj))
    {
        rebuildSubtree(node, obj);
    }
}
