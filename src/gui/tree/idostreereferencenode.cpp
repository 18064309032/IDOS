#include "idostreereferencenode.h"

IDOSTreeReferenceNode::IDOSTreeReferenceNode(IDOSTreeNode* parent)
    : IDOSTreeNode(parent)
{
}

IDOSTreeReferenceNode::~IDOSTreeReferenceNode()
{
}

IDOSCaseItemRef IDOSTreeReferenceNode::itemRef() const
{
    return m_itemRef;
}

void IDOSTreeReferenceNode::setItemRef(const IDOSCaseItemRef& ref)
{
    m_itemRef = ref;
}

QString IDOSTreeReferenceNode::nodeKey() const
{
    return QStringLiteral("r:") + m_itemRef.role() + QStringLiteral(":") + m_itemRef.objectId();
}
