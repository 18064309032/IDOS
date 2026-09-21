#include "idostreenode.h"

IDOSTreeNode::IDOSTreeNode(IDOSTreeNode* parent)
    : m_parent(parent)
    , m_isGroup(false)
{
}

IDOSTreeNode::~IDOSTreeNode()
{
    qDeleteAll(m_children);
}

IDOSTreeNode* IDOSTreeNode::parent() const { return m_parent; }
int IDOSTreeNode::childCount() const { return m_children.size(); }
IDOSTreeNode* IDOSTreeNode::child(int row) const
{
    if (row >= 0 && row < m_children.size()) return m_children.at(row);
    return nullptr;
}

int IDOSTreeNode::row() const
{
    if (m_parent == nullptr) return 0;
    return m_parent->m_children.indexOf(const_cast<IDOSTreeNode*>(this));
}

bool IDOSTreeNode::isGroup() const { return m_isGroup; }
void IDOSTreeNode::setGroup(bool group) { m_isGroup = group; }

QString IDOSTreeNode::name() const { return m_name; }
void IDOSTreeNode::setName(const QString& name) { m_name = name; }

QIcon IDOSTreeNode::icon() const { return m_icon; }
void IDOSTreeNode::setIcon(const QIcon& icon) { m_icon = icon; }

QString IDOSTreeNode::tag() const { return m_tag; }
void IDOSTreeNode::setTag(const QString& tag) { m_tag = tag; }

void IDOSTreeNode::appendChild(IDOSTreeNode* child)
{
    if (child == nullptr) return;
    child->m_parent = this;
    m_children.append(child);
}

void IDOSTreeNode::removeChild(int row)
{
    if (row < 0 || row >= m_children.size()) return;
    auto* child = m_children.takeAt(row);
    delete child;
}

void IDOSTreeNode::clearChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}
