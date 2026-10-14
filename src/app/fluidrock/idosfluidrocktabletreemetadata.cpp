#include "idosdataobject.h"
#include "idosfluidrocktabletreeprovider.h"

#include "idosfluidrocktabletreemetadata.h"

IDOSFluidRockTableTreeMetadata::IDOSFluidRockTableTreeMetadata(IDOSFluidRockTable::Category category)
    :
    m_category(category)
{
}

QString IDOSFluidRockTableTreeMetadata::typeId() const
{
    return m_category == IDOSFluidRockTable::Category::Fluid
        ? QStringLiteral("idos.fluid.table")
        : QStringLiteral("idos.rockPhysics.table");
}

QString IDOSFluidRockTableTreeMetadata::treeName() const
{
    return QStringLiteral("data");
}

QString IDOSFluidRockTableTreeMetadata::groupKey() const
{
    return m_category == IDOSFluidRockTable::Category::Fluid
        ? QStringLiteral("data.fluids")
        : QStringLiteral("data.rockPhysics");
}

IDOSTreeProvider* IDOSFluidRockTableTreeMetadata::create(IDOSDataObject* data) const
{
    return new IDOSFluidRockTableTreeProvider(data);
}
