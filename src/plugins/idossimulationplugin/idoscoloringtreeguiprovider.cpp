#include <QAction>
#include <QMenu>
#include <QVariant>

#include "grid/idosgridrenderprovider.h"
#include "idoscoloringwidget.h"
#include "idosdataobject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosinterface.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "idostree.h"
#include "idostreeitem.h"
#include "idostreeview.h"

#include "idoscoloringtreeguiprovider.h"

IDOSColoringTreeGuiProvider::IDOSColoringTreeGuiProvider(IDOSInterface* interface)
    :
    IDOSTreeGuiProvider(nullptr)
    , m_interface(interface)
    , m_widget(nullptr)
    , m_projects()
    , m_views()
{
    connect(interface, &IDOSInterface::projectChanged, this, &IDOSColoringTreeGuiProvider::onProjectChanged);
}

IDOSColoringTreeGuiProvider::~IDOSColoringTreeGuiProvider()
{
    delete m_widget.data();
}

QString IDOSColoringTreeGuiProvider::name() const
{
    return QStringLiteral("idos.simulation.coloring");
}

bool IDOSColoringTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    if (item == nullptr || item->isGroup() || item->tree() == nullptr || item->tree()->project() == nullptr)
    {
        return false;
    }
    IDOSDataObject* data = item->tree()->project()->object(item->dataId());
    return qobject_cast<IDOSGrid*>(data) != nullptr || qobject_cast<IDOSGridProperty*>(data) != nullptr;
}

bool IDOSColoringTreeGuiProvider::isCheckable(const IDOSTreeItem* item) const
{
    Q_UNUSED(item);
    return false;
}

QAction* IDOSColoringTreeGuiProvider::addAction(QMenu* menu, IDOSTreeView* tree, const QString& text,
                                              const QString& gridId, const QString& propertyId)
{
    QAction* action = menu->addAction(text);
    action->setProperty("gridId", gridId);
    action->setProperty("propertyId", propertyId);
    m_projects.insert(action, tree->project());
    m_views.insert(action, qobject_cast<IDOSRenderView*>(tree->targetView()));
    action->setEnabled(renderObject(action) != nullptr);
    connect(action, &QObject::destroyed, this, &IDOSColoringTreeGuiProvider::onActionDestroyed);
    return action;
}

void IDOSColoringTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* tree)
{
    if (tree == nullptr || tree->project() == nullptr || !supports(tree->menuItem()))
    {
        return;
    }
    IDOSDataObject* data = tree->project()->object(tree->menuItem()->dataId());
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
    const QString gridId = property != nullptr ? property->gridId() : data->objectId();
    menu->addSeparator();
    if (property != nullptr)
    {
        QAction* display = addAction(menu, tree, tr("Display in Target View"), gridId, property->objectId());
        connect(display, &QAction::triggered, this, &IDOSColoringTreeGuiProvider::onDisplayTriggered);
    }
    else
    {
        QMenu* colorMenu = menu->addMenu(tr("Color by"));
        QAction* solid = addAction(colorMenu, tree, tr("Solid color"), gridId, QString());
        connect(solid, &QAction::triggered, this, &IDOSColoringTreeGuiProvider::onDisplayTriggered);
        for (IDOSDataObject* object : tree->project()->objects())
        {
            IDOSGridProperty* candidate = qobject_cast<IDOSGridProperty*>(object);
            if (candidate != nullptr && candidate->gridId() == gridId)
            {
                QAction* display = addAction(colorMenu, tree, candidate->name(), gridId, candidate->objectId());
                connect(display, &QAction::triggered, this, &IDOSColoringTreeGuiProvider::onDisplayTriggered);
            }
        }
    }
    QAction* settings = addAction(menu, tree, tr("Coloring Settings..."), gridId,
                                 property != nullptr ? property->objectId() : QString());
    settings->setObjectName(QStringLiteral("simulationColoringSettingsAction"));
    connect(settings, &QAction::triggered, this, &IDOSColoringTreeGuiProvider::onSettingsTriggered);
}

IDOSRenderObject* IDOSColoringTreeGuiProvider::renderObject(QAction* action) const
{
    IDOSProject* project = m_projects.value(action).data();
    IDOSRenderView* view = m_views.value(action).data();
    if (action == nullptr || project == nullptr || view == nullptr || m_interface.isNull() ||
        m_interface->project() != project)
    {
        return nullptr;
    }
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(project->object(action->property("gridId").toString()));
    const QString propertyId = action->property("propertyId").toString();
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(project->object(propertyId));
    if (grid == nullptr || (!propertyId.isEmpty() &&
        (property == nullptr || property->gridId() != grid->objectId())))
    {
        return nullptr;
    }
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        if (object->provider()->dataSource() == grid &&
            dynamic_cast<IDOSGridRenderProvider*>(object->provider()) != nullptr)
        {
            return object;
        }
    }
    return nullptr;
}

void IDOSColoringTreeGuiProvider::onSettingsTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSRenderObject* object = renderObject(action);
    if (object == nullptr)
    {
        return;
    }
    delete m_widget.data();
    m_widget = new IDOSColoringWidget(m_projects.value(action).data(),
        qobject_cast<IDOSGrid*>(object->provider()->dataSource()), m_views.value(action).data(),
        action->property("propertyId").toString(), m_interface->mainWindow());
    m_widget->show();
}

void IDOSColoringTreeGuiProvider::onDisplayTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSRenderObject* object = renderObject(action);
    if (object == nullptr)
    {
        return;
    }
    IDOSGridRenderProvider* target = dynamic_cast<IDOSGridRenderProvider*>(object->provider());
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(
        m_projects.value(action)->object(action->property("propertyId").toString()));
    target->setProperty(property);
    object->setVisible(true);
    m_views.value(action)->scene()->notifyChanged(object->id());
    emit checkStateChanged();
}

void IDOSColoringTreeGuiProvider::onActionDestroyed(QObject* object)
{
    QAction* action = static_cast<QAction*>(object);
    m_projects.remove(action);
    m_views.remove(action);
}

void IDOSColoringTreeGuiProvider::onProjectChanged()
{
    delete m_widget.data();
    m_widget = nullptr;
}
