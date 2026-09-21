#ifndef IDOS_TREE_MODEL_MENU_PROVIDER_H
#define IDOS_TREE_MODEL_MENU_PROVIDER_H

#include "idostreemenuprovider.h"
#include "idos_gui.h"

class IDOSTreeModel;
class QTreeView;

/**
 * @brief 数据对象树通用右键菜单。
 *
 * 持有 IDOSTreeModel 引用，通过 TreeView 的当前选中构造菜单：
 * 可见性切换 + provider 追加的类型特定动作。
 * 对输入树/模型树通用（面向基类 IDOSTreeModel）。
 */
class GUI_EXPORT IDOSTreeModelMenuProvider : public IDOSTreeMenuProvider
{
    Q_OBJECT

public:
    explicit IDOSTreeModelMenuProvider(QObject* parent = nullptr);
    ~IDOSTreeModelMenuProvider() override;

    /** 设置关联的模型。 */
    void setModel(IDOSTreeModel* model);

    /** 设置关联的 TreeView（用于获取当前选中项）。 */
    void setTreeView(QTreeView* view);

protected:
    QMenu* createContextMenu() override;

private:
    IDOSTreeModel* m_model;
    QTreeView* m_view;
};

#endif // IDOS_TREE_MODEL_MENU_PROVIDER_H
