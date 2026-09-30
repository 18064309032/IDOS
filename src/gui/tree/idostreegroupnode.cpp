#include "idostreegroupnode.h"

IDOSTreeGroupNode::IDOSTreeGroupNode(IDOSTreeNode* parent)
    : IDOSTreeNode(parent)
{
    setGroup(true);
}

IDOSTreeGroupNode::~IDOSTreeGroupNode()
{
}

QString IDOSTreeGroupNode::groupKey() const
{
    return m_groupKey;
}

void IDOSTreeGroupNode::setGroupKey(const QString& groupKey)
{
    m_groupKey = groupKey;
}

QString IDOSTreeGroupNode::nodeKey() const
{
    return QStringLiteral("g:") + m_groupKey;
}
