#include "idosobjecttreenode.h"

IDOSObjectTreeNode::IDOSObjectTreeNode(IDOSTreeNode* parent)
    : IDOSTreeNode(parent)
{
}

IDOSObjectTreeNode::~IDOSObjectTreeNode()
{
}

QString IDOSObjectTreeNode::objectId() const
{
    return m_objectId;
}
void IDOSObjectTreeNode::setObjectId(const QString& objectId)
{
    m_objectId = objectId;
}

QString IDOSObjectTreeNode::nodeKey() const
{
    return QStringLiteral("o:") + m_objectId;
}
