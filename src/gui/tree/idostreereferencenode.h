#ifndef IDOS_TREE_REFERENCE_NODE_H
#define IDOS_TREE_REFERENCE_NODE_H

#include "tree/idostreenode.h"
#include "case/idoscaseitemref.h"

/**
 * @brief 工况树中的引用节点。
 *
 * 引用节点不拥有业务数据，只保存 IDOSCaseItemRef，用于指向数据树中的对象
 * 或对象内部子项。
 */
class GUI_EXPORT IDOSTreeReferenceNode : public IDOSTreeNode
{
public:
    explicit IDOSTreeReferenceNode(IDOSTreeNode* parent = nullptr);
    ~IDOSTreeReferenceNode() override;

    IDOSCaseItemRef itemRef() const;
    void setItemRef(const IDOSCaseItemRef& ref);

private:
    IDOSCaseItemRef m_itemRef;
};

#endif // IDOS_TREE_REFERENCE_NODE_H
