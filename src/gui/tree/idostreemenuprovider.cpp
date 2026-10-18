#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QKeySequence>
#include <QMenu>
#include <QMimeData>
#include <QPersistentModelIndex>
#include <QVariant>

#include "idostreeview.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"

#include "idostreemenuprovider.h"

IDOSTreeMenuProvider::IDOSTreeMenuProvider(IDOSTreeView* view)
    :
    QObject(view)
    , m_view(view)
{
    connect(view, &IDOSTreeView::contextMenuAboutToShow, this, &IDOSTreeMenuProvider::onContextMenuAboutToShow);
}

IDOSTreeView* IDOSTreeMenuProvider::view() const
{
    return m_view.data();
}

IDOSTreeMenuProvider::~IDOSTreeMenuProvider()
{
}

QMimeData* IDOSTreeMenuProvider::createMimeData(const QList<IDOSTreeItem*>& items) const
{
    Q_UNUSED(items)
    return nullptr;
}

bool IDOSTreeMenuProvider::canCut(const QMimeData* data) const
{
    Q_UNUSED(data)
    return false;
}

bool IDOSTreeMenuProvider::canPaste(const QMimeData* data, const IDOSTreeItem* target,
                                    Qt::DropAction action) const
{
    Q_UNUSED(data)
    Q_UNUSED(target)
    Q_UNUSED(action)
    return false;
}

bool IDOSTreeMenuProvider::paste(const QMimeData* data, IDOSTreeItem* target,
                                 Qt::DropAction action)
{
    Q_UNUSED(data)
    Q_UNUSED(target)
    Q_UNUSED(action)
    return false;
}

void IDOSTreeMenuProvider::onContextMenuAboutToShow(QMenu* menu)
{
    IDOSTreeView* treeView = view();
    IDOSTreeModel* model = treeView != nullptr ? treeView->treeModel() : nullptr;
    if (model == nullptr)
    {
        return;
    }
    IDOSTreeItem* context = treeView->contextItem();
    const QModelIndex target = model->index(context);
    QMimeData* source = context != nullptr ? treeView->selectedMimeData() : nullptr;
    const QMimeData* clipboard = QApplication::clipboard()->mimeData();
    const Qt::DropAction action = clipboard != nullptr && clipboard->property("idosCut").toBool()
        ? Qt::MoveAction : Qt::CopyAction;
    const bool canPaste = this->canPaste(clipboard, context, action);
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
        connect(copy, &QAction::triggered, treeView, &IDOSTreeView::onCopy);
        QAction* cut = menu->addAction(tr("Cut"));
        cut->setObjectName(QStringLiteral("cutTreeItemsAction"));
        cut->setShortcut(QKeySequence::Cut);
        cut->setEnabled(canCut(source));
        connect(cut, &QAction::triggered, treeView, &IDOSTreeView::onCut);
    }
    QAction* paste = menu->addAction(tr("Paste"));
    paste->setObjectName(QStringLiteral("pasteTreeItemsAction"));
    paste->setShortcut(QKeySequence::Paste);
    paste->setProperty("targetIndex", QVariant::fromValue(QPersistentModelIndex(target)));
    paste->setEnabled(canPaste);
    connect(paste, &QAction::triggered, treeView, &IDOSTreeView::onPaste);

}
