#include "idosdatatreemenuprovider.h"
#include "idosdataobjecthandling.h"
#include "idosdatatreemodel.h"
#include "idosdatatreeview.h"
#include "idostreegroupnode.h"
#include <QAction>

IDOSDataTreeMenuProvider::IDOSDataTreeMenuProvider(IDOSDataTreeView* view)
    : IDOSTreeMenuProvider(view)
    , m_view(view)
{
}

QMenu* IDOSDataTreeMenuProvider::createContextMenu()
{
    IDOSDataTreeModel* model = qobject_cast<IDOSDataTreeModel*>(m_view->model());
    if (model == nullptr || model->project() == nullptr)
    {
        return nullptr;
    }
    IDOSTreeGroupNode* group = dynamic_cast<IDOSTreeGroupNode*>(model->nodeFromIndex(m_view->currentIndex()));
    if (group == nullptr || group->groupKey() != QStringLiteral("data.wells"))
    {
        return nullptr;
    }
    QMenu* menu = new QMenu(m_view);
    QAction* newWellAction = menu->addAction(QIcon(QStringLiteral(":/images/gui-well-new.svg")), tr("New Well..."));
    newWellAction->setObjectName(QStringLiteral("newWellAction"));
    connect(newWellAction, &QAction::triggered, this, &IDOSDataTreeMenuProvider::onNewWellTriggered);
    QAction* importAction =
        menu->addAction(QIcon(QStringLiteral(":/images/gui-well-import.svg")), tr("Import Well Data..."));
    importAction->setObjectName(QStringLiteral("importWellAction"));
    connect(importAction, &QAction::triggered, this, &IDOSDataTreeMenuProvider::onImportWellTriggered);
    return menu;
}

void IDOSDataTreeMenuProvider::onNewWellTriggered()
{
    IDOSDataTreeModel* model = qobject_cast<IDOSDataTreeModel*>(m_view->model());
    if (model != nullptr && IDOSDataObjectHandling::newWell(model->project(), m_view))
    {
        m_view->expandToDepth(1);
    }
}

void IDOSDataTreeMenuProvider::onImportWellTriggered()
{
    IDOSDataTreeModel* model = qobject_cast<IDOSDataTreeModel*>(m_view->model());
    if (model != nullptr && IDOSDataObjectHandling::importWellData(model->project(), m_view) > 0)
    {
        m_view->expandToDepth(1);
    }
}
