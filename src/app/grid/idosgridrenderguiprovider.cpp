#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include "idoscommandmanager.h"
#include "idosdeletegridcommand.h"
#include "idosgrid.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "idosgridproperty.h"
#include "idosgridrenderprovider.h"
#include "idosrenderserver.h"

#include "idosgridrenderguiprovider.h"

IDOSGridRenderGuiProvider::IDOSGridRenderGuiProvider(IDOSRenderServer* server)
    :
    m_server(server)
{
    if (server != nullptr)
    {
        connect(server, &IDOSRenderServer::objectsChanged, this, &IDOSGridRenderGuiProvider::onObjectsChanged);
        connect(server, &IDOSRenderServer::dataAboutToBeRemoved, this, &IDOSGridRenderGuiProvider::onDataRemoving);
        for (IDOSRenderView* view : server->views())
        {
            onObjectsChanged(view, QStringList(), true);
        }
    }
}

QString IDOSGridRenderGuiProvider::name() const
{
    return QStringLiteral("idos.grid");
}

bool IDOSGridRenderGuiProvider::supports(const IDOSRenderObject* object) const
{
    return object != nullptr && qobject_cast<IDOSGrid*>(object->provider()->dataSource()) != nullptr;
}

void IDOSGridRenderGuiProvider::populateMenu(QMenu* menu, IDOSRenderView* view)
{
    IDOSRenderObject* object = view->menuObject();
    if (!supports(object))
    {
        return;
    }
    QMenu* modes = menu->addMenu(tr("Display Mode"));
    const QStringList titles = {tr("Surface"), tr("Wireframe"), tr("Surface with Edges"), tr("Points")};
    for (int index = 0; index < titles.size(); ++index)
    {
        QAction* action = modes->addAction(titles.at(index));
        action->setData(index);
        action->setProperty("renderId", object->id());
        connect(action, &QAction::triggered, this, &IDOSGridRenderGuiProvider::onDisplayMode);
    }
    if (view->project() != nullptr)
    {
        QAction* remove = menu->addAction(tr("Delete Grid"));
        remove->setProperty("renderId", object->id());
        remove->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
        remove->setProperty("dataId", object->provider()->dataSource()->objectId());
        connect(remove, &QAction::triggered, this, &IDOSGridRenderGuiProvider::onDelete);
    }
}

IDOSRenderView* IDOSGridRenderGuiProvider::actionView() const
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

void IDOSGridRenderGuiProvider::onDelete()
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
    if (qobject_cast<IDOSGrid*>(view->project()->object(id)) == nullptr)
    {
        return;
    }
    IDOSCommandManager* manager = view->project()->commandManager();
    if (!manager->execute(new IDOSDeleteGridCommand(view->project(), id)))
    {
        QMessageBox::warning(view, tr("Delete Grid"), manager->error());
    }
}

void IDOSGridRenderGuiProvider::onDisplayMode()
{
    IDOSRenderView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSRenderObject* object = view != nullptr ? view->scene()->object(action->property("renderId").toString()) : nullptr;
    IDOSGridRenderProvider* provider = object != nullptr ? dynamic_cast<IDOSGridRenderProvider*>(object->provider()) : nullptr;
    if (provider != nullptr && provider->setDisplayMode(static_cast<IDOSDisplayMode>(action->data().toInt())))
    {
        view->scene()->notifyChanged(object->id());
    }
}

void IDOSGridRenderGuiProvider::onObjectsChanged(IDOSRenderView* view, const QStringList& ids, bool dataChanged)
{
    if (m_server == nullptr || m_server->project() == nullptr || view == nullptr)
    {
        return;
    }
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        IDOSGridRenderProvider* provider = dynamic_cast<IDOSGridRenderProvider*>(object->provider());
        if (provider == nullptr || provider->dataSource() == nullptr)
        {
            continue;
        }
        IDOSGridProperty* selectedProperty = provider->property();
        if (dataChanged && selectedProperty != nullptr && ids.contains(selectedProperty->objectId()))
        {
            provider->setProperty(selectedProperty);
            view->scene()->notifyChanged(object->id());
        }
    }
}

void IDOSGridRenderGuiProvider::onDataRemoving(const QString& id)
{
    if (m_server == nullptr)
    {
        return;
    }
    for (IDOSRenderView* view : m_server->views())
    {
        view->scene()->beginUpdate();
        for (IDOSRenderObject* object : view->scene()->objects())
        {
            IDOSGridRenderProvider* provider = dynamic_cast<IDOSGridRenderProvider*>(object->provider());
            if (provider != nullptr && provider->property() != nullptr && provider->property()->objectId() == id)
            {
                provider->setProperty(nullptr);
                view->scene()->notifyChanged(object->id());
            }
        }
        view->scene()->endUpdate();
    }
}
