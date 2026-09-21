#ifndef IDOS_INPUT_TREE_VIEW_H
#define IDOS_INPUT_TREE_VIEW_H

#include "tree/idostreeview.h"
#include "idos_gui.h"

/**
 * @brief 输入树视图（与 IDOSInputTreeModel 成对）。
 *
 * 显示/数据差异由 model 与 item provider 承担；本类只承载输入树专属的
 * 交互行为，当前与基类行为一致。预留扩展点：
 * - 拖拽"加入工况"（dragEnterEvent/dropEvent）
 * - 导入相关快捷交互
 */
class GUI_EXPORT IDOSInputTreeView : public IDOSTreeView
{
    Q_OBJECT

public:
    explicit IDOSInputTreeView(QWidget* parent = nullptr);
    ~IDOSInputTreeView() override;
};

#endif // IDOS_INPUT_TREE_VIEW_H
