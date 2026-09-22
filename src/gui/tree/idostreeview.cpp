#include "idostreeview.h"
#include "idostreemenuprovider.h"
#include <QMenu>
#include <QContextMenuEvent>

IDOSTreeView::IDOSTreeView(QWidget* parent)
    : QTreeView(parent)
    , m_menuProvider(nullptr)
{
    setExpandsOnDoubleClick(true);
    setHeaderHidden(true);
    setAnimated(true);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

IDOSTreeView::~IDOSTreeView() = default;

void IDOSTreeView::setMenuProvider(IDOSTreeMenuProvider* provider)
{
    m_menuProvider = provider;
}

void IDOSTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    if (m_menuProvider == nullptr)
    {
        QTreeView::contextMenuEvent(event);
        return;
    }

    // 点击空区域则清除当前选中
    const QModelIndex clickedIndex = indexAt(event->pos());
    if (!clickedIndex.isValid())
    {
        setCurrentIndex(QModelIndex());
    }

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
