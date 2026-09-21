#include "idosgriditemprovider.h"
#include "tree/idostreemodel.h"
#include "tree/idosobjecttreenode.h"
#include "tree/idostreenode.h"
#include "data/idosdataobject.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridproperty.h"
#include "idosproject.h"
#include <QObject>

QString IDOSGridItemProvider::typeId() const
{
    return QStringLiteral("idos.grid");
}

QString IDOSGridItemProvider::displayName(const IDOSDataObject* obj) const
{
    // 网格属性优先显示 Eclipse 关键字（如 PERMX），无关键字退回对象名
    auto* property = qobject_cast<const IDOSGridProperty*>(obj);
    if (property != nullptr && !property->keyword().isEmpty())
    {
        return property->keyword();
    }
    return obj != nullptr ? obj->name() : QString();
}

void IDOSGridItemProvider::buildChildren(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                                         IDOSDataObject* obj)
{
    auto* grid = qobject_cast<IDOSGrid*>(obj);
    if (grid == nullptr || model == nullptr || parentNode == nullptr) return;

    IDOSProject* project = model->project();
    if (project == nullptr) return;

    // 几何分组（占位：角点数据可视化条目后续接入）
    auto* geometryGroup = new IDOSObjectTreeNode(parentNode);
    geometryGroup->setName(QObject::tr("Geometry"));
    parentNode->appendChild(geometryGroup);

    // 属性分组 → 静态/动态
    auto* propertyGroup = new IDOSObjectTreeNode(parentNode);
    propertyGroup->setName(QObject::tr("Properties"));
    parentNode->appendChild(propertyGroup);

    auto* staticGroup = new IDOSObjectTreeNode(propertyGroup);
    staticGroup->setName(QObject::tr("Static Properties"));
    propertyGroup->appendChild(staticGroup);

    auto* dynamicGroup = new IDOSObjectTreeNode(propertyGroup);
    dynamicGroup->setName(QObject::tr("Dynamic Properties"));
    propertyGroup->appendChild(dynamicGroup);

    // 归集引用本网格的属性对象（对象节点带 objectId → 勾选框/右键菜单可用）
    const QString gridId = grid->objectId();
    for (auto* candidate : project->objects())
    {
        auto* property = qobject_cast<IDOSGridProperty*>(candidate);
        if (property == nullptr || property->gridId() != gridId) continue;

        auto* container = property->kind() == IDOSGridProperty::Kind::Static
            ? staticGroup
            : dynamicGroup;

        auto* propNode = new IDOSObjectTreeNode(container);
        propNode->setObjectId(property->objectId());
        propNode->setName(displayName(property));
        container->appendChild(propNode);
    }
}

bool IDOSGridItemProvider::needsRebuild(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                                        IDOSDataObject* obj)
{
    if (model == nullptr || parentNode == nullptr) return false;
    IDOSProject* project = model->project();
    if (project == nullptr) return false;

    // 工程 vs 树：属性数量比对
    const QString gridId = obj != nullptr ? obj->objectId() : QString();
    int projectCount = 0;
    for (auto* candidate : project->objects())
    {
        auto* property = qobject_cast<IDOSGridProperty*>(candidate);
        if (property != nullptr && property->gridId() == gridId) ++projectCount;
    }

    int treeCount = 0;
    countPropertyNodes(parentNode, gridId, project, treeCount);
    return treeCount != projectCount;
}

void IDOSGridItemProvider::countPropertyNodes(IDOSTreeNode* branch, const QString& gridId,
                                              IDOSProject* project, int& count)
{
    for (int i = 0; i < branch->childCount(); ++i)
    {
        IDOSTreeNode* child = branch->child(i);

        auto* objectNode = dynamic_cast<IDOSObjectTreeNode*>(child);
        if (objectNode != nullptr && !objectNode->objectId().isEmpty())
        {
            auto* candidate = qobject_cast<IDOSGridProperty*>(
                project->objectById(objectNode->objectId()));
            if (candidate != nullptr && candidate->gridId() == gridId) ++count;
        }

        countPropertyNodes(child, gridId, project, count);
    }
}
