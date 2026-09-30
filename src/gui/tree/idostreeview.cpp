#include "idostreeview.h"
#include "idostreemenuprovider.h"
#include "idostreenode.h"
#include <QMenu>
#include <QContextMenuEvent>

IDOSTreeView::IDOSTreeView(QWidget* parent)
    : QTreeView(parent)
    , m_menuProvider(nullptr)
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

void IDOSTreeView::setMenuProvider(IDOSTreeMenuProvider* provider)
{
    m_menuProvider = provider;
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

    QTreeView::setModel(model);
    m_pendingExpandedPaths.clear();

    if (model != nullptr)
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
        const IDOSTreeNode* node = static_cast<const IDOSTreeNode*>(current.internalPointer());
        if (node != nullptr)
        {
            const QString key = node->nodeKey();
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

void IDOSTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    if (m_menuProvider == nullptr)
    {
        QTreeView::contextMenuEvent(event);
        return;
    }

    // 点击空区域则清除当前选中
    const QModelIndex clickedIndex =
        event->reason() == QContextMenuEvent::Keyboard ? currentIndex() : indexAt(event->pos());
    setCurrentIndex(clickedIndex);

    QMenu* menu = m_menuProvider->createContextMenu();
    if (menu != nullptr)
    {
        emit contextMenuAboutToShow(menu);

        if (!menu->actions().isEmpty())
        {
            menu->exec(mapToGlobal(event->pos()));
        }
        delete menu;
    }
}
