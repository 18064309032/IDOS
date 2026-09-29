#include "idoswelldatatreeprovider.h"
#include "idostreebuilder.h"
#include "idostreepartkey.h"
#include "idoswell.h"
#include "idoswelllogchannel.h"
#include "idoswellmarker.h"

#include <QList>
#include <QObject>

QString IDOSWellDataTreeProvider::providerId() const
{
    return QStringLiteral("idos.input.well");
}

QString IDOSWellDataTreeProvider::typeId() const
{
    return QStringLiteral("idos.well");
}

void IDOSWellDataTreeProvider::buildTree(IDOSTreeBuilder& builder, const IDOSDataObject* object)
{
    const IDOSWell* well = qobject_cast<const IDOSWell*>(object);
    if (well == nullptr)
    {
        return;
    }

    IDOSObjectTreeNode* root = builder.addObject(well);
    if (root == nullptr)
    {
        return;
    }
    root->setIcon(QIcon(QStringLiteral(":/images/gui-well.svg")));
    // 井节点可勾选触发 3D 渲染：有轨迹时显示轨迹线，仅有井口时显示井口点。
    root->setCheckable(true);
    root->setChecked(well->isVisible());

    // 懒构建：各数据 part 仅有数据时才出现，空 part 不占位
    IDOSTreeBuilder childBuilder = builder.childBuilder(root);
    if (well->hasPath())
    {
        IDOSTreePartNode* trajectory = childBuilder.addPart(
            well, IDOSTreePartKey(QStringLiteral("idos.well.trajectory")),
            QObject::tr("Trajectory (%1 points)").arg(well->path().pointCount()));
        trajectory->setIcon(QIcon(QStringLiteral(":/images/gui-well-trajectory.svg")));
    }
    if (well->hasLogs())
    {
        IDOSTreePartNode* logs = childBuilder.addPart(
            well, IDOSTreePartKey(QStringLiteral("idos.well.logs")),
            QObject::tr("Well Log Curves (%1 channels)").arg(well->logs().channelCount()));
        logs->setIcon(QIcon(QStringLiteral(":/images/gui-well-logs.svg")));
        IDOSTreeBuilder leafBuilder = childBuilder.childBuilder(logs);
        const QList<IDOSWellLogChannel> channels = well->logs().channels();
        for (const IDOSWellLogChannel& channel : channels)
        {
            // itemKey 取通道名，与 partKey 组合构成叶子唯一键
            leafBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.logs")),
                                channel.name(), channel.name());
        }
    }
    if (!well->completions().isEmpty())
    {
        IDOSTreePartNode* completions = childBuilder.addPart(
            well, IDOSTreePartKey(QStringLiteral("idos.well.completions")),
            QObject::tr("Completions (%1)").arg(well->completions().size()));
        completions->setIcon(QIcon(QStringLiteral(":/images/gui-well-completions.svg")));
    }
    if (well->hasMarkers())
    {
        IDOSTreePartNode* markers = childBuilder.addPart(
            well, IDOSTreePartKey(QStringLiteral("idos.well.zones")),
            QObject::tr("Markers (%1)").arg(well->markers().markerCount()));
        markers->setIcon(QIcon(QStringLiteral(":/images/gui-well-zones.svg")));
        IDOSTreeBuilder leafBuilder = childBuilder.childBuilder(markers);
        const QList<IDOSWellMarker> markerList = well->markers().markers();
        for (const IDOSWellMarker& marker : markerList)
        {
            leafBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.zones")),
                                marker.name(), marker.name());
        }
    }
}

QString IDOSWellDataTreeProvider::groupKey() const
{
    return QStringLiteral("data.wells");
}
