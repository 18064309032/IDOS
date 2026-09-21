#ifndef IDOS_MODEL_TREE_MENU_PROVIDER_H
#define IDOS_MODEL_TREE_MENU_PROVIDER_H

#include "../tree/idostreemenuprovider.h"
#include "idos_gui.h"

class IDOSModelTreeModel;
class QTreeView;

/**
 * @brief Model Tree 右键菜单。
 *
 * 持有 Model 引用，通过 TreeView 的当前选中构造菜单。
 */
class GUI_EXPORT IDOSModelTreeMenuProvider : public IDOSTreeMenuProvider
{
    Q_OBJECT

public:
    explicit IDOSModelTreeMenuProvider(QObject* parent = nullptr);
    ~IDOSModelTreeMenuProvider() override;

    /** 设置关联的 Model。 */
    void setModel(IDOSModelTreeModel* model);

    /** 设置关联的 TreeView（用于获取当前选中项）。 */
    void setTreeView(QTreeView* view);

protected:
    QMenu* createContextMenu() override;

private:
    IDOSModelTreeModel* m_model;
    QTreeView* m_view;
};

#endif // IDOS_MODEL_TREE_MENU_PROVIDER_H
