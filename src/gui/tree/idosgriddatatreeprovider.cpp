#include <QObject>

#include "idosgrid.h"
#include "idostreebuilder.h"
#include "idostreepartkey.h"

#include "idosgriddatatreeprovider.h"

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
    const IDOSGrid* grid = qobject_cast<const IDOSGrid*>(object);
    if (grid == nullptr)
    {
        return;
    }

    IDOSObjectTreeNode* root = builder.addObject(grid);
    if (root == nullptr)
    {
        return;
    }
    root->setIcon(QIcon(QStringLiteral(":/images/gui-grid.svg")));
    IDOSTreeBuilder childBuilder = builder.childBuilder(root);
    childBuilder.addPart(grid, IDOSTreePartKey(QStringLiteral("idos.grid.geometry")), QObject::tr("Geometry"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-grid-geometry.svg")));
    childBuilder
        .addPart(grid, IDOSTreePartKey(QStringLiteral("idos.grid.staticProperties")), QObject::tr("Static Properties"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-grid-static-properties.svg")));
    childBuilder
        .addPart(grid, IDOSTreePartKey(QStringLiteral("idos.grid.dynamicProperties")),
                 QObject::tr("Dynamic Properties"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-grid-dynamic-properties.svg")));
}
