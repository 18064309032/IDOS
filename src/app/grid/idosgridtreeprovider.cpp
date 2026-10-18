#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "tree/idoscasetreeitemmodel.h"
#include "tree/idosdatatreeitemmodel.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"

#include "idosgridtreeprovider.h"

IDOSGridTreeProvider::IDOSGridTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeItemProvider(data)
{
}

IDOSTreeItem* IDOSGridTreeProvider::addItem(IDOSTreeItem* parent, const QString& key, const QString& title,
                                            IDOSDataObject* data)
{
    IDOSTreeItem* item = parent->findChild(key);
    if (item == nullptr)
    {
        item = new IDOSTreeItem(new IDOSGridTreeProvider(data != nullptr ? data : object()));
        parent->appendChild(item);
    }

    item->provider()->setKey(key);
    item->provider()->setName(title);
    item->provider()->setDataId(parent->provider()->dataId());
    item->provider()->setCaseId(parent->provider()->caseId());
    item->provider()->setPartKey(key);
    return item;
}

void IDOSGridTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(object());
    if (grid == nullptr || item == nullptr || item->provider()->isGroup())
    {
        return;
    }

    item->provider()->setName(grid->name());
    item->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-grid.svg")));
    item->provider()->setCheckable(true);
    IDOSTreeItem* staticGroup =
        addItem(item, QStringLiteral("grid.staticProperties"), QObject::tr("Static Properties"));
    staticGroup->provider()->setGroup(true);
    IDOSTreeItem* regions = addItem(item, QStringLiteral("grid.regions"), QObject::tr("Regions"));
    regions->provider()->setGroup(true);
    IDOSTreeItem* dynamicGroup =
        addItem(item, QStringLiteral("grid.dynamicProperties"), QObject::tr("Dynamic Properties"));
    dynamicGroup->provider()->setGroup(true);
    IDOSTreeItem* filters = addItem(item, QStringLiteral("grid.filters"), QObject::tr("Filters"));
    filters->provider()->setGroup(true);
    const IDOSProject* project = qobject_cast<const IDOSProject*>(grid->parent());
    if (project == nullptr)
    {

        return;
    }
    const QStringList regionKeywords = {QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"), QStringLiteral("PVTNUM"),
                                        QStringLiteral("EQLNUM"), QStringLiteral("FIPNUM")};
    for (IDOSDataObject* candidate : project->objects())
    {
        IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(candidate);
        if (filter != nullptr && filter->gridId() == grid->objectId())
        {
            IDOSTreeItem* leaf =
                addItem(filters, QStringLiteral("filter:") + filter->objectId(), filter->name(), filter);
            leaf->provider()->setDataId(filter->objectId());
            leaf->provider()->setReference(true);
            leaf->provider()->setPartKey(QString());
            leaf->provider()->setCheckable(true);
        }
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(candidate);
        if (property == nullptr || property->gridId() != grid->objectId())
        {
            continue;
        }
        IDOSTreeItem* category = property->kind() == IDOSGridPropertyObject::Kind::Dynamic      ? dynamicGroup
                                 : regionKeywords.contains(property->keyword().toUpper()) ? regions
                                                                                          : staticGroup;
        const QString propertyName = property->name();
        IDOSTreeItem* leaf =
            addItem(category, QStringLiteral("property:") + property->objectId(), propertyName, property);
        leaf->provider()->setDataId(property->objectId());
        leaf->provider()->setReference(true);
        leaf->provider()->setPartKey(QString());
        leaf->provider()->setCheckable(true);
    }
}

bool IDOSGridTreeProvider::dependsOn(const QStringList& dataIds) const
{
    return !dataIds.isEmpty();
}

QMenu* IDOSGridTreeProvider::createContextMenu(const QList<IDOSTreeItem*>& selectedItems)
{
    if (item() == nullptr || item()->model() == nullptr)
    {
        return nullptr;
    }
    IDOSTreeModel* model = item()->model();
    const bool renderAvailable =
        model->data(model->index(dataId()), IDOSTreeModel::RenderAvailableRole).toBool();
    QMenu* menu = new QMenu();
    menu->setProperty("treeSession", item()->model()->sessionId());
    const QList<IDOSTreeItem*> targets = selectedItems.isEmpty() ? QList<IDOSTreeItem*>{item()} : selectedItems;
    if (selectedItems.size() <= 1)
    {
        IDOSTreeItem* item = this->item();
        IDOSDataObject* data = model->object(item->provider()->dataId());
        if (qobject_cast<IDOSGridObject*>(data) == nullptr && qobject_cast<IDOSGridPropertyObject*>(data) == nullptr &&
            qobject_cast<IDOSGridFilterObject*>(data) == nullptr)
        {
            return menu;
        }
        IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(data);
        if (filter != nullptr)
        {
            QAction* edit = new QAction(tr("Edit Filter..."), menu);
            menu->addAction(edit);
            edit->setProperty("gridId", filter->gridId());
            edit->setProperty("propertyId", QString());
            edit->setEnabled(renderAvailable);
            edit->setProperty("filterId", filter->objectId());
            edit->setProperty("openFilter", true);
            edit->setObjectName(QStringLiteral("gridFilterEditAction"));
            edit->setEnabled(renderAvailable);
            connect(edit, &QAction::triggered, this, &IDOSGridTreeProvider::onGridSettingsTriggered);

            QAction* settings = new QAction(tr("Display Settings..."), menu);
            menu->addAction(settings);
            settings->setProperty("gridId", filter->gridId());
            settings->setProperty("propertyId", QString());
            settings->setEnabled(renderAvailable);
            settings->setProperty("filterId", filter->objectId());
            settings->setObjectName(QStringLiteral("gridSettingsAction"));
            settings->setEnabled(renderAvailable);
            connect(settings, &QAction::triggered, this, &IDOSGridTreeProvider::onGridSettingsTriggered);

            QAction* rename = new QAction(tr("Rename Filter..."), menu);
            menu->addAction(rename);
            rename->setObjectName(QStringLiteral("gridFilterRenameAction"));
            connect(rename, &QAction::triggered, this, &IDOSGridTreeProvider::onGridRenameFilter);

            QAction* remove = new QAction(tr("Delete Filter..."), menu);
            menu->addAction(remove);
            remove->setObjectName(QStringLiteral("gridFilterDeleteAction"));
            connect(remove, &QAction::triggered, this, &IDOSGridTreeProvider::onGridDeleteFilter);

            return menu;
        }
        if (item->provider()->isGroup() && item->provider()->key() == QStringLiteral("grid.filters"))
        {
            QAction* action = new QAction(tr("New Filter..."), menu);
            menu->addAction(action);
            action->setProperty("gridId", item->provider()->dataId());
            action->setProperty("propertyId", QString());
            action->setEnabled(renderAvailable);
            action->setObjectName(QStringLiteral("gridFilterAction"));
            action->setProperty("openFilter", true);
            connect(action, &QAction::triggered, this, &IDOSGridTreeProvider::onGridSettingsTriggered);

            return menu;
        }
        if (item->provider()->isGroup())
        {
            if (item->provider()->key() == QStringLiteral("grid.dynamicProperties"))
            {
                QAction* import = new QAction(tr("Import Dynamic Properties..."), menu);
                menu->addAction(import);
                import->setObjectName(QStringLiteral("importDynamicPropertiesAction"));
                connect(import, &QAction::triggered, this, &IDOSGridTreeProvider::onGridImportDynamicProperties);
            }
            else if (item->provider()->key() == QStringLiteral("grid.staticProperties"))
            {
                QAction* import = new QAction(tr("Import Property..."), menu);
                menu->addAction(import);
                import->setObjectName(QStringLiteral("importPropertyAction"));
                connect(import, &QAction::triggered, this, &IDOSGridTreeProvider::onGridImportProperty);
            }
            else if (item->provider()->key() == QStringLiteral("grid.regions"))
            {
                QAction* import = new QAction(tr("Import Region Properties..."), menu);
                menu->addAction(import);
                import->setObjectName(QStringLiteral("importRegionPropertiesAction"));
                connect(import, &QAction::triggered, this, &IDOSGridTreeProvider::onGridImportProperty);
            }
        }
        else if (qobject_cast<IDOSGridPropertyObject*>(data) != nullptr)
        {
            QAction* remove = new QAction(tr("Delete Property..."), menu);
            menu->addAction(remove);
            remove->setObjectName(QStringLiteral("deletePropertyAction"));
            connect(remove, &QAction::triggered, this, &IDOSGridTreeProvider::onGridDeleteProperty);
        }
        else if (qobject_cast<IDOSGridObject*>(data) != nullptr)
        {
            QAction* filterAction = new QAction(tr("New Filter..."), menu);
            menu->addAction(filterAction);
            filterAction->setProperty("gridId", item->provider()->dataId());
            filterAction->setProperty("propertyId", QString());
            filterAction->setEnabled(renderAvailable);
            filterAction->setObjectName(QStringLiteral("gridFilterAction"));
            filterAction->setProperty("openFilter", true);
            connect(filterAction, &QAction::triggered, this, &IDOSGridTreeProvider::onGridSettingsTriggered);

            QAction* remove = new QAction(tr("Delete Grid..."), menu);
            menu->addAction(remove);
            remove->setObjectName(QStringLiteral("deleteGridAction"));
            connect(remove, &QAction::triggered, this, &IDOSGridTreeProvider::onGridDelete);
        }
        if (!item->provider()->isGroup() && data != nullptr)
        {
            IDOSDataObject* data = model->object(this->item()->provider()->dataId());
            IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(data);
            const QString gridId = property != nullptr ? property->gridId() : data->objectId();
            menu->addSeparator();
            if (property != nullptr)
            {
                QAction* display = new QAction(tr("Display in Target View"), menu);
                menu->addAction(display);
                display->setProperty("gridId", gridId);
                display->setProperty("propertyId", property->objectId());
                display->setEnabled(renderAvailable);
                display->setObjectName(QStringLiteral("gridDisplayPropertyAction"));
                connect(display, &QAction::triggered, this, &IDOSGridTreeProvider::onGridDisplayTriggered);
            }
            else
            {
                QMenu* colorMenu = menu->addMenu(tr("Color by"));
                colorMenu->setProperty("treeSession", menu->property("treeSession"));
                QAction* solid = new QAction(tr("Solid color"), colorMenu);
                colorMenu->addAction(solid);
                solid->setProperty("gridId", gridId);
                solid->setProperty("propertyId", QString());
                solid->setEnabled(renderAvailable);
                connect(solid, &QAction::triggered, this, &IDOSGridTreeProvider::onGridDisplayTriggered);

                for (IDOSDataObject* object : model->objects())
                {
                    IDOSGridPropertyObject* candidate = qobject_cast<IDOSGridPropertyObject*>(object);
                    if (candidate != nullptr && candidate->gridId() == gridId)
                    {
                        QAction* display = new QAction(candidate->name(), colorMenu);
                        colorMenu->addAction(display);
                        display->setProperty("gridId", gridId);
                        display->setProperty("propertyId", candidate->objectId());
                        display->setEnabled(renderAvailable);
                        connect(display, &QAction::triggered, this, &IDOSGridTreeProvider::onGridDisplayTriggered);
                    }
                }
            }
            QAction* settings = new QAction(tr("Display Settings..."), menu);
            menu->addAction(settings);
            settings->setProperty("gridId", gridId);
            settings->setProperty("propertyId", property != nullptr ? property->objectId() : QString());
            settings->setEnabled(renderAvailable);
            settings->setObjectName(QStringLiteral("gridSettingsAction"));
            connect(settings, &QAction::triggered, this, &IDOSGridTreeProvider::onGridSettingsTriggered);
        }
    }
    {
        IDOSDataObject* object = item()->model()->object(dataId());
        if (targets.size() == 1 && !isGroup() && partKey().isEmpty() && itemKey().isEmpty() && object != nullptr &&
            object->canClone() && qobject_cast<IDOSGridFilterObject*>(object) == nullptr)
        {
            QAction* rename = new QAction(tr("Rename..."), menu);
            menu->addAction(rename);
            rename->setObjectName(QStringLiteral("renameObjectAction"));
            connect(rename, &QAction::triggered, this, &IDOSGridTreeProvider::onRenameObject);
        }
    }
    return menu;
}

void IDOSGridTreeProvider::onRenameObject()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->rename(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->rename(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSGridTreeProvider::onGridSettingsTriggered()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->showGridSettings(id(), action->parent()->property("treeSession").toString(),
                                action->property("gridId").toString(), action->property("filterId").toString(),
                                action->property("propertyId").toString(), action->property("openFilter").toBool());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->showGridSettings(id(), action->parent()->property("treeSession").toString(),
                                action->property("gridId").toString(), action->property("filterId").toString(),
                                action->property("propertyId").toString(), action->property("openFilter").toBool());
    }
}

void IDOSGridTreeProvider::onGridRenameFilter()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->renameGridFilter(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->renameGridFilter(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSGridTreeProvider::onGridDeleteFilter()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->deleteGridFilter(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->deleteGridFilter(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSGridTreeProvider::onGridImportDynamicProperties()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importGridDynamicProperties(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importGridDynamicProperties(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSGridTreeProvider::onGridImportProperty()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importGridProperty(id(), action->parent()->property("treeSession").toString(), dataId(), caseId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importGridProperty(id(), action->parent()->property("treeSession").toString(), dataId(), caseId());
    }
}

void IDOSGridTreeProvider::onGridDeleteProperty()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->deleteGridProperty(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->deleteGridProperty(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSGridTreeProvider::onGridDelete()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->deleteGrid(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->deleteGrid(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSGridTreeProvider::onGridDisplayTriggered()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->displayGridProperty(id(), action->parent()->property("treeSession").toString(),
                                   action->property("gridId").toString(), action->property("propertyId").toString());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->displayGridProperty(id(), action->parent()->property("treeSession").toString(),
                                   action->property("gridId").toString(), action->property("propertyId").toString());
    }
}
