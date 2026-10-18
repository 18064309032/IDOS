#include <QAction>
#include <QIcon>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QObject>

#include "data/fluidrock/idosfluidrocktableobject.h"
#include "data/well/idoswelllogchannel.h"
#include "data/well/idoswellmarker.h"
#include "data/well/idoswellobject.h"
#include "idosdataobject.h"
#include "tree/idoscasetreeitemmodel.h"
#include "tree/idosdatatreeitemmodel.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"

#include "idoswelltreeprovider.h"

IDOSWellTreeProvider::IDOSWellTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeItemProvider(data)
{
}

IDOSTreeItem* IDOSWellTreeProvider::addItem(IDOSTreeItem* parent, const QString& key, const QString& title)
{
    IDOSTreeItem* item = parent->findChild(key);
    if (item == nullptr)
    {
        item = new IDOSTreeItem(new IDOSWellTreeProvider(object()));
        parent->appendChild(item);
    }

    item->provider()->setKey(key);
    item->provider()->setName(title);
    item->provider()->setDataId(parent->provider()->dataId());
    item->provider()->setCaseId(parent->provider()->caseId());
    item->provider()->setPartKey(key);
    return item;
}

void IDOSWellTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    IDOSWellObject* well = qobject_cast<IDOSWellObject*>(object());
    if (well == nullptr || item == nullptr || !partKey().isEmpty() || isGroup())
    {
        return;
    }

    item->provider()->setName(well->name());
    item->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-well.svg")));
    item->provider()->setCheckable(true);
    if (well->hasPath())
    {
        addItem(item, QStringLiteral("idos.well.trajectory"),
                QObject::tr("Trajectory (%1 points)").arg(well->path().pointCount()));
    }
    if (well->hasLogs())
    {
        IDOSTreeItem* logs = addItem(item, QStringLiteral("idos.well.logs"),
                                     QObject::tr("Well Log Curves (%1 channels)").arg(well->logs().channelCount()));
        for (const IDOSWellLogChannel& channel : well->logs().channels())
        {
            IDOSTreeItem* leaf = addItem(logs, channel.name(), channel.name());
            leaf->provider()->setPartKey(QStringLiteral("idos.well.logs"));
            leaf->provider()->setItemKey(channel.name());
        }
    }
    if (!well->completions().isEmpty())
    {
        addItem(item, QStringLiteral("idos.well.completions"),
                QObject::tr("Completions (%1)").arg(well->completions().size()));
    }
    if (well->hasMarkers())
    {
        IDOSTreeItem* markers = addItem(item, QStringLiteral("idos.well.zones"),
                                        QObject::tr("Markers (%1)").arg(well->markers().markerCount()));
        for (const IDOSWellMarker& marker : well->markers().markers())
        {
            IDOSTreeItem* leaf = addItem(markers, marker.name(), marker.name());
            leaf->provider()->setPartKey(QStringLiteral("idos.well.zones"));
            leaf->provider()->setItemKey(marker.name());
        }
    }
    if (well->hasCheckshot())
    {
        addItem(item, QStringLiteral("idos.well.checkshots"),
                QObject::tr("Checkshots (%1 samples)").arg(well->checkshot().depths().size()));
    }
}

QMenu* IDOSWellTreeProvider::createContextMenu(const QList<IDOSTreeItem*>& selectedItems)
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
        if (item->provider()->key() == QStringLiteral("data.wells"))
        {
            if (model == nullptr || model->name() != QStringLiteral("data"))
            {
                return menu;
            }
            QAction* create = new QAction(tr("New Well..."), menu);
            menu->addAction(create);
            create->setObjectName(QStringLiteral("newWellAction"));
            connect(create, &QAction::triggered, this, &IDOSWellTreeProvider::onNewWell);

            QAction* import = new QAction(tr("Import Well Data..."), menu);
            menu->addAction(import);
            import->setObjectName(QStringLiteral("importWellAction"));
            connect(import, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWell);

            return menu;
        }
        IDOSWellObject* well = qobject_cast<IDOSWellObject*>(model->object(item->provider()->dataId()));
        if (well == nullptr)
        {
            return menu;
        }
        const QString part = item->provider()->partKey();
        const bool isWellItem = item->parent() != nullptr &&
                                item->parent()->provider()->key() == QStringLiteral("data.wells") &&
                                item->provider()->key().startsWith(QStringLiteral("object:"));
        const bool isLeafItem =
            item->parent() != nullptr && item->parent()->provider()->key().startsWith(QStringLiteral("idos.well."));
        if (isWellItem)
        {
            QAction* importHeader = new QAction(tr("Import Well Header..."), menu);
            menu->addAction(importHeader);
            importHeader->setObjectName(QStringLiteral("importWellHeaderAction"));
            connect(importHeader, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWellHeader);

            QAction* importWell = new QAction(tr("Import Well Data..."), menu);
            menu->addAction(importWell);
            importWell->setObjectName(QStringLiteral("importWellDataSingleAction"));
            connect(importWell, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWell);

            QAction* show = new QAction(tr("View Well Data..."), menu);
            menu->addAction(show);
            show->setObjectName(QStringLiteral("viewWellSummaryAction"));
            show->setProperty("part", QStringLiteral("idos.well.summary"));
            connect(show, &QAction::triggered, this, &IDOSWellTreeProvider::onViewWellData);
        }
        else if (!isLeafItem && part == QStringLiteral("idos.well.trajectory"))
        {
            QAction* show = new QAction(tr("View Trajectory Data..."), menu);
            menu->addAction(show);
            show->setObjectName(QStringLiteral("viewWellPathAction"));
            show->setProperty("part", part);
            connect(show, &QAction::triggered, this, &IDOSWellTreeProvider::onViewWellData);

            QAction* import = new QAction(tr("Import or Replace Trajectory..."), menu);
            menu->addAction(import);
            import->setObjectName(QStringLiteral("importWellPathAction"));
            connect(import, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWellTrajectory);
        }
        else if (part == QStringLiteral("idos.well.logs"))
        {
            if (!isLeafItem)
            {
                QAction* import = new QAction(tr("Import Well Curves..."), menu);
                menu->addAction(import);
                import->setObjectName(QStringLiteral("importWellCurvesAction"));
                import->setProperty("part", part);
                connect(import, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWellCurves);
            }
            QAction* show = new QAction(tr("View Curve Data..."), menu);
            menu->addAction(show);
            show->setObjectName(QStringLiteral("viewWellInputDataAction"));
            show->setProperty("part", part);
            show->setProperty("itemKey", item->provider()->itemKey());
            connect(show, &QAction::triggered, this, &IDOSWellTreeProvider::onViewWellData);
        }
        else if (!isLeafItem &&
                 (part == QStringLiteral("idos.well.zones") || part == QStringLiteral("idos.well.completions") ||
                  part == QStringLiteral("idos.well.checkshots")))
        {
            const QString label = part == QStringLiteral("idos.well.zones")         ? tr("Markers")
                                  : part == QStringLiteral("idos.well.completions") ? tr("Completions")
                                                                                    : tr("Checkshots");
            QAction* import = new QAction(tr("Import %1 Data...").arg(label), menu);
            menu->addAction(import);
            import->setObjectName(QStringLiteral("importWellPartAction"));
            import->setProperty("part", part);
            if (part == QStringLiteral("idos.well.zones"))
            {
                connect(import, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWellMarkers);
            }
            else if (part == QStringLiteral("idos.well.completions"))
            {
                connect(import, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWellCompletions);
            }
            else
            {
                connect(import, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWellCheckshots);
            }

            QAction* show = new QAction(tr("View %1 Data...").arg(label), menu);
            menu->addAction(show);
            show->setObjectName(QStringLiteral("viewWellInputDataAction"));
            show->setProperty("part", part);
            connect(show, &QAction::triggered, this, &IDOSWellTreeProvider::onViewWellData);
        }
        else if (isWellItem && part == QStringLiteral("idos.well.trajectory"))
        {
            QAction* importHeader = new QAction(tr("Import Well Header..."), menu);
            menu->addAction(importHeader);
            importHeader->setObjectName(QStringLiteral("importWellHeaderAction"));
            connect(importHeader, &QAction::triggered, this, &IDOSWellTreeProvider::onImportWellHeader);
        }
        else if (isLeafItem && part == QStringLiteral("idos.well.zones"))
        {
            QAction* show = new QAction(tr("View Marker Data..."), menu);
            menu->addAction(show);
            show->setObjectName(QStringLiteral("viewWellInputDataAction"));
            show->setProperty("part", part);
            show->setProperty("itemKey", item->provider()->itemKey());
            connect(show, &QAction::triggered, this, &IDOSWellTreeProvider::onViewWellData);
        }
        if (isWellItem)
        {
            QAction* remove = new QAction(tr("Delete Well..."), menu);
            menu->addAction(remove);
            remove->setObjectName(QStringLiteral("deleteWellAction"));
            remove->setProperty("dataId", dataId());
            connect(remove, &QAction::triggered, this, &IDOSWellTreeProvider::onDeleteWell);
        }

        {
            if (!isGroup() && (partKey().isEmpty() || partKey() == QStringLiteral("idos.well.trajectory")) &&
                qobject_cast<IDOSWellObject*>(model->object(dataId())) != nullptr)
            {
                menu->addSeparator();
                QAction* action = new QAction(tr("Display Settings..."), menu);
                menu->addAction(action);
                action->setObjectName(QStringLiteral("wellDisplaySettingsAction"));
                action->setProperty("wellId", this->item()->provider()->dataId());
                action->setProperty("trajectory",
                                    this->item()->provider()->partKey() == QStringLiteral("idos.well.trajectory"));
                action->setEnabled(renderAvailable);
                connect(action, &QAction::triggered, this, &IDOSWellTreeProvider::onWellSettingsTriggered);
            }
        }
    }
    {

        if (caseId().isEmpty())
        {
            QJsonArray records;
            for (IDOSTreeItem* selected : targets)
            {
                IDOSTreeItemProvider* provider = selected->provider();
                IDOSDataObject* object = provider->object();
                const bool well = qobject_cast<IDOSWellObject*>(object) != nullptr;
                const bool table = qobject_cast<IDOSFluidRockTableObject*>(object) != nullptr;
                if ((!well && !table) || provider->isGroup() || !provider->caseId().isEmpty() ||
                    !provider->partKey().isEmpty() || !provider->itemKey().isEmpty())
                {
                    records = QJsonArray();
                    break;
                }
                QJsonObject record;
                record.insert(QStringLiteral("objectId"), object->objectId());
                record.insert(QStringLiteral("role"),
                              well ? QStringLiteral("case.well") : QStringLiteral("case.fluidRockTable"));
                records.append(record);
            }
            if (records.size() == targets.size())
            {
                QAction* attach = new QAction(tr("Add to Case..."), menu);
                menu->addAction(attach);
                attach->setObjectName(QStringLiteral("addToCaseAction"));
                attach->setProperty("items", records);
                connect(attach, &QAction::triggered, this, &IDOSWellTreeProvider::onAddToCase);
            }
        }

        IDOSDataObject* object = item()->model()->object(dataId());
        if (targets.size() == 1 && !isGroup() && partKey().isEmpty() && itemKey().isEmpty() && object != nullptr &&
            object->canClone())
        {
            QAction* rename = new QAction(tr("Rename..."), menu);
            menu->addAction(rename);
            rename->setObjectName(QStringLiteral("renameObjectAction"));
            connect(rename, &QAction::triggered, this, &IDOSWellTreeProvider::onRenameObject);
        }
    }
    return menu;
}

void IDOSWellTreeProvider::onRenameObject()
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

void IDOSWellTreeProvider::onNewWell()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->newWell(id(), action->parent()->property("treeSession").toString());
    }
}

void IDOSWellTreeProvider::onImportWell()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importWell(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importWell(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSWellTreeProvider::onViewWellData()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->viewWellData(id(), action->parent()->property("treeSession").toString(), dataId(),
                            action->property("part").toString(), action->property("itemKey").toString());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->viewWellData(id(), action->parent()->property("treeSession").toString(), dataId(),
                            action->property("part").toString(), action->property("itemKey").toString());
    }
}

void IDOSWellTreeProvider::onDeleteWell()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->deleteWell(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->deleteWell(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSWellTreeProvider::onWellSettingsTriggered()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->showWellSettings(id(), action->parent()->property("treeSession").toString(),
                                action->property("wellId").toString(), action->property("trajectory").toBool());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->showWellSettings(id(), action->parent()->property("treeSession").toString(),
                                action->property("wellId").toString(), action->property("trajectory").toBool());
    }
}

void IDOSWellTreeProvider::onAddToCase()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->addToCase(id(), action->parent()->property("treeSession").toString(),
                         action->property("items").value<QJsonArray>());
    }
}

void IDOSWellTreeProvider::onImportWellHeader()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importWellHeader(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importWellHeader(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSWellTreeProvider::onImportWellTrajectory()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importWellTrajectory(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importWellTrajectory(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSWellTreeProvider::onImportWellCurves()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importWellCurves(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importWellCurves(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSWellTreeProvider::onImportWellMarkers()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importWellMarkers(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importWellMarkers(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSWellTreeProvider::onImportWellCompletions()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importWellCompletions(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importWellCompletions(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSWellTreeProvider::onImportWellCheckshots()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importWellCheckshots(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importWellCheckshots(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}
