#include "tree/idostreereferencenode.h"

IDOSTreeReferenceNode::IDOSTreeReferenceNode(IDOSTreeNode* parent)
    : IDOSTreeNode(parent)
{
}

IDOSTreeReferenceNode::~IDOSTreeReferenceNode() = default;

IDOSCaseItemRef IDOSTreeReferenceNode::itemRef() const
{
    return m_itemRef;
}

void IDOSTreeReferenceNode::setItemRef(const IDOSCaseItemRef& ref)
{
    m_itemRef = ref;
}
