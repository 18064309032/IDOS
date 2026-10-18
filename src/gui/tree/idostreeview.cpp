#include <memory>

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QMenu>
#include <QMimeData>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPointer>
#include <QScrollBar>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

#include "idostreemenuprovider.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"

#include "idostreeview.h"

IDOSTreeView::IDOSTreeView(QWidget* parent)
    :
    QTreeView(parent)
    , m_menuProvider(nullptr)
    , m_blankMenuEnabled(false)
{
    setExpandsOnDoubleClick(true);
    setHeaderHidden(true);
    setIconSize(QSize(16, 16));
    setAnimated(true);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::CopyAction);
    QAction* copy = new QAction(this);
    copy->setObjectName(QStringLiteral("treeCopyShortcutAction"));
    copy->setShortcut(QKeySequence::Copy);
    copy->setShortcutContext(Qt::WidgetShortcut);
    addAction(copy);
    connect(copy, &QAction::triggered, this, &IDOSTreeView::onCopy);
    QAction* cut = new QAction(this);
    cut->setObjectName(QStringLiteral("treeCutShortcutAction"));
    cut->setShortcut(QKeySequence::Cut);
    cut->setShortcutContext(Qt::WidgetShortcut);
    addAction(cut);
    connect(cut, &QAction::triggered, this, &IDOSTreeView::onCut);
    QAction* paste = new QAction(this);
    paste->setObjectName(QStringLiteral("treePasteShortcutAction"));
    paste->setShortcut(QKeySequence::Paste);
    paste->setShortcutContext(Qt::WidgetShortcut);
    addAction(paste);
    connect(paste, &QAction::triggered, this, &IDOSTreeView::onPaste);
}

IDOSTreeView::~IDOSTreeView()
{
}

IDOSTreeModel* IDOSTreeView::treeModel() const
{
    return qobject_cast<IDOSTreeModel*>(model());
}

void IDOSTreeView::setModel(QAbstractItemModel* model)
{
    if (this->model() != nullptr)
    {
        disconnect(this->model(), nullptr, this, nullptr);
    }
    QTreeView::setModel(model);
    m_pendingExpandedPaths.clear();

    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model);
    if (treeModel != nullptr)
    {
        connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
                &IDOSTreeView::onRowsAboutToBeRemoved);
        connect(model, &QAbstractItemModel::rowsInserted, this, &IDOSTreeView::onRowsInserted);
        connect(model, &QAbstractItemModel::modelAboutToBeReset, this,
                &IDOSTreeView::onModelAboutToBeReset);
    }
}

QString IDOSTreeView::nodePath(const QModelIndex& index) const
{
    QStringList keys;
    for (QModelIndex current = index; current.isValid(); current = current.parent())
    {
        const IDOSTreeItem* node = static_cast<const IDOSTreeItem*>(current.internalPointer());
        if (node != nullptr)
        {
            const QString key = node->provider()->key();
            if (!key.isEmpty())
            {
                keys.prepend(key);
            }
        }
    }
    return keys.join(QLatin1Char('/'));
}

void IDOSTreeView::collectExpandedPaths(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    if (isExpanded(index))
    {
        const QString path = nodePath(index);
        if (!path.isEmpty())
        {
            m_pendingExpandedPaths.insert(path);
        }
    }
    const int count = model()->rowCount(index);
    for (int row = 0; row < count; ++row)
    {
        collectExpandedPaths(model()->index(row, 0, index));
    }
}

void IDOSTreeView::restoreExpandedPaths(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    const QString path = nodePath(index);
    if (!path.isEmpty() && m_pendingExpandedPaths.contains(path))
    {
        // 命中即消费：快照只服务于紧接着的这次插入，避免历史展开态污染后续折叠操作
        setExpanded(index, true);
        m_pendingExpandedPaths.remove(path);
    }
    const int count = model()->rowCount(index);
    for (int row = 0; row < count; ++row)
    {
        restoreExpandedPaths(model()->index(row, 0, index));
    }
}

void IDOSTreeView::onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last)
{
    for (int row = first; row <= last; ++row)
    {
        collectExpandedPaths(model()->index(row, 0, parent));
    }
}

void IDOSTreeView::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    for (int row = first; row <= last; ++row)
    {
        restoreExpandedPaths(model()->index(row, 0, parent));
    }
}

void IDOSTreeView::onModelAboutToBeReset()
{
    // 换工程：旧树的展开路径不得带到新树（分组键相同会误展开新工程的同名分组）
    m_pendingExpandedPaths.clear();
}

void IDOSTreeView::setMenuProvider(IDOSTreeMenuProvider* provider)
{
    if (provider == m_menuProvider || (provider != nullptr && provider->view() != this))
    {
        return;
    }
    delete m_menuProvider.data();
    m_menuProvider = provider;
}

IDOSTreeMenuProvider* IDOSTreeView::menuProvider() const
{
    return m_menuProvider.data();
}

QList<IDOSTreeItem*> IDOSTreeView::selectedItems() const
{
    QList<IDOSTreeItem*> result;
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (treeModel != nullptr && selectionModel() != nullptr)
    {
        for (const QModelIndex& index : selectionModel()->selectedRows())
        {
            result.append(treeModel->item(index));
        }
    }
    return result;
}

IDOSTreeItem* IDOSTreeView::contextItem() const
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    return treeModel != nullptr ? treeModel->item(m_menuIndex) : nullptr;
}

void IDOSTreeView::setBlankMenuEnabled(bool enabled)
{
    m_blankMenuEnabled = enabled;
}

QMenu* IDOSTreeView::createContextMenu(const QModelIndex& target)
{
    if (target.isValid() && target.model() != model())
    {
        return new QMenu(this);
    }
    if (target.isValid() && selectionModel() != nullptr)
    {
        if (!selectionModel()->isSelected(target))
        {
            selectionModel()->setCurrentIndex(target, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        else
        {
            selectionModel()->setCurrentIndex(target, QItemSelectionModel::NoUpdate);
        }
    }
    m_menuIndex = target;
    IDOSTreeItem* item = contextItem();
    QMenu* menu = nullptr;
    if (item != nullptr)
    {
        if (item->provider() != nullptr)
        {
            menu = item->provider()->createContextMenu(selectedItems());
        }
    }
    else if (m_blankMenuEnabled && m_menuProvider != nullptr)
    {
        menu = m_menuProvider->createContextMenu(QList<IDOSTreeItem*>(), nullptr);
    }
    if (menu == nullptr)
    {
        menu = new QMenu(this);
    }
    else if (menu->parent() == nullptr)
    {
        menu->setParent(this, menu->windowFlags());
    }
    if (item != nullptr || m_blankMenuEnabled)
    {
        emit contextMenuAboutToShow(menu);
    }
    return menu;
}

void IDOSTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    const QModelIndex target = event->reason() == QContextMenuEvent::Keyboard
                                   ? currentIndex() : indexAt(event->pos());
    if (!target.isValid() && !m_blankMenuEnabled)
    {
        m_menuIndex = QPersistentModelIndex();
        event->accept();
        return;
    }
    QPointer<IDOSTreeView> guard(this);
    QPointer<QMenu> menu = createContextMenu(target);
    if (guard == nullptr)
    {
        return;
    }
    if (menu != nullptr && !menu->actions().isEmpty())
    {
        const QPoint position = event->reason() == QContextMenuEvent::Keyboard
                                    ? viewport()->mapToGlobal(target.isValid() ? visualRect(target).center()
                                                                              : viewport()->rect().center())
                                    : event->globalPos();
        menu->exec(position);
    }
    delete menu.data();
    if (guard != nullptr)
    {
        m_menuIndex = QPersistentModelIndex();
    }
}

QMimeData* IDOSTreeView::cloneMimeData(const QMimeData* data) const
{
    if (data == nullptr)
    {
        return nullptr;
    }
    QMimeData* result = new QMimeData();
    for (const QString& format : data->formats())
    {
        result->setData(format, data->data(format));
    }
    result->setProperty("idosCut", data->property("idosCut"));
    return result;
}

QMimeData* IDOSTreeView::selectedMimeData() const
{
    if (m_menuProvider == nullptr)
    {
        return nullptr;
    }
    const QList<IDOSTreeItem*> selected = selectedItems();
    QList<IDOSTreeItem*> roots;
    for (IDOSTreeItem* item : selected)
    {
        bool covered = false;
        for (IDOSTreeItem* parent = item->parent(); parent != nullptr; parent = parent->parent())
        {
            if (selected.contains(parent))
            {
                covered = true;
                break;
            }
        }
        if (!covered)
        {
            roots.append(item);
        }
    }
    return m_menuProvider->createMimeData(roots);
}

QMimeData* IDOSTreeView::actionMimeData() const
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMenu* menu = action != nullptr ? qobject_cast<QMenu*>(action->parent()) : nullptr;
    if (menu != nullptr)
    {
        return cloneMimeData(menu->findChild<QMimeData*>(QStringLiteral("treeTransferSource")));
    }
    return selectedMimeData();
}

bool IDOSTreeView::canCutMimeData(const QMimeData* data) const
{
    return m_menuProvider != nullptr && m_menuProvider->canCut(data);
}

bool IDOSTreeView::canPasteMimeData(const QMimeData* data, const QModelIndex& target, Qt::DropAction action) const
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (treeModel == nullptr)
    {
        return false;
    }
    IDOSTreeItem* item = treeModel->item(target);
    return m_menuProvider != nullptr && m_menuProvider->canPaste(data, item, action);
}

bool IDOSTreeView::pasteMimeData(const QMimeData* data, const QModelIndex& target, Qt::DropAction action)
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (treeModel == nullptr)
    {
        return false;
    }
    IDOSTreeItem* item = treeModel->item(target);
    return m_menuProvider != nullptr && m_menuProvider->paste(data, item, action);
}

void IDOSTreeView::onCopy()
{
    std::unique_ptr<QMimeData> data(actionMimeData());
    if (data)
    {
        data->setProperty("idosCut", false);
        QApplication::clipboard()->setMimeData(data.release());
    }
}

void IDOSTreeView::onCut()
{
    std::unique_ptr<QMimeData> data(actionMimeData());
    if (data && canCutMimeData(data.get()))
    {
        data->setProperty("idosCut", true);
        QApplication::clipboard()->setMimeData(data.release());
    }
}

void IDOSTreeView::onPaste()
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMenu* menu = action != nullptr ? qobject_cast<QMenu*>(action->parent()) : nullptr;
    const QModelIndex target = menu != nullptr
        ? QModelIndex(action->property("targetIndex").value<QPersistentModelIndex>()) : currentIndex();
    QPointer<const QMimeData> original(QApplication::clipboard()->mimeData());
    std::unique_ptr<QMimeData> data(cloneMimeData(original.data()));
    if (!data)
    {
        return;
    }
    const bool cut = data->property("idosCut").toBool();
    if (pasteMimeData(data.get(), target, cut ? Qt::MoveAction : Qt::CopyAction) && cut &&
        original != nullptr && QApplication::clipboard()->mimeData() == original.data())
    {
        QApplication::clipboard()->clear();
    }
}


void IDOSTreeView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);
    QMimeData* data = selectedMimeData();
    if (data == nullptr)
    {
        return;
    }
    Qt::DropActions actions = Qt::CopyAction;
    if (canCutMimeData(data))
    {
        actions |= Qt::MoveAction;
    }
    QDrag* drag = new QDrag(this);
    drag->setMimeData(data);
    drag->exec(actions, Qt::CopyAction);
}

void IDOSTreeView::dragEnterEvent(QDragEnterEvent* event)
{
    IDOSTreeView* source = qobject_cast<IDOSTreeView*>(event->source());
    if (acceptsSource(source))
    {
        event->setDropAction(dragAction(event->keyboardModifiers()));
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void IDOSTreeView::dragMoveEvent(QDragMoveEvent* event)
{
    const QModelIndex target = indexAt(event->pos());
    const Qt::DropAction action = dragAction(event->keyboardModifiers());
    if (event->possibleActions().testFlag(action) && canPasteMimeData(event->mimeData(), target, action))
    {
        m_dropIndex = target;
        event->setDropAction(action);
        event->accept();
    }
    else
    {
        m_dropIndex = QPersistentModelIndex();
        event->ignore();
    }
    viewport()->update();
}

void IDOSTreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
    m_dropIndex = QPersistentModelIndex();
    viewport()->update();
    event->accept();
}

void IDOSTreeView::dropEvent(QDropEvent* event)
{
    const QModelIndex target = indexAt(event->pos());
    const Qt::DropAction action = dragAction(event->keyboardModifiers());
    m_dropIndex = QPersistentModelIndex();
    viewport()->update();
    IDOSTreeView* source = qobject_cast<IDOSTreeView*>(event->source());
    if (acceptsSource(source) && event->possibleActions().testFlag(action) &&
        pasteMimeData(event->mimeData(), target, action))
    {
        event->setDropAction(action);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void IDOSTreeView::paintEvent(QPaintEvent* event)
{
    QTreeView::paintEvent(event);
    if (m_dropIndex.isValid())
    {
        QPainter painter(viewport());
        QPen pen(palette().highlight().color());
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);
        painter.drawRect(visualRect(m_dropIndex).adjusted(1, 1, -2, -2));
    }
}
Qt::DropAction IDOSTreeView::dragAction(Qt::KeyboardModifiers modifiers) const
{
    return modifiers.testFlag(Qt::ShiftModifier) && !modifiers.testFlag(Qt::ControlModifier)
        ? Qt::MoveAction : Qt::CopyAction;
}

void IDOSTreeView::collectPresentationState(const QModelIndex& parent, QStringList& expanded) const
{
    for (int row = 0; row < model()->rowCount(parent); ++row)
    {
        const QModelIndex index = model()->index(row, 0, parent);
        if (isExpanded(index))
        {
            expanded.append(IDOSTreeModel::itemPath(static_cast<const IDOSTreeItem*>(index.internalPointer())));
        }
        collectPresentationState(index, expanded);
    }
}

QVariantMap IDOSTreeView::saveState() const
{
    QVariantMap state;
    if (model() == nullptr || selectionModel() == nullptr)
    {
        return state;
    }
    QStringList expanded;
    collectPresentationState(QModelIndex(), expanded);
    QStringList selected;
    for (const QModelIndex& index : selectionModel()->selectedRows())
    {
        selected.append(IDOSTreeModel::itemPath(static_cast<const IDOSTreeItem*>(index.internalPointer())));
    }
    expanded.sort();
    selected.sort();
    state.insert(QStringLiteral("expanded"), expanded);
    state.insert(QStringLiteral("selected"), selected);
    state.insert(QStringLiteral("current"), IDOSTreeModel::itemPath(static_cast<const IDOSTreeItem*>(currentIndex().internalPointer())));
    state.insert(QStringLiteral("scroll"), verticalScrollBar()->value());
    state.insert(QStringLiteral("horizontalScroll"), horizontalScrollBar()->value());
    return state;
}

void IDOSTreeView::restoreState(const QVariantMap& state)
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (treeModel == nullptr || state.isEmpty())
    {
        return;
    }
    collapseAll();
    for (const QString& path : state.value(QStringLiteral("expanded")).toStringList())
    {
        const QModelIndex index = treeModel->index(IDOSTreeModel::itemFromPath(treeModel, path));
        if (index.isValid())
        {
            setExpanded(index, true);
        }
    }
    selectionModel()->clearSelection();
    for (const QString& path : state.value(QStringLiteral("selected")).toStringList())
    {
        const QModelIndex index = treeModel->index(IDOSTreeModel::itemFromPath(treeModel, path));
        if (index.isValid())
        {
            selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
    const QModelIndex current = treeModel->index(IDOSTreeModel::itemFromPath(treeModel, state.value(QStringLiteral("current")).toString()));
    selectionModel()->setCurrentIndex(current, QItemSelectionModel::NoUpdate);
    doItemsLayout();
    verticalScrollBar()->setValue(state.value(QStringLiteral("scroll")).toInt());
    horizontalScrollBar()->setValue(state.value(QStringLiteral("horizontalScroll")).toInt());
}

bool IDOSTreeView::acceptsSource(const IDOSTreeView* source) const
{
    IDOSTreeModel* model = treeModel();
    IDOSTreeModel* sourceModel = source != nullptr ? source->treeModel() : nullptr;
    return model != nullptr && sourceModel != nullptr && !model->sessionId().isEmpty() &&
        model->sessionId() == sourceModel->sessionId();
}
