#ifndef IDOS_OBJECT_TREE_NODE_H
#define IDOS_OBJECT_TREE_NODE_H

#include "idostreenode.h"
#include "idos_core.h"

/**
 * @brief 绑定领域对象的树节点。
 *
 * 继承 IDOSTreeNode，通过 objectId 引用 Project 中的领域对象。
 * objectId 为空时表示纯分组节点（无勾选框）。
 * 不持有裸指针，符合跨模块 objectId 交互的架构约定。
 */
class GUI_EXPORT IDOSObjectTreeNode : public IDOSTreeNode
{
public:
    explicit IDOSObjectTreeNode(IDOSTreeNode* parent = nullptr);
    ~IDOSObjectTreeNode() override;

    /** 对应的领域对象 objectId。空字符串表示分组节点。 */
    QString objectId() const;
    void setObjectId(const QString& objectId);

private:
    QString m_objectId;   // 空 = 分组节点
};

#endif // IDOS_OBJECT_TREE_NODE_H
