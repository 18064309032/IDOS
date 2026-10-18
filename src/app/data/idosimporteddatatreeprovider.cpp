#include <QAction>
#include <QMenu>
#include <QObject>
#include <QVariant>
#include <QVariantMap>

#include "data/fracture/idosfracturematerialcatalogobject.h"
#include "data/fracture/idosfracturescheduleobject.h"
#include "data/fracture/idosnaturalfracturesetobject.h"
#include "data/grid/idosgridboundaryobject.h"
#include "data/grid/idosgridsurfaceobject.h"
#include "data/production/idosproductionhistoryobject.h"
#include "data/seismic/idosmicroseismicsetobject.h"
#include "data/seismic/idosseismicvolumeobject.h"
#include "idosdataobject.h"
#include "tree/idoscasetreeitemmodel.h"
#include "tree/idosdatatreeitemmodel.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"

#include "idosimporteddatatreeprovider.h"

IDOSImportedDataTreeProvider::IDOSImportedDataTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeItemProvider(data)
{
}

QMenu* IDOSImportedDataTreeProvider::createContextMenu(const QList<IDOSTreeItem*>& selectedItems)
{
    if (item() == nullptr || item()->model() == nullptr)
    {
        return nullptr;
    }
    QMenu* menu = new QMenu();
    menu->setProperty("treeSession", item()->model()->sessionId());
    const QList<IDOSTreeItem*> targets = selectedItems.isEmpty() ? QList<IDOSTreeItem*>{item()} : selectedItems;
    if (isGroup())
    {
        if (selectedItems.size() <= 1 && caseId().isEmpty())
        {
            if (key() == QStringLiteral("data.microseismic"))
            {
                QAction* action = new QAction(tr("Import Microseismic Events..."), menu);
                menu->addAction(action);
                action->setProperty("providerId", QStringLiteral("idos.microseismic.tsv"));
                action->setProperty("dataTypeId", QStringLiteral("idos.microseismic.set"));
                action->setProperty("fileFilter", tr("Microseismic Event Tables (*.txt *.TXT);;All Files (*)"));
                connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onImportDataset);
            }
            if (key() == QStringLiteral("data.seismic"))
            {
                QAction* action = new QAction(tr("Import SEG-Y Seismic Volume..."), menu);
                menu->addAction(action);
                action->setProperty("providerId", QStringLiteral("idos.seismic.segy"));
                action->setProperty("dataTypeId", QStringLiteral("idos.seismic.volume"));
                action->setProperty("fileFilter",
                                    tr("SEG-Y Seismic Volumes (*.sgy *.SGY *.segy *.SEGY);;All Files (*)"));
                connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onImportDataset);
            }
            if (key() == QStringLiteral("data.hydraulicFractures"))
            {
                QAction* action = new QAction(tr("Import Fracture Pump Schedule..."), menu);
                menu->addAction(action);
                action->setProperty("providerId", QStringLiteral("idos.fracture.schedule.tsv"));
                action->setProperty("dataTypeId", QStringLiteral("idos.fracture.schedule"));
                action->setProperty("fileFilter",
                                    tr("Fracture Pump Schedules (*.txt *.TXT *.xlsx *.XLSX);;All Files (*)"));
                connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onImportDataset);
            }
            if (key() == QStringLiteral("data.hydraulicFractures"))
            {
                QAction* action = new QAction(tr("Import Proppant Catalog..."), menu);
                menu->addAction(action);
                action->setProperty("providerId", QStringLiteral("idos.fracture.materialCatalog.tsv"));
                action->setProperty("dataTypeId", QStringLiteral("idos.fracture.proppantCatalog"));
                action->setProperty("fileFilter", tr("Proppant Catalog (*.txt *.TXT);;All Files (*)"));
                connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onImportDataset);
            }
            if (key() == QStringLiteral("data.naturalFractures"))
            {
                QAction* action = new QAction(tr("Import Natural Fractures..."), menu);
                menu->addAction(action);
                action->setProperty("providerId", QStringLiteral("idos.naturalFracture.ascii"));
                action->setProperty("dataTypeId", QStringLiteral("idos.naturalFracture.set"));
                action->setProperty("fileFilter", tr("ASCII Natural Fracture Files (*.fab *.FAB);;All Files (*)"));
                connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onImportDataset);
            }
            if (key() == QStringLiteral("data.production"))
            {
                QAction* action = new QAction(tr("Import Production History..."), menu);
                menu->addAction(action);
                action->setProperty("providerId", QStringLiteral("idos.production.vol"));
                action->setProperty("dataTypeId", QStringLiteral("idos.production.history"));
                action->setProperty("fileFilter", tr("Production History (*.vol *.VOL);;All Files (*)"));
                connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onImportDataset);
            }
            if (key() == QStringLiteral("data.surfaceProperties"))
            {
                QAction* action = new QAction(tr("Import Grid Surface..."), menu);
                menu->addAction(action);
                action->setProperty("providerId", QStringLiteral("idos.grid.surface.zims"));
                action->setProperty("dataTypeId", QStringLiteral("idos.grid.surface"));
                action->setProperty("fileFilter", tr("ZIMS Grid Surface (*);;All Files (*)"));
                connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onImportDataset);
            }
            if (key() == QStringLiteral("data.boundaries"))
            {
                QAction* action = new QAction(tr("Import Grid Boundary..."), menu);
                menu->addAction(action);
                action->setProperty("providerId", QStringLiteral("idos.grid.boundary.xygi"));
                action->setProperty("dataTypeId", QStringLiteral("idos.grid.boundary"));
                action->setProperty("fileFilter", tr("Grid Boundary Point Files (*);;All Files (*)"));
                connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onImportDataset);
            }
        }
        return menu;
    }
    {
        const QStringList deletableTypes = {QStringLiteral("idos.microseismic.set"),
                                            QStringLiteral("idos.seismic.volume"),
                                            QStringLiteral("idos.fracture.schedule"),
                                            QStringLiteral("idos.naturalFracture.set"),
                                            QStringLiteral("idos.production.history"),
                                            QStringLiteral("idos.fracture.fluidCatalog"),
                                            QStringLiteral("idos.fracture.proppantCatalog"),
                                            QStringLiteral("idos.grid.surface"),
                                            QStringLiteral("idos.grid.boundary"),
                                            QStringLiteral("idos.fluid.table"),
                                            QStringLiteral("idos.rockPhysics.table")};
        QStringList ids;
        for (IDOSTreeItem* selected : targets)
        {
            IDOSDataObject* data = item()->model()->object(selected->provider()->dataId());
            if (data == nullptr || !selected->provider()->caseId().isEmpty() || selected->provider()->isGroup() ||
                !deletableTypes.contains(data->typeId()))
            {
                ids.clear();
                break;
            }
            ids.append(data->objectId());
        }
        if (!ids.isEmpty())
        {
            QAction* remove = new QAction(tr("Delete Data..."), menu);
            menu->addAction(remove);
            remove->setObjectName(QStringLiteral("deleteDataAction"));
            remove->setProperty("objectIds", ids);
            connect(remove, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onDeleteData);
        }

        IDOSDataObject* object = item()->model()->object(dataId());
        if (targets.size() == 1 && !isGroup() && partKey().isEmpty() && itemKey().isEmpty() && object != nullptr &&
            object->canClone())
        {
            QAction* rename = new QAction(tr("Rename..."), menu);
            menu->addAction(rename);
            rename->setObjectName(QStringLiteral("renameObjectAction"));
            connect(rename, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onRenameObject);
        }
    }
    if (selectedItems.size() <= 1 && caseId().isEmpty())
    {

        IDOSSeismicVolumeObject* volume = qobject_cast<IDOSSeismicVolumeObject*>(object());
        if (volume != nullptr)
        {
            QAction* action = new QAction(tr("Select Inline Section..."), menu);
            menu->addAction(action);
            connect(action, &QAction::triggered, this, &IDOSImportedDataTreeProvider::onSelectSeismicInline);
        }
    }
    return menu;
}

void IDOSImportedDataTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    if (item == nullptr)
    {
        return;
    }
    IDOSDataObject* object = this->object();
    if (object == nullptr)
    {
        return;
    }
    item->provider()->setCheckable(qobject_cast<IDOSGridSurfaceObject*>(object) != nullptr ||
                                   qobject_cast<IDOSGridBoundaryObject*>(object) != nullptr ||
                                   qobject_cast<IDOSNaturalFractureSetObject*>(object) != nullptr ||
                                   qobject_cast<IDOSSeismicVolumeObject*>(object) != nullptr);
    IDOSMicroseismicSetObject* microseismic = qobject_cast<IDOSMicroseismicSetObject*>(object);
    if (microseismic != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 events)").arg(object->name()).arg(microseismic->events().size()));
        return;
    }
    IDOSSeismicVolumeObject* seismicVolume = qobject_cast<IDOSSeismicVolumeObject*>(object);
    if (seismicVolume != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 traces × %3 samples)")
                                      .arg(object->name())
                                      .arg(seismicVolume->traceCount())
                                      .arg(seismicVolume->samplesPerTrace()));
        return;
    }
    IDOSGridSurfaceObject* surface = qobject_cast<IDOSGridSurfaceObject*>(object);
    if (surface != nullptr)
    {
        item->provider()->setName(
            QObject::tr("%1 (%2 × %3 surface)").arg(object->name()).arg(surface->nx()).arg(surface->ny()));
        return;
    }
    IDOSGridBoundaryObject* boundary = qobject_cast<IDOSGridBoundaryObject*>(object);
    if (boundary != nullptr)
    {
        item->provider()->setName(
            QObject::tr("%1 (%2 boundary points)").arg(object->name()).arg(boundary->xCoordinates().size()));
        return;
    }
    IDOSFractureScheduleObject* schedule = qobject_cast<IDOSFractureScheduleObject*>(object);
    if (schedule != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 samples)").arg(object->name()).arg(schedule->samples().size()));
        return;
    }
    IDOSNaturalFractureSetObject* fractures = qobject_cast<IDOSNaturalFractureSetObject*>(object);
    if (fractures != nullptr)
    {
        item->provider()->setName(
            QObject::tr("%1 (%2 fractures)").arg(object->name()).arg(fractures->fractures().size()));
        return;
    }
    IDOSProductionHistoryObject* production = qobject_cast<IDOSProductionHistoryObject*>(object);
    if (production != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 records)").arg(object->name()).arg(production->records().size()));
        return;
    }
    IDOSFractureMaterialCatalogObject* catalog = qobject_cast<IDOSFractureMaterialCatalogObject*>(object);
    if (catalog != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 materials)").arg(object->name()).arg(catalog->records().size()));
    }
}

void IDOSImportedDataTreeProvider::onRenameObject()
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

void IDOSImportedDataTreeProvider::onDeleteData()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->removeObjects(id(), action->parent()->property("treeSession").toString(),
                          action->property("objectIds").toStringList());
    }
}

void IDOSImportedDataTreeProvider::onSelectSeismicInline()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->selectSeismicInline(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSImportedDataTreeProvider::onImportDataset()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importDataset(id(), action->parent()->property("treeSession").toString(), action->text(),
                             action->property("fileFilter").toString(), action->property("providerId").toString(),
                             action->property("dataTypeId").toString());
    }
}
