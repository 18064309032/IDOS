#include "input/providers/idoswelldatatreeprovider.h"
#include "tree/idostreebuilder.h"
#include "tree/idostreepartkey.h"
#include "data/well/idoswell.h"

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
    auto* well = qobject_cast<const IDOSWell*>(object);
    if (well == nullptr)
    {
        return;
    }

    auto* root = builder.addObject(well);
    auto childBuilder = builder.childBuilder(root);
    childBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.trajectory")), QObject::tr("Trajectory"));
    childBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.logs")), QObject::tr("Well Logs"));
    childBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.completions")), QObject::tr("Completions"));
    childBuilder.addPart(well, IDOSTreePartKey(QStringLiteral("idos.well.zones")), QObject::tr("Zones"));
}
