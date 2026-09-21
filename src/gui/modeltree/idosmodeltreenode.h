#ifndef IDOS_MODEL_TREE_NODE_H
#define IDOS_MODEL_TREE_NODE_H

#include "../tree/idostreenode.h"
#include "idos_core.h"

/**
 * @brief Model Tree 节点。
 *
 * 继承 IDOSTreeNode，通过 objectId 引用 Project 中的领域对象。
 * 不持有裸指针，符合跨模块 objectId 交互的架构约定。
 */
class GUI_EXPORT IDOSModelTreeNode : public IDOSTreeNode
{
public:
    explicit IDOSModelTreeNode(IDOSTreeNode* parent = nullptr);
    ~IDOSModelTreeNode() override;

    /** 对应的领域对象 objectId。空字符串表示分组节点。 */
    QString objectId() const;
    void setObjectId(const QString& objectId);

private:
    QString m_objectId;   // 空 = 分组节点
};

#endif // IDOS_MODEL_TREE_NODE_H
