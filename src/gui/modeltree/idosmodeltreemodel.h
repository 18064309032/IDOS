#ifndef IDOS_MODEL_TREE_MODEL_H
#define IDOS_MODEL_TREE_MODEL_H

#include "tree/idostreemodel.h"
#include "idos_gui.h"

/**
 * @brief 模型树：展示 Project 中的模型对象（TreeScope::Model）。
 *
 * 网格等模型对象按类型元数据的 IDOSObjectCategory::Model 分流至此。
 * 目前注册网格的节点构建 provider，后续模型类型（工况 IDOSModel、
 * 网格属性 IDOSGridProperty、结果等）注册各自的 provider 即自动上树。
 *
 * 模型树专属行为（运行/导出工况、结果展示、引用成员节点等）在此子类扩展。
 */
class GUI_EXPORT IDOSModelTreeModel : public IDOSTreeModel
{
    Q_OBJECT

public:
    explicit IDOSModelTreeModel(QObject* parent = nullptr);
    ~IDOSModelTreeModel() override;
};

#endif // IDOS_MODEL_TREE_MODEL_H
