#include "idosvaluetreenode.h"

IDOSValueTreeNode::IDOSValueTreeNode(IDOSTreeNode* parent, const QString& ownerObjectId,
                                     ValueRole role, int index)
    : IDOSTreeNode(parent)
    , m_ownerObjectId(ownerObjectId)
    , m_role(role)
    , m_index(index)
{
}

IDOSValueTreeNode::~IDOSValueTreeNode() = default;

QString IDOSValueTreeNode::ownerObjectId() const { return m_ownerObjectId; }

IDOSValueTreeNode::ValueRole IDOSValueTreeNode::role() const { return m_role; }

int IDOSValueTreeNode::index() const { return m_index; }
