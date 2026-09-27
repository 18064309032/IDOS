#include "idoswelldatatreeprovider.h"
#include "idostreebuilder.h"
#include "idostreepartkey.h"
#include "idoswell.h"

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
    IDOSTreeBuilder childBuilder = builder.childBuilder(root);
    childBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.trajectory")), QObject::tr("Trajectory"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-well-trajectory.svg")));
    childBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.logs")), QObject::tr("Well Logs"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-well-logs.svg")));
    childBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.completions")), QObject::tr("Completions"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-well-completions.svg")));
    childBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.zones")), QObject::tr("Zones"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-well-zones.svg")));
}

QString IDOSWellDataTreeProvider::groupKey() const
{
    return QStringLiteral("data.wells");
}
