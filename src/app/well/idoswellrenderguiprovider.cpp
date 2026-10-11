#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include "idoscommandmanager.h"
#include "idosdeletewellcommand.h"
#include "idoswell.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"

#include "idoswellrenderguiprovider.h"

QString IDOSWellRenderGuiProvider::name() const
{
    return QStringLiteral("idos.well");
}

bool IDOSWellRenderGuiProvider::supports(const IDOSRenderObject* object) const
{
    return object != nullptr && qobject_cast<IDOSWell*>(object->provider()->dataSource()) != nullptr;
}

void IDOSWellRenderGuiProvider::populateMenu(QMenu* menu, IDOSRenderView* view)
{
    IDOSRenderObject* object = view->menuObject();
    if (!supports(object))
    {
        return;
    }
    if (view->project() != nullptr)
    {
        QAction* remove = menu->addAction(tr("Delete Well"));
        remove->setProperty("renderId", object->id());
        remove->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
        remove->setProperty("dataId", object->provider()->dataSource()->objectId());
        connect(remove, &QAction::triggered, this, &IDOSWellRenderGuiProvider::onDelete);
    }
}

IDOSRenderView* IDOSWellRenderGuiProvider::actionView() const
{
    QAction* action = qobject_cast<QAction*>(sender());
    for (QObject* parent = action != nullptr ? action->parent() : nullptr; parent != nullptr; parent = parent->parent())
    {
        IDOSRenderView* view = qobject_cast<IDOSRenderView*>(parent);
        if (view != nullptr)
        {
            return view;
        }
    }
    return nullptr;
}

void IDOSWellRenderGuiProvider::onDelete()
{
    IDOSRenderView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || view->project() == nullptr ||
        action->property("project").value<QObject*>() != view->project() ||
        view->scene()->object(action->property("renderId").toString()) == nullptr)
    {
        return;
    }
    const QString id = action->property("dataId").toString();
    if (qobject_cast<IDOSWell*>(view->project()->object(id)) == nullptr)
    {
        return;
    }
    IDOSCommandManager* manager = view->project()->commandManager();
    if (!manager->execute(new IDOSDeleteWellCommand(view->project(), id)))
    {
        QMessageBox::warning(view, tr("Delete Well"), manager->error());
    }
}
