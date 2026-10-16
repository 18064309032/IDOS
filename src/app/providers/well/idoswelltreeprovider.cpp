#include <QIcon>
#include <QObject>

#include "data/well/idoswell.h"
#include "data/well/idoswelllogchannel.h"
#include "data/well/idoswellmarker.h"
#include "idosdataobject.h"
#include "tree/idostreeitem.h"

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
        item = new IDOSTreeItem(new IDOSTreeItemProvider());
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
    IDOSWell* well = qobject_cast<IDOSWell*>(dataSource());
    if (well == nullptr || item == nullptr)
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
