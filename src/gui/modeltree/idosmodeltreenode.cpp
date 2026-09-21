#include "idosmodeltreenode.h"

IDOSModelTreeNode::IDOSModelTreeNode(IDOSTreeNode* parent)
    : IDOSTreeNode(parent)
{
}

IDOSModelTreeNode::~IDOSModelTreeNode() = default;

QString IDOSModelTreeNode::objectId() const { return m_objectId; }
void IDOSModelTreeNode::setObjectId(const QString& objectId) { m_objectId = objectId; }
