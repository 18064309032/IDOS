#include "idoscasetreemodel.h"
#include "idoscaseitemref.h"
#include "idoscaseobject.h"
#include "idosgridproperty.h"
#include "idosobjecttreenode.h"
#include "idosproject.h"
#include "idostreebuilder.h"
#include "idoscasetreeprovider.h"
#include "idostreeproviderregistry.h"
#include "idostreereferencenode.h"

IDOSCaseTreeModel::IDOSCaseTreeModel(QObject* parent)
    : IDOSTreeModel(parent)
    , m_treeProviderRegistry(nullptr)
{
    connect(this, &IDOSTreeModel::checkStateChanged, this, &IDOSCaseTreeModel::onCheckStateChanged);
}

IDOSCaseTreeModel::~IDOSCaseTreeModel()
{
}

void IDOSCaseTreeModel::setTreeProviderRegistry(IDOSTreeProviderRegistry* registry)
{
    m_treeProviderRegistry = registry;
}

bool IDOSCaseTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole && value.toInt() == Qt::Checked)
    {
        IDOSTreeNode* node = nodeFromIndex(index);
        IDOSTreeReferenceNode* referenceNode = dynamic_cast<IDOSTreeReferenceNode*>(node);
        IDOSTreeNode* parentNode = node != nullptr ? node->parent() : nullptr;
        if (referenceNode != nullptr && parentNode != nullptr)
        {
            const QModelIndex parentIndex = parent(index);
            for (int row = 0; row < parentNode->childCount(); ++row)
            {
                IDOSTreeNode* childNode = parentNode->child(row);
                if (childNode == nullptr || childNode == node || !childNode->isCheckable() || !childNode->isChecked())
                {
                    continue;
                }
                IDOSTreeReferenceNode* childReferenceNode = dynamic_cast<IDOSTreeReferenceNode*>(childNode);
                if (childReferenceNode == nullptr)
                {
                    continue;
                }
                childNode->setChecked(false);
                const QModelIndex childIndex = IDOSTreeModel::index(row, 0, parentIndex);
                emit dataChanged(childIndex, childIndex, QVector<int>() << Qt::CheckStateRole);

                // 网格属性互斥只更新 UI，不发 hide 信号：属性共用同一 grid renderObject，
                // hide 会让整个网格消失一帧再被 showGridProperty 重建 = 闪烁；
                // 新属性颜色由 showGridProperty 直接覆盖到 renderObject，无需先 hide 旧属性。
                const IDOSDataObject* childObject =
                    project() != nullptr ? project()->objectById(childReferenceNode->itemRef().objectId()) : nullptr;
                if (qobject_cast<const IDOSGridProperty*>(childObject) != nullptr)
                {
                    continue;
                }
                emit itemCheckedChanged(childReferenceNode->itemRef().objectId(), false);
            }
        }
    }

    const bool ok = IDOSTreeModel::setData(index, value, role);
    if (!ok)
    {
        return false;
    }

    // 父子联动（属性依附网格几何）：属性渲染以网格 renderObject 为载体，
    // 勾选属性须保证网格已显示；取消网格则依附其上的属性一并消失。
    if (index.isValid() && role == Qt::CheckStateRole)
    {
        const bool willCheck = value.toInt() == Qt::Checked;
        IDOSTreeNode* node = nodeFromIndex(index);
        IDOSTreeReferenceNode* referenceNode = dynamic_cast<IDOSTreeReferenceNode*>(node);
        if (referenceNode != nullptr)
        {
            const QString refRole = referenceNode->itemRef().role();
            if (willCheck && refRole == QStringLiteral("case.gridProperty"))
            {
                autoCheckParentGrid(node);
            }
            else if (!willCheck && refRole == QStringLiteral("case.grid"))
            {
                uncheckChildProperties(node);
            }
        }
    }
    return true;
}

bool IDOSCaseTreeModel::shouldShowObject(const IDOSDataObject* object) const
{
    return qobject_cast<const IDOSCaseObject*>(object) != nullptr;
}

void IDOSCaseTreeModel::buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object)
{
    const IDOSCaseObject* caseObject = qobject_cast<const IDOSCaseObject*>(object);
    if (parentNode == nullptr || caseObject == nullptr || m_treeProviderRegistry == nullptr)
    {
        return;
    }

    IDOSTreeBuilder builder(parentNode);
    IDOSCaseTreeProvider* provider = m_treeProviderRegistry->caseProviderFor(caseObject->caseTypeId());
    if (provider != nullptr)
    {
        provider->buildCaseTree(builder, caseObject);
    }
}

void IDOSCaseTreeModel::refreshReferencingBranches(const QString& objectId, bool objectRemoved)
{
    // 单对象路径直接委托批量实现（单元素列表），算法只保留一份
    const QStringList changedIds = objectRemoved ? QStringList() : QStringList(objectId);
    const QStringList removedIds = objectRemoved ? QStringList(objectId) : QStringList();
    refreshReferencingBranchesBatch(changedIds, removedIds);
}

void IDOSCaseTreeModel::refreshReferencingBranchesBatch(const QStringList& changedIds,
                                                        const QStringList& removedIds)
{
    if (project() == nullptr || m_treeProviderRegistry == nullptr)
    {
        return;
    }

    const QSet<QString> changedSet(changedIds.begin(), changedIds.end());

    // 预计算一批变化对象涉及的网格：网格属性按所属网格影响工况
    QSet<QString> affectedGridIds;
    for (const QString& id : changedIds)
    {
        const IDOSGridProperty* property =
            qobject_cast<const IDOSGridProperty*>(project()->objectById(id));
        if (property != nullptr && !property->gridId().isEmpty())
        {
            affectedGridIds.insert(property->gridId());
        }
    }

    const bool anyRemoved = !removedIds.isEmpty();

    // 每个工况最多重建一次：一批属性只让引用其网格的工况重建一轮，不再 O(N²)
    for (IDOSDataObject* candidate : project()->objects())
    {
        const IDOSCaseObject* caseObject = qobject_cast<const IDOSCaseObject*>(candidate);
        if (caseObject == nullptr)
        {
            continue;
        }
        // 工况自身的分支已由基类 processChangedObject 重建，这里不重复处理
        if (changedSet.contains(caseObject->objectId()))
        {
            continue;
        }

        bool affected = anyRemoved;   // 删除无法再判定归属，保守地全部重建（失效引用显示 Missing object）
        if (!affected)
        {
            for (const IDOSCaseItemRef& ref : caseObject->itemRefs())
            {
                if (changedSet.contains(ref.objectId())
                    || (ref.role() == QStringLiteral("case.grid")
                        && affectedGridIds.contains(ref.objectId())))
                {
                    affected = true;
                    break;
                }
            }
        }
        if (!affected)
        {
            continue;
        }

        IDOSObjectTreeNode* caseNode = findObjectNode(caseObject->objectId());
        if (caseNode != nullptr)
        {
            rebuildObjectBranch(caseNode, caseObject);
        }
    }
}

void IDOSCaseTreeModel::onCheckStateChanged(const QModelIndex& index, bool checked)
{
    IDOSTreeReferenceNode* referenceNode = dynamic_cast<IDOSTreeReferenceNode*>(nodeFromIndex(index));
    if (referenceNode != nullptr)
    {
        emit itemCheckedChanged(referenceNode->itemRef().objectId(), checked);
    }
}

void IDOSCaseTreeModel::autoCheckParentGrid(IDOSTreeNode* propertyNode)
{
    if (propertyNode == nullptr)
    {
        return;
    }
    // 向上遍历父节点链，找首个 role=="case.grid" 的引用节点即所属网格
    for (IDOSTreeNode* parent = propertyNode->parent(); parent != nullptr; parent = parent->parent())
    {
        IDOSTreeReferenceNode* gridRef = dynamic_cast<IDOSTreeReferenceNode*>(parent);
        if (gridRef == nullptr || gridRef->itemRef().role() != QStringLiteral("case.grid"))
        {
            continue;
        }
        if (!gridRef->isChecked() && gridRef->isCheckable())
        {
            gridRef->setChecked(true);
            const QModelIndex idx = indexOfNode(gridRef);
            if (idx.isValid())
            {
                emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
            }
            // 通知渲染服务器显示网格几何（showGridProperty 已隐式创建 renderObject，
            // 此处补发网格 shown 使树 UI 与渲染状态一致）
            emit itemCheckedChanged(gridRef->itemRef().objectId(), true);
        }
        break;
    }
}

void IDOSCaseTreeModel::uncheckChildProperties(IDOSTreeNode* gridNode)
{
    if (gridNode == nullptr)
    {
        return;
    }
    QList<IDOSTreeNode*> toUncheck;
    collectCheckedGridProperties(gridNode, toUncheck);
    for (IDOSTreeNode* node : toUncheck)
    {
        node->setChecked(false);
        const QModelIndex idx = indexOfNode(node);
        if (idx.isValid())
        {
            emit dataChanged(idx, idx, QVector<int>() << Qt::CheckStateRole);
        }
        IDOSTreeReferenceNode* ref = dynamic_cast<IDOSTreeReferenceNode*>(node);
        if (ref != nullptr)
        {
            emit itemCheckedChanged(ref->itemRef().objectId(), false);
        }
    }
}

void IDOSCaseTreeModel::collectCheckedGridProperties(IDOSTreeNode* branch, QList<IDOSTreeNode*>& out) const
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
        IDOSTreeReferenceNode* ref = dynamic_cast<IDOSTreeReferenceNode*>(child);
        if (ref != nullptr && child->isCheckable() && child->isChecked() &&
            ref->itemRef().role() == QStringLiteral("case.gridProperty"))
        {
            out.append(child);
        }
        collectCheckedGridProperties(child, out);
    }
}

