#include "idosdatatreemenuprovider.h"
#include "idosdataobjecthandling.h"
#include "idosdatatreemodel.h"
#include "idosdatatreeview.h"
#include "idostreegroupnode.h"
#include "idostreenode.h"
#include "idostreemodel.h"
#include "idosdataobject.h"
#include "idoswell.h"
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
    const QModelIndex index = m_view->currentIndex();
    IDOSTreeNode* node = model->nodeFromIndex(index);
    if (node == nullptr)
    {
        return nullptr;
    }
    // 井组分组节点：批量导入 + 新建井
    IDOSTreeGroupNode* group = dynamic_cast<IDOSTreeGroupNode*>(node);
    if (group != nullptr)
    {
        if (group->groupKey() != QStringLiteral("data.wells"))
        {
            return nullptr;
        }
        QMenu* menu = new QMenu(m_view);
        QAction* newWellAction =
            menu->addAction(QIcon(QStringLiteral(":/images/gui-well-new.svg")), tr("New Well..."));
        newWellAction->setObjectName(QStringLiteral("newWellAction"));
        connect(newWellAction, &QAction::triggered, this, &IDOSDataTreeMenuProvider::onNewWellTriggered);
        QAction* importAction =
            menu->addAction(QIcon(QStringLiteral(":/images/gui-well-import.svg")), tr("Import Well Data..."));
        importAction->setObjectName(QStringLiteral("importWellAction"));
        connect(importAction, &QAction::triggered, this, &IDOSDataTreeMenuProvider::onImportWellDataTriggered);
        return menu;
    }
    // 对象节点：数据树仅管理原始输入数据，网格/属性归工况树不在此分派
    IDOSDataObject* object = model->objectFromIndex(index);
    IDOSWell* well = qobject_cast<IDOSWell*>(object);
    if (well == nullptr)
    {
        return nullptr;
    }
    QMenu* menu = new QMenu(m_view);
    QAction* importAction =
        menu->addAction(QIcon(QStringLiteral(":/images/gui-well-import.svg")), tr("Import Data..."));
    importAction->setObjectName(QStringLiteral("importWellDataSingleAction"));
    connect(importAction, &QAction::triggered, this,
            &IDOSDataTreeMenuProvider::onImportWellDataSingleTriggered);
    menu->addSeparator();
    QAction* deleteAction = menu->addAction(tr("Delete Well..."));
    deleteAction->setObjectName(QStringLiteral("deleteWellAction"));
    connect(deleteAction, &QAction::triggered, this, &IDOSDataTreeMenuProvider::onDeleteWellTriggered);
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

void IDOSDataTreeMenuProvider::onImportWellDataTriggered()
{
    IDOSDataTreeModel* model = qobject_cast<IDOSDataTreeModel*>(m_view->model());
    if (model != nullptr && IDOSDataObjectHandling::importWellData(model->project(), m_view) > 0)
    {
        m_view->expandToDepth(2);
    }
}

void IDOSDataTreeMenuProvider::onImportWellDataSingleTriggered()
{
    IDOSDataTreeModel* model = qobject_cast<IDOSDataTreeModel*>(m_view->model());
    if (model == nullptr)
    {
        return;
    }
    IDOSDataObject* object = model->objectFromIndex(m_view->currentIndex());
    if (object == nullptr)
    {
        return;
    }
    if (IDOSDataObjectHandling::importWellData(model->project(), object->objectId(), m_view) > 0)
    {
        m_view->expandToDepth(2);
    }
}

void IDOSDataTreeMenuProvider::onDeleteWellTriggered()
{
    IDOSDataTreeModel* model = qobject_cast<IDOSDataTreeModel*>(m_view->model());
    if (model == nullptr)
    {
        return;
    }
    IDOSDataObject* object = model->objectFromIndex(m_view->currentIndex());
    if (object == nullptr)
    {
        return;
    }
    IDOSDataObjectHandling::deleteWell(model->project(), object->objectId(), m_view);
}
