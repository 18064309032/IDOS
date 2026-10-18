#ifndef IDOS_TREE_MODEL_H
#define IDOS_TREE_MODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

#include "idos_core.h"

class IDOSDataObject;
class IDOSTreeItem;
class IDOSTreeItemProvider;
class IDOSTreeItemProviderRegistry;

class CORE_EXPORT IDOSTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Role
    {
        RenderAvailableRole = Qt::UserRole + 1
    };

    explicit IDOSTreeModel(IDOSTreeItemProviderRegistry* registry,
                          QObject* parent = nullptr);
    ~IDOSTreeModel() override;
    IDOSTreeItem* root() const;
    // Takes ownership of an unbound provider; rejected requests retain caller ownership.
    void setRootProvider(IDOSTreeItemProvider* provider);
    IDOSTreeItemProviderRegistry* registry() const;
    bool transferItems(const QVariantMap& arguments, bool validateOnly = false);
    QString sessionId() const;
    void setSessionId(const QString& sessionId);
    QList<IDOSDataObject*> objects() const;
    IDOSDataObject* object(const QString& dataId) const;
    QString name() const;
    void setName(const QString& name);
    IDOSTreeItem* item(const QString& id) const;
    IDOSTreeItem* find(const QString& key) const;
    bool insertItem(IDOSTreeItem* parent, IDOSTreeItem* item, int row = -1);
    bool removeItem(IDOSTreeItem* item);
    void clear();
    void refresh();
    void refreshItem(IDOSTreeItem* item);
    void retainItem(IDOSTreeItem* item);
    void addObject(IDOSDataObject* data);
    void updateItem(IDOSTreeItem* item, const QList<int>& roles = QList<int>());
    void setCheckState(IDOSTreeItem* item, Qt::CheckState state, bool checkable);
    void refreshCheckStates();
    bool setChecked(IDOSTreeItem* item, Qt::CheckState state);
    bool isInserting(const IDOSTreeItem* parent, const IDOSTreeItem* item, int row) const;
    bool isRemoving(const IDOSTreeItem* parent, const IDOSTreeItem* item) const;
    IDOSTreeItem* item(const QModelIndex& index) const;
    QModelIndex index(const IDOSTreeItem* item) const;
    QModelIndex index(const QString& id) const;
    IDOSDataObject* object(const QModelIndex& index) const;
    static QString itemPath(const IDOSTreeItem* item);
    static IDOSTreeItem* itemFromPath(IDOSTreeModel* model, const QString& path);
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
signals:
    void checkStateChanged(IDOSTreeItem* item, Qt::CheckState state);
    void registryChanged();
    void transferRequested(const QVariantMap& arguments, bool validateOnly, bool& accepted);
protected:
    bool acceptsContext(const QString& session, const QString& itemId) const;
private slots:
    void onMetadataRegistered(const QString& metadataId);
    void onMetadataRemoving(const QString& metadataId);
private:
    void refreshCheckStates(IDOSTreeItem* item);
    IDOSTreeItem* lookup(IDOSTreeItem* branch, const QString& value, bool byId) const;
    IDOSTreeItem* findData(IDOSTreeItem* branch, const QString& id) const;
    void forgetRefreshItems(IDOSTreeItem* item);
    void collectRefreshItems(IDOSTreeItem* item, QList<IDOSTreeItem*>& items);
    void removeMetadataItems(const QString& metadataId);
    IDOSTreeItem* m_root;
    QPointer<IDOSTreeItemProviderRegistry> m_registry;
    QString m_name;
    QString m_sessionId;
    QSet<IDOSTreeItem*> m_refreshItems;
    bool m_changing;
    bool m_committing;
    bool m_resetting;
    bool m_refreshing;
    bool m_inserting;
    IDOSTreeItem* m_changeParent;
    IDOSTreeItem* m_changeItem;
    int m_changeRow;
};

#endif
