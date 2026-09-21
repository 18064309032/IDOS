#include "idosobjecttreenode.h"

IDOSObjectTreeNode::IDOSObjectTreeNode(IDOSTreeNode* parent)
    : IDOSTreeNode(parent)
{
}

IDOSObjectTreeNode::~IDOSObjectTreeNode() = default;

QString IDOSObjectTreeNode::objectId() const { return m_objectId; }
void IDOSObjectTreeNode::setObjectId(const QString& objectId) { m_objectId = objectId; }
