#ifndef IDOS_INPUT_TREE_MODEL_H
#define IDOS_INPUT_TREE_MODEL_H

#include "tree/idostreemodel.h"
#include "idos_gui.h"

/**
 * @brief 输入树：展示 Project 中的输入数据（TreeScope::Input）。
 *
 * 井、地震、流体等输入数据按类型元数据的 inputGroup 归组显示
 * （如"井组"）。目前注册井的节点构建 provider，后续输入类型
 * （地震、断层、流体等）注册各自的 provider 即自动上树。
 *
 * 输入树专属行为（导入菜单、拖拽"加入工况"等）在此子类扩展。
 */
class GUI_EXPORT IDOSInputTreeModel : public IDOSTreeModel
{
    Q_OBJECT

public:
    explicit IDOSInputTreeModel(QObject* parent = nullptr);
    ~IDOSInputTreeModel() override;
};

#endif // IDOS_INPUT_TREE_MODEL_H
