#include "idosproject.h"
#include "idostreeitem.h"

#include "idostree.h"

IDOSTree::IDOSTree(QObject* parent)
    :
    QObject(parent)
    , m_root(new IDOSTreeItem())
    , m_changing(false)
{
    m_root->attach(this);
}

IDOSTree::~IDOSTree()
{
    delete m_root;
}

IDOSTreeItem* IDOSTree::root() const
{
    return m_root;
}

IDOSProject* IDOSTree::project() const
{
    return m_project.data();
}

void IDOSTree::setProject(IDOSProject* project)
{
    m_project = project;
    emit projectChanged();
}

QString IDOSTree::name() const
{
    return m_name;
}

void IDOSTree::setName(const QString& name)
{
    m_name = name;
}

bool IDOSTree::insert(IDOSTreeItem* parent, IDOSTreeItem* item, int row)
{
    if (m_changing || parent == nullptr || parent->tree() != this ||
        item == nullptr || item->m_parent != nullptr || item->m_tree != nullptr)
    {
        return false;
    }
    for (IDOSTreeItem* ancestor = parent; ancestor != nullptr; ancestor = ancestor->parent())
    {
        if (ancestor == item)
        {
            return false;
        }
    }
    if (row < 0 || row > parent->childCount())
    {
        row = parent->childCount();
    }
    m_changing = true;
    emit aboutToInsert(parent, row);
    item->m_parent = parent;
    parent->m_children.insert(row, item);
    item->attach(this);
    emit inserted();
    m_changing = false;
    return true;
}

bool IDOSTree::remove(IDOSTreeItem* item)
{
    if (m_changing || item == nullptr || item == m_root || item->tree() != this)
    {
        return false;
    }
    IDOSTreeItem* parent = item->parent();
    const int row = item->row();
    m_changing = true;
    emit aboutToRemove(parent, row);
    parent->m_children.removeAt(row);
    delete item;
    emit removed();
    m_changing = false;
    return true;
}

void IDOSTree::clear()
{
    while (!m_changing && m_root->childCount() > 0)
    {
        remove(m_root->child(m_root->childCount() - 1));
    }
}

IDOSTreeItem* IDOSTree::lookup(IDOSTreeItem* branch, const QString& value, bool byId) const
{
    if ((byId ? branch->id() : branch->key()) == value)
    {
        return branch;
    }
    for (IDOSTreeItem* child : branch->children())
    {
        IDOSTreeItem* found = lookup(child, value, byId);
        if (found != nullptr)
        {
            return found;
        }
    }
    return nullptr;
}

IDOSTreeItem* IDOSTree::item(const QString& id) const
{
    return id.isEmpty() ? nullptr : lookup(m_root, id, true);
}

IDOSTreeItem* IDOSTree::find(const QString& key) const
{
    return key.isEmpty() ? nullptr : lookup(m_root, key, false);
}

void IDOSTree::changed(IDOSTreeItem* item)
{
    if (item != nullptr && item->tree() == this)
    {
        emit itemChanged(item);
    }
}
