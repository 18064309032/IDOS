#ifndef IDOS_TREE_MODEL_H
#define IDOS_TREE_MODEL_H

#include <QAbstractItemModel>
#include <QPointer>

#include "idos_core.h"

class IDOSDataObject;
class IDOSProject;
class IDOSTree;
class IDOSTreeItem;

class CORE_EXPORT IDOSTreeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    explicit IDOSTreeModel(IDOSTree* tree, QObject* parent = nullptr);
    ~IDOSTreeModel() override;
    IDOSTree* tree() const;
    IDOSProject* project() const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    IDOSTreeItem* nodeFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromNode(const IDOSTreeItem* item) const;
    QModelIndex indexFromDataId(const QString& id) const;
    IDOSDataObject* objectFromIndex(const QModelIndex& index) const;

  signals:
    void checkRequested(IDOSTreeItem* item, bool checked, bool* accepted);

  private slots:
    void onAboutToInsert(IDOSTreeItem* parent, int row);
    void onInserted();
    void onAboutToRemove(IDOSTreeItem* parent, int row);
    void onRemoved();
    void onItemChanged(IDOSTreeItem* item);
    void onTreeDestroyed();
    void onProjectChanged();

  private:
    IDOSTreeItem* findData(IDOSTreeItem* branch, const QString& id) const;
    QPointer<IDOSTree> m_tree;
};

#endif
