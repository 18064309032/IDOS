#include <QAction>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "command/idoscommandmanager.h"
#include "idosdataobject.h"
#include "idospointset.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "idostree.h"
#include "idostreeitem.h"
#include "idostreeview.h"

#include "idospointsettreeguiprovider.h"

IDOSPointSetTreeGuiProvider::~IDOSPointSetTreeGuiProvider()
{
    for (const QPointer<QAction>& action : m_actions)
    {
        delete action.data();
    }
}

QString IDOSPointSetTreeGuiProvider::name() const
{
    return QString::fromLatin1("idos.example.pointset");
}

bool IDOSPointSetTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    if (item == nullptr)
    {
        return false;
    }
    IDOSProject* project = item->tree() != nullptr ? item->tree()->project() : nullptr;
    IDOSDataObject* data = project != nullptr ? project->object(item->dataId()) : nullptr;
    return data != nullptr && data->typeId() == name();
}

bool IDOSPointSetTreeGuiProvider::isChecked(const IDOSTreeItem* item,
                                            const IDOSTreeView* view) const
{
    IDOSRenderView* renderView = view != nullptr
                                     ? qobject_cast<IDOSRenderView*>(view->targetView())
                                     : nullptr;
    IDOSPointSet* pointSet = item != nullptr && item->tree() != nullptr &&
                                     item->tree()->project() != nullptr
                                 ? qobject_cast<IDOSPointSet*>(
                                       item->tree()->project()->object(item->dataId()))
                                 : nullptr;
    if (renderView == nullptr || pointSet == nullptr)
    {
        return false;
    }
    for (IDOSRenderObject* object : renderView->scene()->objects())
    {
        if (object->provider()->dataSource() == pointSet)
        {
            return object->visible();
        }
    }
    return false;
}

bool IDOSPointSetTreeGuiProvider::setChecked(IDOSTreeItem* item,
                                             bool checked,
                                             IDOSTreeView* view)
{
    IDOSRenderView* renderView = view != nullptr
                                     ? qobject_cast<IDOSRenderView*>(view->targetView())
                                     : nullptr;
    IDOSPointSet* pointSet = item != nullptr && item->tree() != nullptr &&
                                     item->tree()->project() != nullptr
                                 ? qobject_cast<IDOSPointSet*>(
                                       item->tree()->project()->object(item->dataId()))
                                 : nullptr;
    if (renderView == nullptr || pointSet == nullptr)
    {
        return false;
    }
    for (IDOSRenderObject* object : renderView->scene()->objects())
    {
        if (object->provider()->dataSource() == pointSet)
        {
            object->setVisible(checked);
            renderView->scene()->notifyChanged(object->id());
            if (checked)
            {
                renderView->resetCamera();
            }
            emit checkStateChanged();
            return true;
        }
    }
    return false;
}

void IDOSPointSetTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
{
    if (view->project() == nullptr)
    {
        return;
    }
    IDOSTreeItem* item = view->menuItem();
    if (!supports(item))
    {
        return;
    }
    QAction* action = menu->addAction(tr("Delete Point Set"));
    m_actions.append(QPointer<QAction>(action));
    action->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    action->setProperty("dataId", item->dataId());
    action->setObjectName(QString::fromLatin1("deletePointSetAction"));
    connect(action, &QAction::triggered, this, &IDOSPointSetTreeGuiProvider::onDeletePointSet);
}

IDOSTreeView* IDOSPointSetTreeGuiProvider::actionView() const
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMenu* menu = action != nullptr ? qobject_cast<QMenu*>(action->parent()) : nullptr;
    IDOSTreeView* view = menu != nullptr ? qobject_cast<IDOSTreeView*>(menu->parentWidget()) : nullptr;
    if (view == nullptr || view->project() == nullptr ||
        action->property("project").value<QObject*>() != view->project())
    {
        return nullptr;
    }
    return view;
}

void IDOSPointSetTreeGuiProvider::onDeletePointSet()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    QJsonObject arguments;
    arguments.insert(QString::fromLatin1("objectId"), action->property("dataId").toString());
    view->project()->commandManager()->execute(QString::fromLatin1("pointset.delete"), arguments);
    if (!view->project()->commandManager()->error().isEmpty())
    {
        QMessageBox::warning(view, tr("Delete Point Set"), view->project()->commandManager()->error());
    }
}

