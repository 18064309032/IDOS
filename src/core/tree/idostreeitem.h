#ifndef IDOS_TREE_ITEM_H
#define IDOS_TREE_ITEM_H

#include <QList>

#include "idos_core.h"
#include "providers/idostreeitemprovider.h"

class IDOSTreeModel;

class CORE_EXPORT IDOSTreeItem
{
  public:
    explicit IDOSTreeItem(IDOSTreeItemProvider* provider);
    ~IDOSTreeItem();
    IDOSTreeItem(const IDOSTreeItem&) = delete;
    IDOSTreeItem& operator=(const IDOSTreeItem&) = delete;
    IDOSTreeItem* parent() const;
    QList<IDOSTreeItem*> children() const;
    IDOSTreeItem* child(int row) const;
    int childCount() const;
    IDOSTreeItem* findChild(const QString& key);
    int row() const;
    IDOSTreeModel* model() const;
    IDOSTreeItemProvider* provider() const;
    bool appendChild(IDOSTreeItem* item);
    bool insertChild(int row, IDOSTreeItem* item);
    bool removeChild(IDOSTreeItem* item);
    bool setModel(IDOSTreeModel* model);
    void clearChildren();

  private:
    void attach(IDOSTreeModel* model);
    IDOSTreeItem* m_parent;
    IDOSTreeModel* m_model;
    IDOSTreeItemProvider* m_provider;

    QList<IDOSTreeItem*> m_children;
    bool m_changing;
};

#endif
