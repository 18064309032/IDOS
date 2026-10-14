#include <QIcon>

#include "idosdataobject.h"
#include "idosfluidrocktable.h"
#include "idostreeitem.h"

#include "idosfluidrocktabletreeprovider.h"

IDOSFluidRockTableTreeProvider::IDOSFluidRockTableTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeProvider(data)
{
}

void IDOSFluidRockTableTreeProvider::populate(IDOSTreeItem* item)
{
    IDOSFluidRockTable* table = qobject_cast<IDOSFluidRockTable*>(dataSource());
    if (item == nullptr || table == nullptr)
    {
        return;
    }
    item->clearChildren();
    item->setName(table->name());
    item->setIcon(QIcon(table->category() == IDOSFluidRockTable::Category::Fluid
        ? QStringLiteral(":/images/gui-fluids.svg")
        : QStringLiteral(":/images/gui-rock-physics.svg")));
}
