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
                emit itemCheckedChanged(childReferenceNode->itemRef().objectId(), false);
            }
        }
    }

    return IDOSTreeModel::setData(index, value, role);
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

