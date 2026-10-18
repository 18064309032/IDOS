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

class IDOSTreeModel;
class IDOSTreeItem;
class IDOSTreeMenuProvider;
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
    void setModel(QAbstractItemModel* model) override;
    IDOSTreeModel* treeModel() const;
    void setMenuProvider(IDOSTreeMenuProvider* provider);
    IDOSTreeMenuProvider* menuProvider() const;
    QList<IDOSTreeItem*> selectedItems() const;
    IDOSTreeItem* contextItem() const;
    void setBlankMenuEnabled(bool enabled);
    QMenu* createContextMenu(const QModelIndex& target);
    QVariantMap saveState() const;
    void restoreState(const QVariantMap& state);
    QMimeData* selectedMimeData() const;
    bool pasteMimeData(const QMimeData* data, const QModelIndex& target, Qt::DropAction action);

  signals:
    void contextMenuAboutToShow(QMenu* menu);

  protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

  public slots:
    void onCopy();
    void onCut();
    void onPaste();

  private slots:
    void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onModelAboutToBeReset();

  private:
    bool acceptsSource(const IDOSTreeView* source) const;
    void collectPresentationState(const QModelIndex& parent, QStringList& expanded) const;
    QMimeData* actionMimeData() const;
    QMimeData* cloneMimeData(const QMimeData* data) const;
    bool canCutMimeData(const QMimeData* data) const;
    bool canPasteMimeData(const QMimeData* data, const QModelIndex& target, Qt::DropAction action) const;
    Qt::DropAction dragAction(Qt::KeyboardModifiers modifiers) const;
    QString nodePath(const QModelIndex& index) const;
    void collectExpandedPaths(const QModelIndex& index);
    void restoreExpandedPaths(const QModelIndex& index);
    QPointer<IDOSTreeMenuProvider> m_menuProvider;
    QPersistentModelIndex m_menuIndex;
    QPersistentModelIndex m_dropIndex;
    QSet<QString> m_pendingExpandedPaths;
    bool m_blankMenuEnabled;
};

#endif
