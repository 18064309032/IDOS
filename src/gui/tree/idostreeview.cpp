#include <memory>

#include <QAction>
#include <QApplication>
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
#include <QScrollBar>
#include <QStringList>
#include <QVariant>

#include "idosproject.h"
#include "idostreeguiprovider.h"
#include "idostreeserver.h"
#include "providers/idostreeregistry.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"

#include "idostreeview.h"

IDOSTreeView::IDOSTreeView(QWidget* parent)
    :
    QTreeView(parent)
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

void IDOSTreeView::setModel(QAbstractItemModel* model)
{
    if (this->model() != nullptr)
    {
        disconnect(this->model(), &QAbstractItemModel::rowsAboutToBeRemoved, this,
                   &IDOSTreeView::onRowsAboutToBeRemoved);
        disconnect(this->model(), &QAbstractItemModel::rowsInserted, this,
                   &IDOSTreeView::onRowsInserted);
        disconnect(this->model(), &QAbstractItemModel::modelAboutToBeReset, this,
                   &IDOSTreeView::onModelAboutToBeReset);
    }

    if (this->model() != nullptr)
    {
        disconnect(this->model(), nullptr, this, nullptr);
    }
    QTreeView::setModel(model);
    m_pendingExpandedPaths.clear();

    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model);
    if (treeModel != nullptr)
    {
        connect(treeModel, &IDOSTreeModel::checkRequested, this, &IDOSTreeView::onCheckRequested, Qt::DirectConnection);
        connect(treeModel, &QAbstractItemModel::dataChanged, this, &IDOSTreeView::onDataChanged);
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
            const QString key = node->key();
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
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    for (int row = first; row <= last; ++row)
    {
        if (treeModel != nullptr)
        {
            refreshChecks(treeModel->nodeFromIndex(model()->index(row, 0, parent)));
        }
        restoreExpandedPaths(model()->index(row, 0, parent));
    }
}

void IDOSTreeView::onModelAboutToBeReset()
{
    // 换工程：旧树的展开路径不得带到新树（分组键相同会误展开新工程的同名分组）
    m_pendingExpandedPaths.clear();
}

void IDOSTreeView::setRegistry(IDOSTreeRegistry* registry)
{
    if (m_registry != nullptr)
    {
        disconnect(m_registry, nullptr, this, nullptr);
    }
    m_registry = registry;
    if (registry != nullptr)
    {
        connect(registry, &IDOSTreeRegistry::guiProvidersChanged, this, &IDOSTreeView::onGuiProvidersChanged);
    }
    onGuiProvidersChanged();
}

void IDOSTreeView::refreshChecks(IDOSTreeItem* item)
{
    if (item == nullptr || m_registry == nullptr)
    {
        return;
    }
    std::shared_ptr<IDOSTreeGuiProvider> responsible;
    bool conflict = false;
    for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : m_registry->guiProviders())
    {
        if (provider->supports(item) && provider->isCheckable(item))
        {
            if (responsible)
            {
                conflict = true;
                break;
            }
            responsible = provider;
        }
    }
    if (responsible && !conflict)
    {
        item->setCheckable(responsible->isCheckable(item));
        item->setCheckState(responsible->checkState(item, this));
    }
    else
    {
        item->setCheckable(false);
        item->setCheckState(Qt::Unchecked);
    }
    for (IDOSTreeItem* child : item->children())
    {
        refreshChecks(child);
    }
}

void IDOSTreeView::onGuiProvidersChanged()
{
    if (m_registry != nullptr)
    {
        const QList<std::shared_ptr<IDOSTreeGuiProvider>> providers = m_registry->guiProviders();
        for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : providers)
        {
            disconnect(provider.get(), nullptr, this, nullptr);
            connect(provider.get(),
                    &IDOSTreeGuiProvider::checkStateChanged,
                    this,
                    &IDOSTreeView::onCheckStateChanged);
        }
    }
    refreshCheckStates();
}

void IDOSTreeView::refreshCheckStates()
{
    if (tree() != nullptr)
    {
        refreshChecks(tree()->root());
    }
}

void IDOSTreeView::onCheckStateChanged()
{
    refreshCheckStates();
}

void IDOSTreeView::onDataChanged(const QModelIndex& first, const QModelIndex& last)
{
    Q_UNUSED(last);
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (treeModel != nullptr)
    {
        refreshChecks(treeModel->nodeFromIndex(first));
    }
}

IDOSTree* IDOSTreeView::tree() const
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    return treeModel != nullptr ? treeModel->tree() : nullptr;
}

IDOSProject* IDOSTreeView::project() const
{
    return tree() != nullptr ? tree()->project() : nullptr;
}

QObject* IDOSTreeView::targetView() const
{
    return m_targetView.data();
}

void IDOSTreeView::setTargetView(QObject* view)
{
    m_targetView = view;
    refreshCheckStates();
}

QList<IDOSTreeItem*> IDOSTreeView::selectedItems() const
{
    QList<IDOSTreeItem*> result;
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (treeModel != nullptr && selectionModel() != nullptr)
    {
        for (const QModelIndex& index : selectionModel()->selectedRows())
        {
            result.append(treeModel->nodeFromIndex(index));
        }
    }
    return result;
}

IDOSTreeItem* IDOSTreeView::menuItem() const
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    return treeModel != nullptr ? treeModel->nodeFromIndex(m_menuIndex) : nullptr;
}

void IDOSTreeView::setBlankMenuEnabled(bool enabled)
{
    m_blankMenuEnabled = enabled;
}

QMenu* IDOSTreeView::createMenu(const QModelIndex& target)
{
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
    QMenu* menu = new QMenu(this);
    if (menuItem() == nullptr && !m_blankMenuEnabled)
    {
        return menu;
    }
    if (m_registry != nullptr && (!target.isValid() || selectedItems().size() <= 1))
    {
        const QList<std::shared_ptr<IDOSTreeGuiProvider>> providers = m_registry->guiProviders();
        for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : providers)
        {
            if (provider->supports(menuItem()))
            {
                provider->populateMenu(menu, this);
            }
        }
    }
    if (m_registry != nullptr)
    {
        for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : m_registry->guiProviders())
        {
            provider->populateSelectionMenu(menu, this);
        }
    }
    populateTransferMenu(menu, target);
    return menu;
}

void IDOSTreeView::onCheckRequested(IDOSTreeItem* item, bool checked, bool* accepted)
{
    if (m_registry == nullptr || item == nullptr || accepted == nullptr)
    {
        return;
    }
    std::shared_ptr<IDOSTreeGuiProvider> responsible;
    const QList<std::shared_ptr<IDOSTreeGuiProvider>> providers = m_registry->guiProviders();
    for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : providers)
    {
        if (provider->supports(item) && provider->isCheckable(item))
        {
            if (responsible)
            {
                return;
            }
            responsible = provider;
        }
    }
    if (responsible)
    {
        *accepted = responsible->setChecked(item, checked, this);
    }
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
    QMenu* menu = createMenu(target);
    if (!menu->actions().isEmpty())
    {
        const QPoint position = event->reason() == QContextMenuEvent::Keyboard
                                    ? viewport()->mapToGlobal(visualRect(currentIndex()).center())
                                    : event->globalPos();
        menu->exec(position);
    }
    delete menu;
    m_menuIndex = QPersistentModelIndex();
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
    if (m_registry == nullptr)
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
    QMimeData* result = nullptr;
    for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : m_registry->guiProviders())
    {
        QMimeData* candidate = provider->createMimeData(roots, this);
        if (candidate != nullptr)
        {
            if (result != nullptr)
            {
                delete result;
                delete candidate;
                return nullptr;
            }
            result = candidate;
        }
    }
    return result;
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
    int count = 0;
    if (m_registry != nullptr)
    {
        for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : m_registry->guiProviders())
        {
            if (provider->canCut(data, this))
            {
                ++count;
            }
        }
    }
    return count == 1;
}

bool IDOSTreeView::canPasteMimeData(const QMimeData* data, const QModelIndex& target, Qt::DropAction action) const
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (m_registry == nullptr || treeModel == nullptr)
    {
        return false;
    }
    IDOSTreeItem* item = treeModel->nodeFromIndex(target);
    int count = 0;
    for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : m_registry->guiProviders())
    {
        if (provider->canPaste(data, item, action, this))
        {
            ++count;
        }
    }
    return count == 1;
}

bool IDOSTreeView::pasteMimeData(const QMimeData* data, const QModelIndex& target, Qt::DropAction action)
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (m_registry == nullptr || treeModel == nullptr)
    {
        return false;
    }
    IDOSTreeItem* item = treeModel->nodeFromIndex(target);
    std::shared_ptr<IDOSTreeGuiProvider> responsible;
    for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : m_registry->guiProviders())
    {
        if (provider->canPaste(data, item, action, this))
        {
            if (responsible)
            {
                return false;
            }
            responsible = provider;
        }
    }
    return responsible && responsible->paste(data, item, action, this);
}

void IDOSTreeView::populateTransferMenu(QMenu* menu, const QModelIndex& target)
{
    QMimeData* source = selectedMimeData();
    const QMimeData* clipboard = QApplication::clipboard()->mimeData();
    const Qt::DropAction action = clipboard != nullptr && clipboard->property("idosCut").toBool()
        ? Qt::MoveAction : Qt::CopyAction;
    const bool canPaste = canPasteMimeData(clipboard, target, action);
    if (source == nullptr && !canPaste)
    {
        return;
    }
    if (!menu->actions().isEmpty())
    {
        menu->addSeparator();
    }
    if (source != nullptr)
    {
        source->setObjectName(QStringLiteral("treeTransferSource"));
        source->setParent(menu);
        QAction* copy = menu->addAction(tr("Copy"));
        copy->setObjectName(QStringLiteral("copyTreeItemsAction"));
        copy->setShortcut(QKeySequence::Copy);
        connect(copy, &QAction::triggered, this, &IDOSTreeView::onCopy);
        QAction* cut = menu->addAction(tr("Cut"));
        cut->setObjectName(QStringLiteral("cutTreeItemsAction"));
        cut->setShortcut(QKeySequence::Cut);
        cut->setEnabled(canCutMimeData(source));
        connect(cut, &QAction::triggered, this, &IDOSTreeView::onCut);
    }
    QAction* paste = menu->addAction(tr("Paste"));
    paste->setObjectName(QStringLiteral("pasteTreeItemsAction"));
    paste->setShortcut(QKeySequence::Paste);
    paste->setProperty("targetIndex", QVariant::fromValue(QPersistentModelIndex(target)));
    paste->setEnabled(canPaste);
    connect(paste, &QAction::triggered, this, &IDOSTreeView::onPaste);

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
    if (source != nullptr && source->project() != nullptr && source->project() == project())
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
    if (source != nullptr && source->project() == project() && event->possibleActions().testFlag(action) &&
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
            expanded.append(IDOSTreeServer::itemPath(static_cast<const IDOSTreeItem*>(index.internalPointer())));
        }
        collectPresentationState(index, expanded);
    }
}

QVariantMap IDOSTreeView::presentationState() const
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
        selected.append(IDOSTreeServer::itemPath(static_cast<const IDOSTreeItem*>(index.internalPointer())));
    }
    expanded.sort();
    selected.sort();
    state.insert(QStringLiteral("expanded"), expanded);
    state.insert(QStringLiteral("selected"), selected);
    state.insert(QStringLiteral("current"), IDOSTreeServer::itemPath(static_cast<const IDOSTreeItem*>(currentIndex().internalPointer())));
    state.insert(QStringLiteral("scroll"), verticalScrollBar()->value());
    state.insert(QStringLiteral("horizontalScroll"), horizontalScrollBar()->value());
    return state;
}

void IDOSTreeView::restorePresentationState(const QVariantMap& state)
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    if (treeModel == nullptr || state.isEmpty())
    {
        return;
    }
    collapseAll();
    for (const QString& path : state.value(QStringLiteral("expanded")).toStringList())
    {
        const QModelIndex index = treeModel->indexFromNode(IDOSTreeServer::itemFromPath(tree(), path));
        if (index.isValid())
        {
            setExpanded(index, true);
        }
    }
    selectionModel()->clearSelection();
    for (const QString& path : state.value(QStringLiteral("selected")).toStringList())
    {
        const QModelIndex index = treeModel->indexFromNode(IDOSTreeServer::itemFromPath(tree(), path));
        if (index.isValid())
        {
            selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
    const QModelIndex current = treeModel->indexFromNode(IDOSTreeServer::itemFromPath(tree(), state.value(QStringLiteral("current")).toString()));
    selectionModel()->setCurrentIndex(current, QItemSelectionModel::NoUpdate);
    doItemsLayout();
    verticalScrollBar()->setValue(state.value(QStringLiteral("scroll")).toInt());
    horizontalScrollBar()->setValue(state.value(QStringLiteral("horizontalScroll")).toInt());
}
