#ifndef IDOS_WELL_ITEM_PROVIDER_H
#define IDOS_WELL_ITEM_PROVIDER_H

#include "tree/idostreeitemprovider.h"
#include "idos_gui.h"

/**
 * @brief 井类型的树节点构建策略。
 *
 * 为 IDOSWell 构建虚拟子树：井头、井控、轨迹分组（含各轨迹段）、
 * 射孔分组（含各射孔段）。列表项用位置命名（"Segment 1"），
 * 内容编辑不改变树显示，仅数量变化触发子树重建。
 */
class GUI_EXPORT IDOSWellItemProvider : public IDOSTreeItemProvider
{
public:
    QString typeId() const override;
    void buildChildren(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                       IDOSDataObject* obj) override;
    bool needsRebuild(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                      IDOSDataObject* obj) override;
};

#endif // IDOS_WELL_ITEM_PROVIDER_H
