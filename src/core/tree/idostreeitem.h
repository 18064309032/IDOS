#ifndef IDOS_TREE_ITEM_H
#define IDOS_TREE_ITEM_H

#include <QIcon>
#include <QString>
#include <QVector>

#include "idos_core.h"

class IDOSTree;
class IDOSTreeProvider;

class CORE_EXPORT IDOSTreeItem
{
  public:
    IDOSTreeItem();
    ~IDOSTreeItem();
    IDOSTreeItem(const IDOSTreeItem&) = delete;
    IDOSTreeItem& operator=(const IDOSTreeItem&) = delete;
    QString id() const;
    QString key() const;
    void setKey(const QString& key);
    QString name() const;
    void setName(const QString& name);
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    QString dataId() const;
    void setDataId(const QString& id);
    QString caseId() const;
    void setCaseId(const QString& id);
    QString partKey() const;
    void setPartKey(const QString& key);
    QString itemKey() const;
    void setItemKey(const QString& key);
    bool isGroup() const;
    void setGroup(bool group);
    bool isReference() const;
    void setReference(bool reference);
    bool isCheckable() const;
    void setCheckable(bool checkable);
    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);
    bool isChecked() const;
    void setChecked(bool checked);
    IDOSTreeItem* parent() const;
    QVector<IDOSTreeItem*> children() const;
    IDOSTreeItem* child(int row) const;
    int childCount() const;
    int row() const;
    IDOSTree* tree() const;
    IDOSTreeProvider* provider() const;
    // Takes ownership on success. An existing provider cannot be replaced.
    bool setProvider(IDOSTreeProvider* provider);
    bool appendChild(IDOSTreeItem* item);
    void clearChildren();

  private:
    friend class IDOSTree;
    void attach(IDOSTree* tree);
    void changed();
    QString m_id;
    QString m_key;
    QString m_name;
    QIcon m_icon;
    QString m_dataId;
    QString m_caseId;
    QString m_partKey;
    QString m_itemKey;
    bool m_group;
    bool m_reference;
    bool m_checkable;
    Qt::CheckState m_checkState;
    IDOSTreeItem* m_parent;
    IDOSTree* m_tree;
    IDOSTreeProvider* m_provider;
    QVector<IDOSTreeItem*> m_children;
};

#endif
