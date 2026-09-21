#include "idoswellitemprovider.h"
#include "tree/idostreemodel.h"
#include "tree/idosvaluetreenode.h"
#include "data/idosdataobject.h"
#include "data/well/idoswell.h"
#include <QObject>

using Role = IDOSValueTreeNode::ValueRole;

QString IDOSWellItemProvider::typeId() const
{
    return QStringLiteral("idos.well");
}

void IDOSWellItemProvider::buildChildren(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                                         IDOSDataObject* obj)
{
    Q_UNUSED(model);
    auto* well = qobject_cast<IDOSWell*>(obj);
    if (well == nullptr || parentNode == nullptr) return;

    const QString owner = well->objectId();

    // 井头（单值）
    auto* headNode = new IDOSValueTreeNode(nullptr, owner, Role::WellHead, 0);
    headNode->setName(QObject::tr("Wellhead"));
    parentNode->appendChild(headNode);

    // 井控（单值）
    auto* controlNode = new IDOSValueTreeNode(nullptr, owner, Role::WellControl, 0);
    controlNode->setName(QObject::tr("Well Control"));
    parentNode->appendChild(controlNode);

    // 井轨迹分组
    auto* trajGroup = new IDOSValueTreeNode(nullptr, owner, Role::SegmentGroup, 0);
    trajGroup->setName(QObject::tr("Trajectory"));
    trajGroup->setGroup(true);
    parentNode->appendChild(trajGroup);

    const QList<IDOSWellSegment> segments = well->segments();
    for (int i = 0; i < segments.size(); ++i)
    {
        auto* segNode = new IDOSValueTreeNode(nullptr, owner, Role::Segment, i);
        segNode->setName(QObject::tr("Segment %1").arg(i + 1));
        trajGroup->appendChild(segNode);
    }

    // 射孔分组
    auto* perfGroup = new IDOSValueTreeNode(nullptr, owner, Role::CompletionGroup, 0);
    perfGroup->setName(QObject::tr("Perforations"));
    perfGroup->setGroup(true);
    parentNode->appendChild(perfGroup);

    const QList<IDOSWellCompletion> completions = well->completions();
    for (int i = 0; i < completions.size(); ++i)
    {
        auto* compNode = new IDOSValueTreeNode(nullptr, owner, Role::Completion, i);
        compNode->setName(QObject::tr("Completion %1").arg(i + 1));
        perfGroup->appendChild(compNode);
    }
}

bool IDOSWellItemProvider::needsRebuild(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                                        IDOSDataObject* obj)
{
    Q_UNUSED(model);
    auto* well = qobject_cast<IDOSWell*>(obj);
    if (well == nullptr || parentNode == nullptr) return false;

    // 统计树中现有轨迹段/射孔段数量，与井数据比对
    int treeSegments = -1;
    int treeCompletions = -1;
    for (int i = 0; i < parentNode->childCount(); ++i)
    {
        auto* group = dynamic_cast<IDOSValueTreeNode*>(parentNode->child(i));
        if (group == nullptr) continue;
        if (group->role() == Role::SegmentGroup)
        {
            treeSegments = group->childCount();
        }
        else if (group->role() == Role::CompletionGroup)
        {
            treeCompletions = group->childCount();
        }
    }

    // 分组节点缺失视为结构损坏，重建
    if (treeSegments < 0 || treeCompletions < 0) return true;

    return treeSegments != well->segments().size()
        || treeCompletions != well->completions().size();
}
