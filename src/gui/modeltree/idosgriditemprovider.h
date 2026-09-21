#ifndef IDOS_GRID_ITEM_PROVIDER_H
#define IDOS_GRID_ITEM_PROVIDER_H

#include "tree/idostreeitemprovider.h"
#include "idos_gui.h"

class IDOSProject;

/**
 * @brief 网格类型的树节点构建策略。
 *
 * 为 IDOSGrid 构建子树（仿 tNavigator 模型树布局）：
 *   几何（空分组占位） / 属性 → 静态属性、动态属性 → 各属性对象节点。
 * 属性对象（IDOSGridProperty）通过 containerId 引用本网格，从 Project
 * 查询归集；属性节点是 IDOSObjectTreeNode（带可见性勾选框）。
 */
class GUI_EXPORT IDOSGridItemProvider : public IDOSTreeItemProvider
{
public:
    QString typeId() const override;
    QString displayName(const IDOSDataObject* obj) const override;
    void buildChildren(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                       IDOSDataObject* obj) override;
    bool needsRebuild(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                      IDOSDataObject* obj) override;

private:
    /** 递归统计分支内属于指定网格的属性对象节点数。 */
    static void countPropertyNodes(class IDOSTreeNode* branch, const QString& gridId,
                                   IDOSProject* project, int& count);
};

#endif // IDOS_GRID_ITEM_PROVIDER_H
