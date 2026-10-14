#ifndef IDOS_TREE_VIEW_H
#define IDOS_TREE_VIEW_H

#include <QList>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTreeView>
#include <QVariantMap>

#include "idos_gui.h"

class IDOSProject;
class IDOSTree;
class IDOSTreeItem;
class IDOSTreeRegistry;
class QMenu;
class QMimeData;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QPaintEvent;

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
    QVariantMap presentationState() const;
    void restorePresentationState(const QVariantMap& state);
    bool pasteMimeData(const QMimeData* data, const QModelIndex& target, Qt::DropAction action);

  protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

  private slots:
    void onCopy();
    void onCut();
    void onPaste();
    void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onModelAboutToBeReset();
    void onCheckRequested(IDOSTreeItem* item, bool checked, bool* accepted);
    void onGuiProvidersChanged();
    void onDataChanged(const QModelIndex& first, const QModelIndex& last);
    void onCheckStateChanged();

  private:
    void collectPresentationState(const QModelIndex& parent, QStringList& expanded) const;
    QMimeData* selectedMimeData() const;
    QMimeData* actionMimeData() const;
    QMimeData* cloneMimeData(const QMimeData* data) const;
    bool canCutMimeData(const QMimeData* data) const;
    bool canPasteMimeData(const QMimeData* data, const QModelIndex& target, Qt::DropAction action) const;
    void populateTransferMenu(QMenu* menu, const QModelIndex& target);
    Qt::DropAction dragAction(Qt::KeyboardModifiers modifiers) const;
    QString nodePath(const QModelIndex& index) const;
    void collectExpandedPaths(const QModelIndex& index);
    void restoreExpandedPaths(const QModelIndex& index);
    void refreshChecks(IDOSTreeItem* item);
    QPointer<IDOSTreeRegistry> m_registry;
    QPointer<QObject> m_targetView;
    QPersistentModelIndex m_menuIndex;
    QPersistentModelIndex m_dropIndex;
    QSet<QString> m_pendingExpandedPaths;
    bool m_blankMenuEnabled;
};

#endif
