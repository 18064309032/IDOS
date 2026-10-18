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
#include "data/fluidrock/idosfluidrocktableobject.h"
#include "data/fracture/idosfracturematerialcatalogobject.h"
#include "data/fracture/idosfracturescheduleobject.h"
#include "data/fracture/idosnaturalfracturesetobject.h"
#include "data/grid/idosgridboundaryobject.h"
#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "data/grid/idosgridsurfaceobject.h"
#include "data/production/idosproductionhistoryobject.h"
#include "data/seismic/idosmicroseismicsetobject.h"
#include "data/seismic/idosseismicvolumeobject.h"
#include "data/well/idoswellobject.h"
#include "idosdataobject.h"
#include "tree/idosdatatreeview.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeitemprovider.h"
#include "tree/idostreemodel.h"
#include "tree/idostreeview.h"

#include "idosdatatreemenuprovider.h"

IDOSDataTreeMenuProvider::IDOSDataTreeMenuProvider(IDOSDataTreeView* view)
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
    Q_UNUSED(selectedItems)
    Q_UNUSED(contextItem)
    return new QMenu();
}

QMimeData* IDOSDataTreeMenuProvider::createMimeData(const QList<IDOSTreeItem*>& items) const
{
    IDOSTreeModel* model = !items.isEmpty() && items.first() != nullptr ? items.first()->model() : nullptr;
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
        IDOSDataObject* data = model->object(item->provider()->dataId());
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
        else if (qobject_cast<IDOSWellObject*>(data) != nullptr)
        {
            role = QStringLiteral("case.well");
            type = QStringLiteral("well");
            group = QStringLiteral("data.wells");
        }
        else if (qobject_cast<IDOSGridObject*>(data) != nullptr)
        {
            role = QStringLiteral("case.grid");
            type = QStringLiteral("grid");
        }
        else if (qobject_cast<IDOSGridPropertyObject*>(data) != nullptr)
        {
            type = QStringLiteral("property");
        }
        else if (qobject_cast<IDOSGridFilterObject*>(data) != nullptr)
        {
            type = QStringLiteral("filter");
        }
        else if (qobject_cast<IDOSFluidRockTableObject*>(data) != nullptr)
        {
            role = QStringLiteral("case.fluidRockTable");
            type = QStringLiteral("fluidRock");
            group = qobject_cast<IDOSFluidRockTableObject*>(data)->category() == IDOSFluidRockTableObject::Category::Fluid
                ? QStringLiteral("data.fluids") : QStringLiteral("data.rockPhysics");
        }
        QJsonObject record;
        record.insert(QStringLiteral("objectId"), data->objectId());
        record.insert(QStringLiteral("role"), role);
        record.insert(QStringLiteral("group"), group);
        record.insert(QStringLiteral("type"), type);
        record.insert(QStringLiteral("containerId"), data->containerId());
        record.insert(QStringLiteral("sourceCaseId"), item->provider()->caseId());
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(data);
        if (property != nullptr)
        {
            record.insert(QStringLiteral("dynamic"), property->kind() == IDOSGridPropertyObject::Kind::Dynamic);
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
            IDOSDataObject* destination = model->object(target->provider()->dataId());
            IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(destination);
            IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(destination);
            IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(destination);
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
