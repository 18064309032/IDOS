#include <memory>

#include <QContextMenuEvent>
#include <QItemSelectionModel>
#include <QMenu>
#include <QStringList>

#include "idosproject.h"
#include "idostree.h"
#include "idostreeguiprovider.h"
#include "idostreeitem.h"
#include "idostreemodel.h"
#include "idostreeregistry.h"

#include "idostreeview.h"

IDOSTreeView::IDOSTreeView(QWidget* parent)
    :
    QTreeView(parent)

{
    setExpandsOnDoubleClick(true);
    setHeaderHidden(true);
    setIconSize(QSize(16, 16));
    setAnimated(true);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
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
        item->setChecked(responsible->isChecked(item, this));
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

IDOSTreeItem* IDOSTreeView::currentItem() const
{
    IDOSTreeModel* treeModel = qobject_cast<IDOSTreeModel*>(model());
    return treeModel != nullptr ? treeModel->nodeFromIndex(currentIndex()) : nullptr;
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

QMenu* IDOSTreeView::createMenu(const QModelIndex& target)
{
    m_menuIndex = target;
    QMenu* menu = new QMenu(this);
    if (m_registry != nullptr)
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
    emit contextMenuAboutToShow(menu);
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
