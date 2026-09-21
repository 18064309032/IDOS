#ifndef IDOS_MODEL_ITEM_PROVIDER_H
#define IDOS_MODEL_ITEM_PROVIDER_H

#include "tree/idostreeitemprovider.h"
#include "idos_gui.h"

class IDOSProject;
class IDOSModel;

/**
 * @brief 工况类型的树节点构建策略（仿 tNavigator 模型树顶层布局）。
 *
 * 为 IDOSModel 构建子树：网格（引用影子节点） / 井（引用影子节点列表）。
 * 引用成员是展示节点（objectId 为空，不带勾选框），数据本体仍在其
 * 所属位置（网格在模型树顶层，井在输入树）——避免同一 objectId 在
 * 同一棵树出现两个对象节点导致查找/删除歧义。
 * 构建/重建签名存于节点 tag，needsRebuild 据此感知引用增删与被引用对象改名。
 */
class GUI_EXPORT IDOSModelItemProvider : public IDOSTreeItemProvider
{
public:
    QString typeId() const override;
    void buildChildren(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                       IDOSDataObject* obj) override;
    bool needsRebuild(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                      IDOSDataObject* obj) override;

private:
    /** 生成成员签名："gridId|名称;wellId|名称;..."，改名/增删引用都会变化。 */
    static QString memberSignature(IDOSProject* project, const IDOSModel* modelObj);
};

#endif // IDOS_MODEL_ITEM_PROVIDER_H
