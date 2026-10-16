#include <QAction>
#include <QMenu>
#include <QObject>
#include <QVariant>

#include "data/fracture/idosfracturematerialcatalog.h"
#include "data/fracture/idosfractureschedule.h"
#include "data/fracture/idosnaturalfractureset.h"
#include "data/grid/idosgridboundary.h"
#include "data/grid/idosgridsurface.h"
#include "data/production/idosproductionhistory.h"
#include "data/seismic/idosmicroseismicset.h"
#include "data/seismic/idosseismicvolume.h"
#include "idosdataobject.h"
#include "tree/idostreeitem.h"

#include "idosimporteddatatreeprovider.h"

IDOSImportedDataTreeProvider::IDOSImportedDataTreeProvider(IDOSDataObject* data)
    :
    QObject(nullptr)
    , IDOSTreeItemProvider(data)
{
}

QMenu* IDOSImportedDataTreeProvider::createContextMenu()
{
    IDOSTreeItem* treeItem = item();
    IDOSSeismicVolume* volume = qobject_cast<IDOSSeismicVolume*>(dataSource());
    if (treeItem == nullptr || !treeItem->provider()->caseId().isEmpty())
    {
        return nullptr;
    }
    QMenu* menu = new QMenu();
    if (volume != nullptr)
    {
        QAction* inlineAction = menu->addAction(tr("Select Inline Section..."));
        inlineAction->setProperty("dataId", volume->objectId());
        inlineAction->setProperty("operation", QStringLiteral("node.selectSeismicInline"));
        return menu;
    }
    const QString key = treeItem->provider()->key();
    if (key == QStringLiteral("data.microseismic"))
    {
        addImportAction(menu, tr("Import Microseismic Events..."),
            QStringLiteral("idos.microseismic.tsv"), QStringLiteral("idos.microseismic.set"),
            tr("Microseismic Event Tables (*.txt *.TXT);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.seismic"))
    {
        addImportAction(menu, tr("Import SEG-Y Seismic Volume..."),
            QStringLiteral("idos.seismic.segy"), QStringLiteral("idos.seismic.volume"),
            tr("SEG-Y Seismic Volumes (*.sgy *.SGY *.segy *.SEGY);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.hydraulicFractures"))
    {
        addImportAction(menu, tr("Import Fracture Pump Schedule..."),
            QStringLiteral("idos.fracture.schedule.tsv"), QStringLiteral("idos.fracture.schedule"),
            tr("Fracture Pump Schedules (*.txt *.TXT *.xlsx *.XLSX);;All Files (*)"));
        addImportAction(menu, tr("Import Proppant Catalog..."),
            QStringLiteral("idos.fracture.materialCatalog.tsv"), QStringLiteral("idos.fracture.proppantCatalog"),
            tr("Proppant Catalog (*.txt *.TXT);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.naturalFractures"))
    {
        addImportAction(menu, tr("Import Natural Fractures..."),
            QStringLiteral("idos.naturalFracture.ascii"), QStringLiteral("idos.naturalFracture.set"),
            tr("ASCII Natural Fracture Files (*.fab *.FAB);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.production"))
    {
        addImportAction(menu, tr("Import Production History..."),
            QStringLiteral("idos.production.vol"), QStringLiteral("idos.production.history"),
            tr("Production History (*.vol *.VOL);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.surfaceProperties"))
    {
        addImportAction(menu, tr("Import Grid Surface..."),
            QStringLiteral("idos.grid.surface.zims"), QStringLiteral("idos.grid.surface"),
            tr("ZIMS Grid Surface (*);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.boundaries"))
    {
        addImportAction(menu, tr("Import Grid Boundary..."),
            QStringLiteral("idos.grid.boundary.xygi"), QStringLiteral("idos.grid.boundary"),
            tr("Grid Boundary Point Files (*);;All Files (*)"));
    }
    if (menu->actions().isEmpty())
    {
        delete menu;
        return nullptr;
    }
    return menu;
}

QAction* IDOSImportedDataTreeProvider::addImportAction(
    QMenu* menu, const QString& text, const QString& providerId,
    const QString& dataTypeId, const QString& fileFilter)
{
    QAction* action = menu->addAction(text);
    action->setProperty("providerId", providerId);
    action->setProperty("dataTypeId", dataTypeId);
    action->setProperty("fileFilter", fileFilter);
    action->setProperty("operation", QStringLiteral("node.importDataset"));
    return action;
}

void IDOSImportedDataTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    if (item == nullptr)
    {
        return;
    }
    IDOSDataObject* object = dataSource();
    if (object == nullptr)
    {
        return;
    }
    item->provider()->setCheckable(qobject_cast<IDOSGridSurface*>(object) != nullptr ||
                       qobject_cast<IDOSGridBoundary*>(object) != nullptr ||
                       qobject_cast<IDOSNaturalFractureSet*>(object) != nullptr ||
                       qobject_cast<IDOSSeismicVolume*>(object) != nullptr);
    IDOSMicroseismicSet* microseismic = qobject_cast<IDOSMicroseismicSet*>(object);
    if (microseismic != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 events)").arg(object->name()).arg(microseismic->events().size()));
        return;
    }
    IDOSSeismicVolume* seismicVolume = qobject_cast<IDOSSeismicVolume*>(object);
    if (seismicVolume != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 traces × %3 samples)")
                          .arg(object->name()).arg(seismicVolume->traceCount())
                          .arg(seismicVolume->samplesPerTrace()));
        return;
    }
    IDOSGridSurface* surface = qobject_cast<IDOSGridSurface*>(object);
    if (surface != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 × %3 surface)")
                          .arg(object->name()).arg(surface->nx()).arg(surface->ny()));
        return;
    }
    IDOSGridBoundary* boundary = qobject_cast<IDOSGridBoundary*>(object);
    if (boundary != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 boundary points)")
                          .arg(object->name()).arg(boundary->xCoordinates().size()));
        return;
    }
    IDOSFractureSchedule* schedule = qobject_cast<IDOSFractureSchedule*>(object);
    if (schedule != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 samples)").arg(object->name()).arg(schedule->samples().size()));
        return;
    }
    IDOSNaturalFractureSet* fractures = qobject_cast<IDOSNaturalFractureSet*>(object);
    if (fractures != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 fractures)").arg(object->name()).arg(fractures->fractures().size()));
        return;
    }
    IDOSProductionHistory* production = qobject_cast<IDOSProductionHistory*>(object);
    if (production != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 records)").arg(object->name()).arg(production->records().size()));
        return;
    }
    IDOSFractureMaterialCatalog* catalog = qobject_cast<IDOSFractureMaterialCatalog*>(object);
    if (catalog != nullptr)
    {
        item->provider()->setName(QObject::tr("%1 (%2 materials)").arg(object->name()).arg(catalog->records().size()));
    }
}
