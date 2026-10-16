#include <QAction>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMenu>
#include <QMimeData>
#include <QSet>
#include <QStringList>
#include <QVariant>

#include "data/case/idoscaseobject.h"
#include "data/fluidrock/idosfluidrocktable.h"
#include "data/fracture/idosfracturematerialcatalog.h"
#include "data/fracture/idosfractureschedule.h"
#include "data/fracture/idosnaturalfractureset.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridboundary.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"
#include "data/grid/idosgridsurface.h"
#include "data/production/idosproductionhistory.h"
#include "data/seismic/idosmicroseismicset.h"
#include "data/seismic/idosseismicvolume.h"
#include "data/well/idoswell.h"
#include "idosdataobject.h"
#include "providers/idostreeitemprovider.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"
#include "tree/idostreeview.h"

#include "idosdatatreemenuprovider.h"

IDOSDataTreeMenuProvider::IDOSDataTreeMenuProvider(IDOSTreeView* view)
    :
    IDOSTreeMenuProvider(view)
{
}

IDOSDataTreeMenuProvider::~IDOSDataTreeMenuProvider()
{
}

QMenu* IDOSDataTreeMenuProvider::createContextMenu(
    const QList<IDOSTreeItem*>& selectedItems, IDOSTreeItem* contextItem)
{
    IDOSTreeView* view = this->view();
    QMenu* menu = nullptr;
    if (contextItem != nullptr && selectedItems.size() <= 1 && contextItem->provider() != nullptr)
    {
        menu = contextItem->provider()->createContextMenu();
    }
    if (menu == nullptr)
    {
        menu = new QMenu();
    }
    if (view == nullptr || view->treeModel() == nullptr || view->treeModel()->name() != QStringLiteral("data"))
    {

        return menu;
    }
    if (selectedItems.size() <= 1)
    {
        addGridActions(menu, view);
        addWellActions(menu, view);
        addWellDisplayActions(menu, view);
    }
    addDeleteActions(menu, view, selectedItems);
    addTransferActions(menu);

    return menu;
}

void IDOSDataTreeMenuProvider::addDeleteActions(
    QMenu* menu, IDOSTreeView* view, const QList<IDOSTreeItem*>& selectedItems)
{
    if (menu == nullptr || view == nullptr || view->treeModel() == nullptr || view->treeModel()->sessionId().isEmpty() || selectedItems.isEmpty())
    {
        return;
    }
    QStringList ids;
    for (IDOSTreeItem* item : selectedItems)
    {
        if (item == nullptr)
        {
            return;
        }
        IDOSDataObject* object = view->treeModel()->dataObject(item->provider()->dataId());
        if (!supportsDelete(object))
        {
            return;
        }
        if (!ids.contains(object->objectId()))
        {
            ids.append(object->objectId());
        }
    }
    if (ids.isEmpty())
    {
        return;
    }
    QAction* action = menu->addAction(tr("Delete Data..."));
    action->setObjectName(QStringLiteral("deleteDataAction"));
    action->setProperty("objectIds", ids);
    action->setProperty("operation", QStringLiteral("data.onDeleteDatasets"));

}

void IDOSDataTreeMenuProvider::addTransferActions(QMenu* menu)
{
    IDOSTreeView* view = this->view();
    if (view->contextItem() == nullptr)
    {
        return;
    }
    QMimeData* data = createMimeData(view->selectedItems());
    if (data == nullptr)
    {
        return;
    }
    const QJsonArray items = readTransfer(data).value(QStringLiteral("items")).toArray();
    delete data;
    bool canAttach = view->treeModel()->name() == QStringLiteral("data");
    for (const QJsonValue& value : items)
    {
        const QString role = value.toObject().value(QStringLiteral("role")).toString();
        if (role != QStringLiteral("case.well") && role != QStringLiteral("case.fluidRockTable"))
        {
            canAttach = false;
        }
    }
    if (canAttach)
    {
        QAction* attach = addTransferAction(menu, tr("Add to Case..."));
        attach->setObjectName(QStringLiteral("addToCaseAction"));
        attach->setProperty("items", items);
        attach->setProperty("operation", QStringLiteral("data.onAddToCase"));

    }
    if (items.size() == 1)
    {
        IDOSDataObject* object = view->treeModel()->dataObject(items.first().toObject().value(QStringLiteral("objectId")).toString());
        // Case and filter menus already provide rename actions.
        if (qobject_cast<IDOSCaseObject*>(object) == nullptr && qobject_cast<IDOSGridFilter*>(object) == nullptr)
        {
            QAction* rename = addTransferAction(menu, tr("Rename..."));
            rename->setObjectName(QStringLiteral("renameObjectAction"));
            rename->setProperty("operation", QStringLiteral("data.onRenameObject"));

        }
    }
}

QAction* IDOSDataTreeMenuProvider::addTransferAction(QMenu* menu, const QString& text)
{
    IDOSTreeView* view = this->view();
    QAction* action = menu->addAction(text);
    IDOSTreeItem* item = view->contextItem();
    action->setProperty("dataId", item != nullptr ? item->provider()->dataId() : QString());
    action->setProperty("caseId", item != nullptr ? item->provider()->caseId() : QString());
    return action;
}

QAction* IDOSDataTreeMenuProvider::addGridAction(QMenu* menu, IDOSTreeView* view, const QString& text)
{
    QAction* action = menu->addAction(text);
    IDOSTreeItem* item = view->contextItem();
    action->setProperty("dataId", item != nullptr ? item->provider()->dataId() : QString());
    action->setProperty("caseId", item != nullptr ? item->provider()->caseId() : QString());
    return action;
}

void IDOSDataTreeMenuProvider::addGridActions(QMenu* menu, IDOSTreeView* view)
{
    if (view == nullptr || view->treeModel() == nullptr || view->treeModel()->sessionId().isEmpty())
    {
        return;
    }
    IDOSTreeItem* item = view->contextItem();

    if (item == nullptr)
    {
        return;
    }
    IDOSDataObject* data = view->treeModel()->dataObject(item->provider()->dataId());
    if (qobject_cast<IDOSGrid*>(data) == nullptr && qobject_cast<IDOSGridProperty*>(data) == nullptr &&
        qobject_cast<IDOSGridFilter*>(data) == nullptr)
    {
        return;
    }
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(data);
    if (filter != nullptr)
    {
        QAction* edit = addGridDisplayAction(menu, view, tr("Edit Filter..."), filter->gridId(), QString());
        edit->setProperty("filterId", filter->objectId());
        edit->setProperty("openFilter", true);
        edit->setObjectName(QStringLiteral("gridFilterEditAction"));
        edit->setEnabled(view->contextItem() != nullptr && view->treeModel()->data(
        view->treeModel()->indexFromDataId(view->contextItem()->provider()->dataId()), IDOSTreeModel::RenderAvailableRole).toBool());
        edit->setProperty("operation", QStringLiteral("data.onGridSettingsTriggered"));

        QAction* settings = addGridDisplayAction(menu, view, tr("Display Settings..."), filter->gridId(), QString());
        settings->setProperty("filterId", filter->objectId());
        settings->setObjectName(QStringLiteral("gridSettingsAction"));
        settings->setEnabled(view->contextItem() != nullptr && view->treeModel()->data(
        view->treeModel()->indexFromDataId(view->contextItem()->provider()->dataId()), IDOSTreeModel::RenderAvailableRole).toBool());
        settings->setProperty("operation", QStringLiteral("data.onGridSettingsTriggered"));

        QAction* rename = addGridAction(menu, view, tr("Rename Filter..."));
        rename->setObjectName(QStringLiteral("gridFilterRenameAction"));
        rename->setProperty("operation", QStringLiteral("data.onGridRenameFilter"));

        QAction* remove = addGridAction(menu, view, tr("Delete Filter..."));
        remove->setObjectName(QStringLiteral("gridFilterDeleteAction"));
        remove->setProperty("operation", QStringLiteral("data.onGridDeleteFilter"));

        return;
    }
    if (item->provider()->isGroup() && item->provider()->key() == QStringLiteral("grid.filters"))
    {
        QAction* action = addGridDisplayAction(menu, view, tr("New Filter..."), item->provider()->dataId(), QString());
        action->setObjectName(QStringLiteral("gridFilterAction"));
        action->setProperty("openFilter", true);
        action->setProperty("operation", QStringLiteral("data.onGridSettingsTriggered"));

        return;
    }
    if (item->provider()->isGroup())
    {
        if (item->provider()->key() == QStringLiteral("grid.dynamicProperties"))
        {
            QAction* import = addGridAction(menu, view, tr("Import Dynamic Properties..."));
            import->setObjectName(QStringLiteral("importDynamicPropertiesAction"));
            import->setProperty("operation", QStringLiteral("data.onGridImportDynamicProperties"));

        }
        else if (item->provider()->key() == QStringLiteral("grid.staticProperties"))
        {
            QAction* import = addGridAction(menu, view, tr("Import Property..."));
            import->setObjectName(QStringLiteral("importPropertyAction"));
            import->setProperty("operation", QStringLiteral("data.onGridImportProperty"));

        }
        else if (item->provider()->key() == QStringLiteral("grid.regions"))
        {
            QAction* import = addGridAction(menu, view, tr("Import Region Properties..."));
            import->setObjectName(QStringLiteral("importRegionPropertiesAction"));
            import->setProperty("operation", QStringLiteral("data.onGridImportProperty"));

        }
    }
    else if (qobject_cast<IDOSGridProperty*>(data) != nullptr)
    {
        QAction* remove = addGridAction(menu, view, tr("Delete Property..."));
        remove->setObjectName(QStringLiteral("deletePropertyAction"));
        remove->setProperty("operation", QStringLiteral("data.onGridDeleteProperty"));

    }
    else if (qobject_cast<IDOSGrid*>(data) != nullptr)
    {
        QAction* filterAction = addGridDisplayAction(menu, view, tr("New Filter..."), item->provider()->dataId(), QString());
        filterAction->setObjectName(QStringLiteral("gridFilterAction"));
        filterAction->setProperty("openFilter", true);
        filterAction->setProperty("operation", QStringLiteral("data.onGridSettingsTriggered"));

        QAction* remove = addGridAction(menu, view, tr("Delete Grid..."));
        remove->setObjectName(QStringLiteral("deleteGridAction"));
        remove->setProperty("operation", QStringLiteral("data.onGridDelete"));

    }
    if (!item->provider()->isGroup() && data != nullptr)
    {
        addGridDisplayActions(menu, view);
    }
}

void IDOSDataTreeMenuProvider::addGridDisplayActions(QMenu* menu, IDOSTreeView* view)
{
    IDOSDataObject* data = view->treeModel()->dataObject(view->contextItem()->provider()->dataId());
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
    const QString gridId = property != nullptr ? property->gridId() : data->objectId();
    menu->addSeparator();
    if (property != nullptr)
    {
        QAction* display = addGridDisplayAction(menu, view, tr("Display in Target View"), gridId, property->objectId());
        display->setObjectName(QStringLiteral("gridDisplayPropertyAction"));
        display->setProperty("operation", QStringLiteral("data.onGridDisplayTriggered"));

    }
    else
    {
        QMenu* colorMenu = menu->addMenu(tr("Color by"));
        QAction* solid = addGridDisplayAction(colorMenu, view, tr("Solid color"), gridId, QString());
        solid->setProperty("operation", QStringLiteral("data.onGridDisplayTriggered"));

        for (IDOSDataObject* object : view->treeModel()->dataObjects())
        {
            IDOSGridProperty* candidate = qobject_cast<IDOSGridProperty*>(object);
            if (candidate != nullptr && candidate->gridId() == gridId)
            {
                QAction* display = addGridDisplayAction(colorMenu, view, candidate->name(), gridId, candidate->objectId());
                display->setProperty("operation", QStringLiteral("data.onGridDisplayTriggered"));

            }
        }
    }
    QAction* settings = addGridDisplayAction(menu, view, tr("Display Settings..."), gridId,
        property != nullptr ? property->objectId() : QString());
    settings->setObjectName(QStringLiteral("gridSettingsAction"));
    settings->setProperty("operation", QStringLiteral("data.onGridSettingsTriggered"));

}

QAction* IDOSDataTreeMenuProvider::addGridDisplayAction(QMenu* menu, IDOSTreeView* view, const QString& text,
                                                 const QString& gridId, const QString& propertyId)
{
    QAction* action = menu->addAction(text);
    IDOSTreeItem* context = view->contextItem();
    action->setProperty("dataId", context != nullptr ? context->provider()->dataId() : QString());
    action->setProperty("caseId", context != nullptr ? context->provider()->caseId() : QString());
    action->setProperty("gridId", gridId);
    action->setProperty("propertyId", propertyId);
    action->setEnabled(view->contextItem() != nullptr && view->treeModel()->data(
        view->treeModel()->indexFromDataId(view->contextItem()->provider()->dataId()), IDOSTreeModel::RenderAvailableRole).toBool());
    return action;
}

void IDOSDataTreeMenuProvider::addWellActions(QMenu* menu, IDOSTreeView* view)
{
    if (view->treeModel() == nullptr || view->treeModel()->sessionId().isEmpty())
    {
        return;
    }
    IDOSTreeItem* item = view->contextItem();
    if (item == nullptr)
    {
        return;
    }
    if (item->provider()->key() == QStringLiteral("data.wells"))
    {
        if (view->treeModel() == nullptr || view->treeModel()->name() != QStringLiteral("data"))
        {
            return;
        }
        QAction* create = addGridAction(menu, view, tr("New Well..."));
        create->setObjectName(QStringLiteral("newWellAction"));
        create->setProperty("operation", QStringLiteral("data.onNewWell"));

        QAction* import = addGridAction(menu, view, tr("Import Well Data..."));
        import->setObjectName(QStringLiteral("importWellAction"));
        import->setProperty("operation", QStringLiteral("data.onImportWell"));

        return;
    }
    IDOSWell* well = qobject_cast<IDOSWell*>(view->treeModel()->dataObject(item->provider()->dataId()));
    if (well == nullptr)
    {
        return;
    }
    const QString part = item->provider()->partKey();
    const bool isWellItem = item->parent() != nullptr &&
                            item->parent()->provider()->key() == QStringLiteral("data.wells") &&
                            item->provider()->key().startsWith(QStringLiteral("object:"));
    const bool isLeafItem = item->parent() != nullptr &&
                            item->parent()->provider()->key().startsWith(QStringLiteral("idos.well."));
    if (isWellItem)
    {
        QAction* importHeader = addGridAction(menu, view, tr("Import Well Header..."));
        importHeader->setObjectName(QStringLiteral("importWellHeaderAction"));
        importHeader->setProperty("operation", QStringLiteral("data.onImportWell"));

        QAction* importWell = addGridAction(menu, view, tr("Import Well Data..."));
        importWell->setObjectName(QStringLiteral("importWellDataSingleAction"));
        importWell->setProperty("operation", QStringLiteral("data.onImportWell"));

        QAction* show = addGridAction(menu, view, tr("View Well Data..."));
        show->setObjectName(QStringLiteral("viewWellSummaryAction"));
        show->setProperty("part", QStringLiteral("idos.well.summary"));
        show->setProperty("operation", QStringLiteral("data.onViewWellData"));

    }
    else if (!isLeafItem && part == QStringLiteral("idos.well.trajectory"))
    {
        QAction* show = addGridAction(menu, view, tr("View Trajectory Data..."));
        show->setObjectName(QStringLiteral("viewWellPathAction"));
        show->setProperty("part", part);
        show->setProperty("operation", QStringLiteral("data.onViewWellData"));

        QAction* import = addGridAction(menu, view, tr("Import or Replace Trajectory..."));
        import->setObjectName(QStringLiteral("importWellPathAction"));
        import->setProperty("operation", QStringLiteral("data.onImportWell"));

        return;
    }
    else if (part == QStringLiteral("idos.well.logs"))
    {
        if (!isLeafItem)
        {
            QAction* import = addGridAction(menu, view, tr("Import Well Curves..."));
            import->setObjectName(QStringLiteral("importWellCurvesAction"));
            import->setProperty("part", part);
            import->setProperty("operation", QStringLiteral("data.onImportWell"));

        }
        QAction* show = addGridAction(menu, view, tr("View Curve Data..."));
        show->setObjectName(QStringLiteral("viewWellInputDataAction"));
        show->setProperty("part", part);
        show->setProperty("itemKey", item->provider()->itemKey());
        show->setProperty("operation", QStringLiteral("data.onViewWellData"));

    }
    else if (!isLeafItem && (part == QStringLiteral("idos.well.zones") || part == QStringLiteral("idos.well.completions") ||
        part == QStringLiteral("idos.well.checkshots")))
    {
        const QString label = part == QStringLiteral("idos.well.zones") ? tr("Markers")
            : part == QStringLiteral("idos.well.completions") ? tr("Completions") : tr("Checkshots");
        QAction* import = addGridAction(menu, view, tr("Import %1 Data...").arg(label));
        import->setObjectName(QStringLiteral("importWellPartAction"));
        import->setProperty("part", part);
        import->setProperty("operation", QStringLiteral("data.onImportWell"));

        QAction* show = addGridAction(menu, view, tr("View %1 Data...").arg(label));
        show->setObjectName(QStringLiteral("viewWellInputDataAction"));
        show->setProperty("part", part);
        show->setProperty("operation", QStringLiteral("data.onViewWellData"));

    }
    else if (isWellItem && part == QStringLiteral("idos.well.trajectory"))
    {
        QAction* importHeader = addGridAction(menu, view, tr("Import Well Header..."));
        importHeader->setObjectName(QStringLiteral("importWellHeaderAction"));
        importHeader->setProperty("operation", QStringLiteral("data.onImportWell"));

    }
    else if (isLeafItem && part == QStringLiteral("idos.well.zones"))
    {
        QAction* show = addGridAction(menu, view, tr("View Marker Data..."));
        show->setObjectName(QStringLiteral("viewWellInputDataAction"));
        show->setProperty("part", part);
        show->setProperty("itemKey", item->provider()->itemKey());
        show->setProperty("operation", QStringLiteral("data.onViewWellData"));

    }
    if (isWellItem)
    {
        QAction* remove = addGridAction(menu, view, tr("Delete Well..."));
        remove->setObjectName(QStringLiteral("deleteWellAction"));
        remove->setProperty("operation", QStringLiteral("data.onDeleteWell"));

    }
}

bool IDOSDataTreeMenuProvider::supportsWellDisplayItem(const IDOSTreeItem* item) const
{
    if (item == nullptr || item->provider()->isGroup() || view() == nullptr || view()->treeModel() == nullptr)
    {
        return false;
    }
    return (item->provider()->partKey().isEmpty() || item->provider()->partKey() == QStringLiteral("idos.well.trajectory")) &&
        qobject_cast<IDOSWell*>(view()->treeModel()->dataObject(item->provider()->dataId())) != nullptr;
}

void IDOSDataTreeMenuProvider::addWellDisplayActions(QMenu* menu, IDOSTreeView* view)
{
    if (view == nullptr || !supportsWellDisplayItem(view->contextItem()))
    {
        return;
    }
    menu->addSeparator();
    QAction* action = menu->addAction(tr("Display Settings..."));
    action->setObjectName(QStringLiteral("wellDisplaySettingsAction"));
    action->setProperty("wellId", view->contextItem()->provider()->dataId());
    action->setProperty("trajectory", view->contextItem()->provider()->partKey() == QStringLiteral("idos.well.trajectory"));
    action->setEnabled(view->contextItem() != nullptr && view->treeModel()->data(
        view->treeModel()->indexFromDataId(view->contextItem()->provider()->dataId()), IDOSTreeModel::RenderAvailableRole).toBool());
    action->setProperty("operation", QStringLiteral("data.onWellSettingsTriggered"));

}



bool IDOSDataTreeMenuProvider::supportsDelete(const IDOSDataObject* data) const
{
    return qobject_cast<const IDOSNaturalFractureSet*>(data) != nullptr
        || qobject_cast<const IDOSMicroseismicSet*>(data) != nullptr
        || qobject_cast<const IDOSSeismicVolume*>(data) != nullptr
        || qobject_cast<const IDOSFractureSchedule*>(data) != nullptr
        || qobject_cast<const IDOSFractureMaterialCatalog*>(data) != nullptr
        || qobject_cast<const IDOSProductionHistory*>(data) != nullptr
        || qobject_cast<const IDOSGridSurface*>(data) != nullptr
        || qobject_cast<const IDOSGridBoundary*>(data) != nullptr
        || qobject_cast<const IDOSFluidRockTable*>(data) != nullptr;
}

QMimeData* IDOSDataTreeMenuProvider::createMimeData(const QList<IDOSTreeItem*>& items) const
{
    IDOSTreeView* treeView = view();
    IDOSTreeModel* model = treeView != nullptr ? treeView->treeModel() : nullptr;
    if (model == nullptr || model->sessionId().isEmpty() || items.isEmpty())
    {
        return nullptr;
    }
    QJsonArray records;
    QSet<QString> ids;
    for (IDOSTreeItem* item : items)
    {
        if (item == nullptr || item->model() != model || item->provider()->isGroup() ||
            !item->provider()->partKey().isEmpty() || !item->provider()->itemKey().isEmpty())
        {
            return nullptr;
        }
        IDOSDataObject* data = model->dataObject(item->provider()->dataId());
        if (data == nullptr || !data->canClone() || ids.contains(data->objectId()))
        {
            return nullptr;
        }
        ids.insert(data->objectId());
        QString role;
        QString type;
        QString group = item->parent() != nullptr ? item->parent()->provider()->key() : QString();
        if (qobject_cast<IDOSCaseObject*>(data) != nullptr)
        {
            type = QStringLiteral("case");
        }
        else if (qobject_cast<IDOSWell*>(data) != nullptr)
        {
            role = QStringLiteral("case.well");
            type = QStringLiteral("well");
            group = QStringLiteral("data.wells");
        }
        else if (qobject_cast<IDOSGrid*>(data) != nullptr)
        {
            role = QStringLiteral("case.grid");
            type = QStringLiteral("grid");
        }
        else if (qobject_cast<IDOSGridProperty*>(data) != nullptr)
        {
            type = QStringLiteral("property");
        }
        else if (qobject_cast<IDOSGridFilter*>(data) != nullptr)
        {
            type = QStringLiteral("filter");
        }
        else if (qobject_cast<IDOSFluidRockTable*>(data) != nullptr)
        {
            role = QStringLiteral("case.fluidRockTable");
            type = QStringLiteral("fluidRock");
            group = qobject_cast<IDOSFluidRockTable*>(data)->category() == IDOSFluidRockTable::Category::Fluid
                ? QStringLiteral("data.fluids") : QStringLiteral("data.rockPhysics");
        }
        QJsonObject record;
        record.insert(QStringLiteral("objectId"), data->objectId());
        record.insert(QStringLiteral("role"), role);
        record.insert(QStringLiteral("group"), group);
        record.insert(QStringLiteral("type"), type);
        record.insert(QStringLiteral("containerId"), data->containerId());
        record.insert(QStringLiteral("sourceCaseId"), item->provider()->caseId());
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
        if (property != nullptr)
        {
            record.insert(QStringLiteral("dynamic"), property->kind() == IDOSGridProperty::Kind::Dynamic);
        }
        records.append(record);
    }
    QJsonObject payload;
    payload.insert(QStringLiteral("version"), 1);
    payload.insert(QStringLiteral("projectSession"), model->sessionId());
    payload.insert(QStringLiteral("items"), records);
    QMimeData* mime = new QMimeData();
    mime->setData(QStringLiteral("application/x-idos-objects"), QJsonDocument(payload).toJson(QJsonDocument::Compact));
    return mime;
}

QJsonObject IDOSDataTreeMenuProvider::readTransfer(const QMimeData* data) const
{
    IDOSTreeView* treeView = view();
    IDOSTreeModel* model = treeView != nullptr ? treeView->treeModel() : nullptr;
    if (data == nullptr || model == nullptr || model->sessionId().isEmpty())
    {
        return QJsonObject();
    }
    const QJsonObject payload = QJsonDocument::fromJson(
        data->data(QStringLiteral("application/x-idos-objects"))).object();
    if (payload.value(QStringLiteral("version")).toInt() != 1 ||
        payload.value(QStringLiteral("projectSession")).toString() != model->sessionId() ||
        payload.value(QStringLiteral("items")).toArray().isEmpty())
    {
        return QJsonObject();
    }
    return payload;
}

QJsonObject IDOSDataTreeMenuProvider::transferArguments(const QMimeData* data, const IDOSTreeItem* target) const
{
    IDOSTreeView* treeView = view();
    IDOSTreeModel* model = treeView != nullptr ? treeView->treeModel() : nullptr;
    const QJsonObject payload = readTransfer(data);
    if (payload.isEmpty() || model == nullptr || (target != nullptr && target->model() != model))
    {
        return QJsonObject();
    }
    QString caseId;
    QString gridId;
    const QJsonArray records = payload.value(QStringLiteral("items")).toArray();
    for (const QJsonValue& value : records)
    {
        const QJsonObject record = value.toObject();
        const QString type = record.value(QStringLiteral("type")).toString();
        if (record.value(QStringLiteral("objectId")).toString().isEmpty())
        {
            return QJsonObject();
        }
        if (type == QStringLiteral("case"))
        {
            if (model->name() != QStringLiteral("case") ||
                (target != nullptr && target->parent() != model->root()))
            {
                return QJsonObject();
            }
        }
        else if (type == QStringLiteral("property") || type == QStringLiteral("filter"))
        {
            if (target == nullptr)
            {
                return QJsonObject();
            }
            IDOSDataObject* destination = model->dataObject(target->provider()->dataId());
            IDOSGrid* grid = qobject_cast<IDOSGrid*>(destination);
            IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(destination);
            IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(destination);
            gridId = grid != nullptr ? grid->objectId() : property != nullptr ? property->gridId()
                : filter != nullptr ? filter->gridId() : QString();
            const QString key = target->provider()->key();
            if (gridId.isEmpty() || (target->provider()->isGroup() &&
                key != QStringLiteral("grid.staticProperties") && key != QStringLiteral("grid.regions") &&
                key != QStringLiteral("grid.dynamicProperties") && key != QStringLiteral("grid.filters")))
            {
                return QJsonObject();
            }
            if (type == QStringLiteral("property") && target->provider()->isGroup() &&
                ((key == QStringLiteral("grid.dynamicProperties") && !record.value(QStringLiteral("dynamic")).toBool()) ||
                 (key == QStringLiteral("grid.staticProperties") && record.value(QStringLiteral("dynamic")).toBool()) ||
                 key == QStringLiteral("grid.filters")))
            {
                return QJsonObject();
            }
        }
        else if (model->name() == QStringLiteral("data"))
        {
            const IDOSTreeItem* group = target;
            while (group != nullptr && group->provider()->key() != record.value(QStringLiteral("group")).toString())
            {
                group = group->parent();
            }
            if (target != nullptr && group == nullptr)
            {
                return QJsonObject();
            }
        }
        else
        {
            if (target == nullptr || record.value(QStringLiteral("role")).toString().isEmpty())
            {
                return QJsonObject();
            }
            const QString role = record.value(QStringLiteral("role")).toString();
            const QString key = target->provider()->key();
            const bool root = target->parent() == model->root() && !target->provider()->isGroup();
            const bool group = target->provider()->isGroup() &&
                ((key == QStringLiteral("case.grids") && role == QStringLiteral("case.grid")) ||
                 (key == QStringLiteral("case.schedule") && role == QStringLiteral("case.well")) ||
                 (key == QStringLiteral("case.fluidRock") && role == QStringLiteral("case.fluidRockTable")));
            if (!root && !group)
            {
                return QJsonObject();
            }
            caseId = target->provider()->caseId();
        }
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("items"), records);
    arguments.insert(QStringLiteral("targetCaseId"), caseId);
    arguments.insert(QStringLiteral("targetGridId"), gridId);
    return arguments;
}

bool IDOSDataTreeMenuProvider::canCut(const QMimeData* data) const
{
    const QJsonArray records = readTransfer(data).value(QStringLiteral("items")).toArray();
    if (records.isEmpty())
    {
        return false;
    }
    for (const QJsonValue& value : records)
    {
        const QJsonObject record = value.toObject();
        if (record.value(QStringLiteral("type")).toString() == QStringLiteral("case") ||
            (record.value(QStringLiteral("containerId")).toString().isEmpty() &&
             record.value(QStringLiteral("sourceCaseId")).toString().isEmpty()))
        {
            return false;
        }
    }
    return true;
}

bool IDOSDataTreeMenuProvider::canPaste(const QMimeData* data, const IDOSTreeItem* target, Qt::DropAction action) const
{
    if ((action != Qt::CopyAction && action != Qt::MoveAction) || (action == Qt::MoveAction && !canCut(data)))
    {
        return false;
    }
    const QJsonObject transfer = transferArguments(data, target);
    if (transfer.isEmpty())
    {
        return false;
    }
    QVariantMap arguments = transfer.toVariantMap();
    arguments.insert(QStringLiteral("move"), action == Qt::MoveAction);
    arguments.insert(QStringLiteral("targetItemId"), target != nullptr ? target->provider()->id() : QString());
    return view()->treeModel()->transferItems(arguments, true);
}

bool IDOSDataTreeMenuProvider::paste(const QMimeData* data, IDOSTreeItem* target, Qt::DropAction action)
{
    if (!canPaste(data, target, action))
    {
        return false;
    }
    QVariantMap arguments = transferArguments(data, target).toVariantMap();
    arguments.insert(QStringLiteral("move"), action == Qt::MoveAction);
    arguments.insert(QStringLiteral("targetItemId"), target != nullptr ? target->provider()->id() : QString());
    return view()->treeModel()->transferItems(arguments);
}
