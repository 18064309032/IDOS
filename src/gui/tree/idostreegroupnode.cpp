#include "tree/idostreegroupnode.h"

IDOSTreeGroupNode::IDOSTreeGroupNode(IDOSTreeNode* parent)
    : IDOSTreeNode(parent)
{
    setGroup(true);
}

IDOSTreeGroupNode::~IDOSTreeGroupNode() = default;

QString IDOSTreeGroupNode::groupKey() const
{
    return m_groupKey;
}

void IDOSTreeGroupNode::setGroupKey(const QString& groupKey)
{
    m_groupKey = groupKey;
}
