#include "idoscasetreemenuprovider.h"
#include "idoscasetreeview.h"
#include "idoscasetreemodel.h"
#include "idosdataobjecthandling.h"
#include <QAction>

IDOSCaseTreeMenuProvider::IDOSCaseTreeMenuProvider(IDOSCaseTreeView* view)
    : IDOSTreeMenuProvider(view)
    , m_view(view)
{
}
QMenu* IDOSCaseTreeMenuProvider::createContextMenu()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model == nullptr || model->project() == nullptr)
    {
        return nullptr;
    }
    QMenu* menu = new QMenu(m_view);
    QAction* createAction = menu->addAction(QIcon(QStringLiteral(":/images/gui-case.svg")), tr("New Case..."));
    createAction->setObjectName(QStringLiteral("newCaseAction"));
    connect(createAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onNewCase);
    QAction* importAction =
        menu->addAction(QIcon(QStringLiteral(":/images/gui-case-inputs.svg")), tr("Import Case..."));
    importAction->setObjectName(QStringLiteral("importCaseAction"));
    connect(importAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onImportCase);
    return menu;
}
void IDOSCaseTreeMenuProvider::onNewCase()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model != nullptr && IDOSDataObjectHandling::newCase(model->project(), m_view))
    {
        m_view->expandToDepth(0);
    }
}
void IDOSCaseTreeMenuProvider::onImportCase()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model != nullptr && IDOSDataObjectHandling::importCase(model->project(), m_view))
    {
        m_view->expandToDepth(1);
    }
}
