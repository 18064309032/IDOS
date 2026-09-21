#ifndef IDOS_MODEL_TREE_VIEW_H
#define IDOS_MODEL_TREE_VIEW_H

#include "tree/idostreeview.h"
#include "idos_gui.h"

/**
 * @brief 模型树视图（与 IDOSModelTreeModel 成对）。
 *
 * 显示/数据差异由 model 与 item provider 承担；本类只承载模型树专属的
 * 交互行为，当前与基类行为一致。预留扩展点：
 * - 结果节点双击切换 3D 显示模式
 * - 工况运行/导出入口
 */
class GUI_EXPORT IDOSModelTreeView : public IDOSTreeView
{
    Q_OBJECT

public:
    explicit IDOSModelTreeView(QWidget* parent = nullptr);
    ~IDOSModelTreeView() override;
};

#endif // IDOS_MODEL_TREE_VIEW_H
