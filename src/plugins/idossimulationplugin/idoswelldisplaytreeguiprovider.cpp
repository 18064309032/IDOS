#include <QAction>
#include <QMenu>
#include <QVariant>

#include "idosinterface.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "idostree.h"
#include "idostreeitem.h"
#include "idostreeview.h"
#include "idoswell.h"
#include "idoswelldisplaywidget.h"
#include "well/idoswellrenderprovider.h"

#include "idoswelldisplaytreeguiprovider.h"

IDOSWellDisplayTreeGuiProvider::IDOSWellDisplayTreeGuiProvider(IDOSInterface* interface)
    :
    IDOSTreeGuiProvider(nullptr)
    , m_interface(interface)
    , m_widget(nullptr)
    , m_projects()
    , m_views()
{
    connect(interface, &IDOSInterface::projectChanged, this, &IDOSWellDisplayTreeGuiProvider::onProjectChanged);
}

IDOSWellDisplayTreeGuiProvider::~IDOSWellDisplayTreeGuiProvider()
{
    delete m_widget.data();
    const QList<QAction*> actions = m_projects.keys();
    for (QAction* action : actions)
    {
        delete action;
    }
}

QString IDOSWellDisplayTreeGuiProvider::name() const
{
    return QStringLiteral("idos.simulation.wellDisplay");
}

bool IDOSWellDisplayTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    if (item == nullptr || item->isGroup() || item->tree() == nullptr || item->tree()->project() == nullptr)
    {
        return false;
    }
    return (item->partKey().isEmpty() || item->partKey() == QStringLiteral("idos.well.trajectory")) &&
        qobject_cast<IDOSWell*>(item->tree()->project()->object(item->dataId())) != nullptr;
}

bool IDOSWellDisplayTreeGuiProvider::isCheckable(const IDOSTreeItem* item) const
{
    Q_UNUSED(item);
    return false;
}

void IDOSWellDisplayTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
{
    if (view == nullptr || !supports(view->menuItem()))
    {
        return;
    }
    menu->addSeparator();
    QAction* action = menu->addAction(tr("Display Settings..."));
    action->setObjectName(QStringLiteral("wellDisplaySettingsAction"));
    action->setProperty("wellId", view->menuItem()->dataId());
    action->setProperty("trajectory", view->menuItem()->partKey() == QStringLiteral("idos.well.trajectory"));
    m_projects.insert(action, view->project());
    m_views.insert(action, qobject_cast<IDOSRenderView*>(view->targetView()));
    action->setEnabled(renderObject(action) != nullptr);
    connect(action, &QAction::triggered, this, &IDOSWellDisplayTreeGuiProvider::onSettingsTriggered);
    connect(action, &QObject::destroyed, this, &IDOSWellDisplayTreeGuiProvider::onActionDestroyed);
}

IDOSRenderObject* IDOSWellDisplayTreeGuiProvider::renderObject(QAction* action) const
{
    IDOSProject* project = m_projects.value(action).data();
    IDOSRenderView* view = m_views.value(action).data();
    if (action == nullptr || project == nullptr || view == nullptr || m_interface.isNull() || m_interface->project() != project)
    {
        return nullptr;
    }
    IDOSWell* well = qobject_cast<IDOSWell*>(project->object(action->property("wellId").toString()));
    if (well == nullptr)
    {
        return nullptr;
    }
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        if (object->provider()->dataSource() == well && dynamic_cast<IDOSWellRenderProvider*>(object->provider()) != nullptr)
        {
            return object;
        }
    }
    return nullptr;
}

void IDOSWellDisplayTreeGuiProvider::onSettingsTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSRenderObject* object = renderObject(action);
    if (object == nullptr)
    {
        return;
    }
    delete m_widget.data();
    m_widget = new IDOSWellDisplayWidget(m_interface->mainWindow());
    m_widget->setProject(m_projects.value(action).data());
    m_widget->setWell(qobject_cast<IDOSWell*>(object->provider()->dataSource()));
    m_widget->setView(m_views.value(action).data());
    m_widget->setTrajectory(action->property("trajectory").toBool());
    m_widget->show();
}

void IDOSWellDisplayTreeGuiProvider::onActionDestroyed(QObject* object)
{
    QAction* action = static_cast<QAction*>(object);
    m_projects.remove(action);
    m_views.remove(action);
}

void IDOSWellDisplayTreeGuiProvider::onProjectChanged()
{
    delete m_widget.data();
    m_widget = nullptr;
}
