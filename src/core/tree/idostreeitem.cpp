#include <QUuid>

#include "idostree.h"
#include "idostreeprovider.h"

#include "idostreeitem.h"

IDOSTreeItem::IDOSTreeItem()
    :
    m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_group(false)
    , m_reference(false)
    , m_checkable(false)
    , m_checked(false)
    , m_parent(nullptr)
    , m_tree(nullptr)
    , m_provider(nullptr)
{
}

IDOSTreeItem::~IDOSTreeItem()
{
    qDeleteAll(m_children);
    delete m_provider;
}

QString IDOSTreeItem::id() const
{
    return m_id;
}

QString IDOSTreeItem::key() const
{
    return m_key;
}

void IDOSTreeItem::setKey(const QString& value)
{
    if (m_key == value)
    {
        return;
    }
    m_key = value;
    changed();
}

QString IDOSTreeItem::name() const
{
    return m_name;
}

void IDOSTreeItem::setName(const QString& value)
{
    if (m_name == value)
    {
        return;
    }
    m_name = value;
    changed();
}

QIcon IDOSTreeItem::icon() const
{
    return m_icon;
}

void IDOSTreeItem::setIcon(const QIcon& value)
{
    if (m_icon.cacheKey() == value.cacheKey())
    {
        return;
    }
    m_icon = value;
    changed();
}

QString IDOSTreeItem::dataId() const
{
    return m_dataId;
}

void IDOSTreeItem::setDataId(const QString& value)
{
    if (m_dataId == value)
    {
        return;
    }
    m_dataId = value;
    changed();
}

QString IDOSTreeItem::caseId() const
{
    return m_caseId;
}

void IDOSTreeItem::setCaseId(const QString& value)
{
    if (m_caseId == value)
    {
        return;
    }
    m_caseId = value;
    changed();
}

QString IDOSTreeItem::partKey() const
{
    return m_partKey;
}

void IDOSTreeItem::setPartKey(const QString& value)
{
    if (m_partKey == value)
    {
        return;
    }
    m_partKey = value;
    changed();
}

QString IDOSTreeItem::itemKey() const
{
    return m_itemKey;
}

void IDOSTreeItem::setItemKey(const QString& value)
{
    if (m_itemKey == value)
    {
        return;
    }
    m_itemKey = value;
    changed();
}

bool IDOSTreeItem::isGroup() const
{
    return m_group;
}

void IDOSTreeItem::setGroup(bool value)
{
    if (m_group == value)
    {
        return;
    }
    m_group = value;
    changed();
}

bool IDOSTreeItem::isReference() const
{
    return m_reference;
}

void IDOSTreeItem::setReference(bool value)
{
    if (m_reference == value)
    {
        return;
    }
    m_reference = value;
    changed();
}

bool IDOSTreeItem::isCheckable() const
{
    return m_checkable;
}

void IDOSTreeItem::setCheckable(bool value)
{
    if (m_checkable == value)
    {
        return;
    }
    m_checkable = value;
    changed();
}

bool IDOSTreeItem::isChecked() const
{
    return m_checked;
}

void IDOSTreeItem::setChecked(bool value)
{
    if (m_checked == value)
    {
        return;
    }
    m_checked = value;
    changed();
}

IDOSTreeItem* IDOSTreeItem::parent() const
{
    return m_parent;
}

QVector<IDOSTreeItem*> IDOSTreeItem::children() const
{
    return m_children;
}

IDOSTreeItem* IDOSTreeItem::child(int row) const
{
    return row >= 0 && row < m_children.size() ? m_children.at(row) : nullptr;
}

int IDOSTreeItem::childCount() const
{
    return m_children.size();
}

int IDOSTreeItem::row() const
{
    return m_parent == nullptr ? 0 : m_parent->m_children.indexOf(const_cast<IDOSTreeItem*>(this));
}

IDOSTree* IDOSTreeItem::tree() const
{
    return m_tree;
}

IDOSTreeProvider* IDOSTreeItem::provider() const
{
    return m_provider != nullptr ? m_provider : m_parent != nullptr ? m_parent->provider() : nullptr;
}

bool IDOSTreeItem::setProvider(IDOSTreeProvider* provider)
{
    if (m_provider != nullptr || provider == nullptr)
    {
        return false;
    }
    m_provider = provider;
    return true;
}

bool IDOSTreeItem::appendChild(IDOSTreeItem* item)
{
    if (m_tree != nullptr)
    {
        return m_tree->insert(this, item);
    }
    if (item == nullptr || item->m_parent != nullptr || item->m_tree != nullptr)
    {
        return false;
    }
    for (IDOSTreeItem* ancestor = this; ancestor != nullptr; ancestor = ancestor->parent())
    {
        if (ancestor == item)
        {
            return false;
        }
    }
    item->m_parent = this;
    m_children.append(item);
    return true;
}

void IDOSTreeItem::clearChildren()
{
    if (m_tree != nullptr)
    {
        while (!m_children.isEmpty())
        {
            if (!m_tree->remove(m_children.last()))
            {
                break;
            }
        }
        return;
    }
    qDeleteAll(m_children);
    m_children.clear();
}

void IDOSTreeItem::attach(IDOSTree* tree)
{
    m_tree = tree;
    for (IDOSTreeItem* child : m_children)
    {
        child->attach(tree);
    }
}

void IDOSTreeItem::changed()
{
    if (m_tree != nullptr)
    {
        m_tree->changed(this);
    }
}
