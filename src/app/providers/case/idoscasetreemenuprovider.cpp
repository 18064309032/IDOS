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

#include "idoscasetreemenuprovider.h"

IDOSCaseTreeMenuProvider::IDOSCaseTreeMenuProvider(IDOSTreeView* view)
    :
    IDOSTreeMenuProvider(view)
{
}

IDOSCaseTreeMenuProvider::~IDOSCaseTreeMenuProvider()
{
}

QAction* IDOSCaseTreeMenuProvider::addAction(QMenu* menu, IDOSTreeView* view, const QString& text)
{
    QAction* action = menu->addAction(text);
    IDOSTreeItem* item = view->contextItem();
    action->setProperty("dataId", item != nullptr ? item->provider()->dataId() : QString());
    action->setProperty("caseId", item != nullptr ? item->provider()->caseId() : QString());
    return action;
}

void IDOSCaseTreeMenuProvider::addCaseActions(QMenu* menu, IDOSTreeView* view)
{
    if (view->treeModel() == nullptr || view->treeModel()->sessionId().isEmpty())
    {
        return;
    }
    IDOSTreeItem* item = view->contextItem();

    if (view->treeModel() == nullptr || view->treeModel()->name() != QStringLiteral("case"))
    {
        return;
    }
    if (item != nullptr && item->provider()->key() == QStringLiteral("case.grids"))
    {
        QAction* import = addAction(menu, view, tr("Import Grid..."));
        import->setObjectName(QStringLiteral("importGridAction"));
        import->setProperty("operation", QStringLiteral("case.onImportGrid"));

        return;
    }
    if (item == nullptr)
    {
        QAction* create = addAction(menu, view, tr("New Case..."));
        create->setObjectName(QStringLiteral("newCaseAction"));
        create->setProperty("operation", QStringLiteral("case.onNewCase"));

        QAction* import = addAction(menu, view, tr("Import Case..."));
        import->setObjectName(QStringLiteral("importCaseAction"));
        import->setProperty("operation", QStringLiteral("case.onImportCase"));

        return;
    }
    if (item->provider()->isGroup())
    {
        if (item->provider()->key() == QStringLiteral("case.summaryResults"))
        {
            QAction* import = addAction(menu, view, tr("Import Summary Results..."));
            import->setObjectName(QStringLiteral("importSummaryResultsAction"));
            import->setProperty("operation", QStringLiteral("case.onImportSummaryResults"));

        }
        return;
    }
    if (item->provider()->key().startsWith(QStringLiteral("case.summaryResults:")))
    {
        QAction* show = addAction(menu, view, tr("View Summary Curves..."));
        show->setObjectName(QStringLiteral("viewSummaryCurvesAction"));
        show->setProperty("operation", QStringLiteral("case.onViewSummaryResults"));

        return;
    }
    if (item->provider()->key().startsWith(QStringLiteral("case.curveTable:")) ||
        item->provider()->key().startsWith(QStringLiteral("case.fluidRockTable:")))
    {
        QAction* viewCurves = addAction(menu, view, tr("View Property Curves..."));
        viewCurves->setObjectName(QStringLiteral("viewPropertyCurvesAction"));
        viewCurves->setProperty("operation", QStringLiteral("case.onViewPropertyCurves"));

        return;
    }
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(view->treeModel()->dataObject(item->provider()->dataId()));
    if (caseObject != nullptr && item->parent() == view->treeModel()->root())
    {
        QAction* generate = addAction(menu, view, tr("Generate Simulation Input..."));
        generate->setObjectName(QStringLiteral("generateSimulationInputAction"));
        generate->setProperty("operation", QStringLiteral("case.onGenerateInput"));

        QAction* rename = addAction(menu, view, tr("Rename Case..."));
        rename->setObjectName(QStringLiteral("renameCaseAction"));
        rename->setProperty("operation", QStringLiteral("case.onRenameCase"));

        QAction* remove = addAction(menu, view, tr("Delete Case..."));
        remove->setObjectName(QStringLiteral("deleteCaseAction"));
        remove->setProperty("operation", QStringLiteral("case.onDeleteCase"));

        return;
    }
}

QMenu* IDOSCaseTreeMenuProvider::createContextMenu(
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
    if (view == nullptr)
    {

        return menu;
    }
    if (selectedItems.size() <= 1)
    {
        addCaseActions(menu, view);
    }
    addTransferActions(menu);

    return menu;
}

void IDOSCaseTreeMenuProvider::addTransferActions(QMenu* menu)
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
    if (items.size() == 1)
    {
        IDOSDataObject* object = view->treeModel()->dataObject(items.first().toObject().value(QStringLiteral("objectId")).toString());
        // Case and filter menus already provide rename actions.
        if (qobject_cast<IDOSCaseObject*>(object) == nullptr && qobject_cast<IDOSGridFilter*>(object) == nullptr)
        {
            QAction* rename = addAction(menu, view, tr("Rename..."));
            rename->setObjectName(QStringLiteral("renameObjectAction"));
            rename->setProperty("operation", QStringLiteral("case.onRenameObject"));

        }
    }
}



QMimeData* IDOSCaseTreeMenuProvider::createMimeData(const QList<IDOSTreeItem*>& items) const
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

QJsonObject IDOSCaseTreeMenuProvider::readTransfer(const QMimeData* data) const
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

QJsonObject IDOSCaseTreeMenuProvider::transferArguments(const QMimeData* data, const IDOSTreeItem* target) const
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

bool IDOSCaseTreeMenuProvider::canCut(const QMimeData* data) const
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

bool IDOSCaseTreeMenuProvider::canPaste(const QMimeData* data, const IDOSTreeItem* target, Qt::DropAction action) const
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

bool IDOSCaseTreeMenuProvider::paste(const QMimeData* data, IDOSTreeItem* target, Qt::DropAction action)
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
