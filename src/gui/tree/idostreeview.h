#ifndef IDOS_TREE_VIEW_H
#define IDOS_TREE_VIEW_H

#include <QList>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QTreeView>

#include "idos_gui.h"

class IDOSProject;
class IDOSTree;
class IDOSTreeItem;
class IDOSTreeRegistry;
class QMenu;

class GUI_EXPORT IDOSTreeView : public QTreeView
{
    Q_OBJECT

  public:
    explicit IDOSTreeView(QWidget* parent = nullptr);
    ~IDOSTreeView() override;
    void setRegistry(IDOSTreeRegistry* registry);
    void setModel(QAbstractItemModel* model) override;
    IDOSTree* tree() const;
    IDOSProject* project() const;
    QObject* targetView() const;
    void setTargetView(QObject* view);
    QList<IDOSTreeItem*> selectedItems() const;
    IDOSTreeItem* menuItem() const;
    void setBlankMenuEnabled(bool enabled);
    QMenu* createMenu(const QModelIndex& target);
    void refreshCheckStates();

  protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

  private slots:
    void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onModelAboutToBeReset();
    void onCheckRequested(IDOSTreeItem* item, bool checked, bool* accepted);
    void onGuiProvidersChanged();
    void onDataChanged(const QModelIndex& first, const QModelIndex& last);
    void onCheckStateChanged();

  private:
    QString nodePath(const QModelIndex& index) const;
    void collectExpandedPaths(const QModelIndex& index);
    void restoreExpandedPaths(const QModelIndex& index);
    void refreshChecks(IDOSTreeItem* item);
    QPointer<IDOSTreeRegistry> m_registry;
    QPointer<QObject> m_targetView;
    QPersistentModelIndex m_menuIndex;
    QSet<QString> m_pendingExpandedPaths;
    bool m_blankMenuEnabled;
};

#endif
