#ifndef IDOS_TREE_ITEM_PROVIDER_H
#define IDOS_TREE_ITEM_PROVIDER_H

#include <QHash>
#include <QIcon>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtCore/qnamespace.h>

#include "idos_core.h"

class IDOSDataObject;
class IDOSTreeItem;
class IDOSTreeItemProviderRegistry;
class QMenu;

class CORE_EXPORT IDOSTreeItemProvider : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSTreeItemProvider(IDOSDataObject* data = nullptr);
    virtual ~IDOSTreeItemProvider();
    IDOSTreeItem* item() const;
    bool setItem(IDOSTreeItem* item);
    QString metadataId() const;
    bool setMetadataId(const QString& id);
    IDOSDataObject* object() const;
    virtual QString id() const;
    virtual QString key() const;
    void setKey(const QString& value);
    virtual QString name() const;
    void setName(const QString& value);
    virtual QIcon icon() const;
    void setIcon(const QIcon& value);
    virtual QString dataId() const;
    void setDataId(const QString& value);
    virtual QString caseId() const;
    void setCaseId(const QString& value);
    virtual QString partKey() const;
    void setPartKey(const QString& value);
    virtual QString itemKey() const;
    void setItemKey(const QString& value);
    virtual bool isGroup() const;
    void setGroup(bool value);
    virtual bool isReference() const;
    void setReference(bool value);
    virtual bool isCheckable() const;
    void setCheckable(bool value);
    void setCheckEnabled(bool enabled);
    virtual Qt::CheckState checkState() const;
    virtual QString displayName() const;
    bool isChecked() const;
    virtual void setChecked(Qt::CheckState state);
    virtual QVariant data(int role) const;
    virtual bool setData(const QVariant& value, int role);
    virtual Qt::ItemFlags flags() const;
    virtual QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems = QList<IDOSTreeItem*>());
    virtual void refresh();
    virtual bool dependsOn(const QStringList& dataIds) const;

  private:
    QString m_id;
    QString m_metadataId;
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
    bool m_checkEnabled;
    Qt::CheckState m_checkState;
    QHash<int, QVariant> m_roles;
    IDOSTreeItem* m_item;
    QPointer<IDOSDataObject> m_data;
};

#endif
