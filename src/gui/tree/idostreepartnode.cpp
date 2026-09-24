#include "tree/idostreepartnode.h"

IDOSTreePartNode::IDOSTreePartNode(IDOSTreeNode* parent)
    : IDOSTreeNode(parent)
{
}

IDOSTreePartNode::~IDOSTreePartNode() = default;

QString IDOSTreePartNode::ownerObjectId() const
{
    return m_ownerObjectId;
}

void IDOSTreePartNode::setOwnerObjectId(const QString& ownerObjectId)
{
    m_ownerObjectId = ownerObjectId;
}

IDOSTreePartKey IDOSTreePartNode::partKey() const
{
    return m_partKey;
}

void IDOSTreePartNode::setPartKey(const IDOSTreePartKey& partKey)
{
    m_partKey = partKey;
}

QString IDOSTreePartNode::itemKey() const
{
    return m_itemKey;
}

void IDOSTreePartNode::setItemKey(const QString& itemKey)
{
    m_itemKey = itemKey;
}
