#include <QScopedValueRollback>

#include "idostreemodel.h"
#include "providers/idostreeitemprovider.h"

#include "idostreeitem.h"

IDOSTreeItem::IDOSTreeItem(IDOSTreeItemProvider* provider)
    :
    m_parent(nullptr)
    , m_model(nullptr)
    , m_provider(provider)
    , m_changing(false)
{
    const bool bound = provider != nullptr && provider->bindItem(this);
    Q_ASSERT(bound);
    Q_UNUSED(bound)
}

IDOSTreeItem::~IDOSTreeItem()
{
    qDeleteAll(m_children);
    delete m_provider;
}

IDOSTreeItem* IDOSTreeItem::parent() const
{
    return m_parent;
}

QList<IDOSTreeItem*> IDOSTreeItem::children() const
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

IDOSTreeModel* IDOSTreeItem::model() const
{
    return m_model;
}

IDOSTreeItemProvider* IDOSTreeItem::provider() const
{
    return m_provider;
}



bool IDOSTreeItem::appendChild(IDOSTreeItem* item)
{
    if (m_model != nullptr)
    {
        return m_model->insertItem(this, item);
    }
    return insertChild(childCount(), item);
}

bool IDOSTreeItem::insertChild(int row, IDOSTreeItem* item)
{
    if (m_model != nullptr && !m_model->isInserting(this, item, row))
    {
        return m_model->insertItem(this, item, row);
    }
    IDOSTreeItem* root = m_model != nullptr ? m_model->root() : this;
    if (root->m_changing || item == nullptr || item->parent() != nullptr || item->model() != nullptr ||
        row < 0 || row > childCount())
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
    QScopedValueRollback<bool> changing(root->m_changing, true);
    item->m_parent = this;
    m_children.insert(row, item);
    item->attach(m_model);
    return true;
}

bool IDOSTreeItem::removeChild(IDOSTreeItem* item)
{
    if (m_model != nullptr && !m_model->isRemoving(this, item))
    {
        return m_model->removeItem(item);
    }
    IDOSTreeItem* root = m_model != nullptr ? m_model->root() : this;
    if (root->m_changing || item == nullptr || item->parent() != this)
    {
        return false;
    }
    const int row = m_children.indexOf(item);
    if (row < 0)
    {
        return false;
    }
    QScopedValueRollback<bool> changing(root->m_changing, true);
    m_children.removeAt(row);
    item->m_parent = nullptr;
    item->attach(nullptr);
    delete item;
    return true;
}

bool IDOSTreeItem::setModel(IDOSTreeModel* model)
{
    if (model == nullptr || m_parent != nullptr || m_model != nullptr || model->root() != this)
    {
        return false;
    }
    attach(model);
    return true;
}

void IDOSTreeItem::clearChildren()
{
    if (m_model != nullptr)
    {
        while (!m_children.isEmpty())
        {
            if (!m_model->removeItem(m_children.last()))
            {
                break;
            }
        }
        return;
    }
    qDeleteAll(m_children);
    m_children.clear();
}

void IDOSTreeItem::attach(IDOSTreeModel* model)
{
    m_model = model;
    for (IDOSTreeItem* child : m_children)
    {
        child->attach(model);
    }
}

IDOSTreeItem* IDOSTreeItem::findChild(const QString& key)
{
    for (IDOSTreeItem* child : m_children)
    {
        if (child->provider()->key() == key)
        {
            if (m_model != nullptr)
            {
                m_model->retainItem(child);
            }
            return child;
        }
    }
    return nullptr;
}
