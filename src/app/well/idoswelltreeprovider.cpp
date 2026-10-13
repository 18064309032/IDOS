#include <QObject>

#include "idosdataobject.h"
#include "idostreeitem.h"
#include "idoswell.h"
#include "idoswelllogchannel.h"
#include "idoswellmarker.h"

#include "idoswelltreeprovider.h"

IDOSWellTreeProvider::IDOSWellTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeProvider(data)
{
}

IDOSTreeItem* IDOSWellTreeProvider::addItem(IDOSTreeItem* parent, const QString& key, const QString& title)
{
    IDOSTreeItem* item = new IDOSTreeItem();
    item->setKey(key);
    item->setName(title);
    item->setDataId(parent->dataId());
    item->setCaseId(parent->caseId());
    item->setPartKey(key);
    parent->appendChild(item);
    return item;
}

void IDOSWellTreeProvider::populate(IDOSTreeItem* item)
{
    IDOSWell* well = qobject_cast<IDOSWell*>(dataSource());
    if (well == nullptr || item == nullptr)
    {
        return;
    }
    item->clearChildren();
    item->setName(well->name());
    item->setIcon(QIcon(QStringLiteral(":/images/gui-well.svg")));
    item->setCheckable(true);
    item->setChecked(well->isVisible());
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
            leaf->setPartKey(QStringLiteral("idos.well.logs"));
            leaf->setItemKey(channel.name());
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
            leaf->setPartKey(QStringLiteral("idos.well.zones"));
            leaf->setItemKey(marker.name());
        }
    }
    if (well->hasCheckshot())
    {
        addItem(item, QStringLiteral("idos.well.checkshots"),
                QObject::tr("Checkshots (%1 samples)").arg(well->checkshot().depths().size()));
    }
}
