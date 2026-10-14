#include <QObject>

#include "idosfractureschedule.h"
#include "idosfracturematerialcatalog.h"
#include "idosgridboundary.h"
#include "idosgridsurface.h"
#include "idosmicroseismicset.h"
#include "idosnaturalfractureset.h"
#include "idosproductionhistory.h"
#include "idosseismicvolume.h"
#include "idostreeitem.h"

#include "idosimporteddatatreeprovider.h"

IDOSImportedDataTreeProvider::IDOSImportedDataTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeProvider(data)
{
}

void IDOSImportedDataTreeProvider::populate(IDOSTreeItem* item)
{
    if (item == nullptr)
    {
        return;
    }
    IDOSDataObject* object = dataSource();
    if (object == nullptr)
    {
        return;
    }
    item->clearChildren();
    IDOSMicroseismicSet* microseismic = qobject_cast<IDOSMicroseismicSet*>(object);
    if (microseismic != nullptr)
    {
        item->setName(QObject::tr("%1 (%2 events)").arg(object->name()).arg(microseismic->events().size()));
        return;
    }
    IDOSSeismicVolume* seismicVolume = qobject_cast<IDOSSeismicVolume*>(object);
    if (seismicVolume != nullptr)
    {
        item->setName(QObject::tr("%1 (%2 traces × %3 samples)")
                          .arg(object->name()).arg(seismicVolume->traceCount())
                          .arg(seismicVolume->samplesPerTrace()));
        return;
    }
    IDOSGridSurface* surface = qobject_cast<IDOSGridSurface*>(object);
    if (surface != nullptr)
    {
        item->setName(QObject::tr("%1 (%2 × %3 surface)")
                          .arg(object->name()).arg(surface->nx()).arg(surface->ny()));
        return;
    }
    IDOSGridBoundary* boundary = qobject_cast<IDOSGridBoundary*>(object);
    if (boundary != nullptr)
    {
        item->setName(QObject::tr("%1 (%2 boundary points)")
                          .arg(object->name()).arg(boundary->xCoordinates().size()));
        return;
    }
    IDOSFractureSchedule* schedule = qobject_cast<IDOSFractureSchedule*>(object);
    if (schedule != nullptr)
    {
        item->setName(QObject::tr("%1 (%2 samples)").arg(object->name()).arg(schedule->samples().size()));
        return;
    }
    IDOSNaturalFractureSet* fractures = qobject_cast<IDOSNaturalFractureSet*>(object);
    if (fractures != nullptr)
    {
        item->setName(QObject::tr("%1 (%2 fractures)").arg(object->name()).arg(fractures->fractures().size()));
        return;
    }
    IDOSProductionHistory* production = qobject_cast<IDOSProductionHistory*>(object);
    if (production != nullptr)
    {
        item->setName(QObject::tr("%1 (%2 records)").arg(object->name()).arg(production->records().size()));
        return;
    }
    IDOSFractureMaterialCatalog* catalog = qobject_cast<IDOSFractureMaterialCatalog*>(object);
    if (catalog != nullptr)
    {
        item->setName(QObject::tr("%1 (%2 materials)").arg(object->name()).arg(catalog->records().size()));
    }
}
