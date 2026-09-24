#include "input/providers/idosgriddatatreeprovider.h"
#include "tree/idostreebuilder.h"
#include "tree/idostreepartkey.h"
#include "data/grid/idosgrid.h"

#include <QObject>

QString IDOSGridDataTreeProvider::providerId() const
{
    return QStringLiteral("idos.input.grid");
}

QString IDOSGridDataTreeProvider::typeId() const
{
    return QStringLiteral("idos.grid");
}

void IDOSGridDataTreeProvider::buildTree(IDOSTreeBuilder& builder, const IDOSDataObject* object)
{
    auto* grid = qobject_cast<const IDOSGrid*>(object);
    if (grid == nullptr)
    {
        return;
    }

    auto* root = builder.addObject(grid);
    auto childBuilder = builder.childBuilder(root);
    childBuilder.addPart(grid, IDOSTreePartKey(QStringLiteral("idos.grid.geometry")), QObject::tr("Geometry"));
    childBuilder.addPart(grid, IDOSTreePartKey(QStringLiteral("idos.grid.staticProperties")), QObject::tr("Static Properties"));
    childBuilder.addPart(grid, IDOSTreePartKey(QStringLiteral("idos.grid.dynamicProperties")), QObject::tr("Dynamic Properties"));
}
