#include <QObject>

#include "idosfluidrocktablemetadata.h"

IDOSFluidRockTableMetadata::IDOSFluidRockTableMetadata(IDOSFluidRockTable::Category category)
    :
    m_category(category)
{
}

IDOSFluidRockTableMetadata::~IDOSFluidRockTableMetadata()
{
}

QString IDOSFluidRockTableMetadata::typeId() const
{
    return m_category == IDOSFluidRockTable::Category::Fluid
        ? QStringLiteral("idos.fluid.table")
        : QStringLiteral("idos.rockPhysics.table");
}

QString IDOSFluidRockTableMetadata::displayName() const
{
    return m_category == IDOSFluidRockTable::Category::Fluid
        ? QObject::tr("Fluid Table")
        : QObject::tr("Rock Physics Table");
}

IDOSDataObject* IDOSFluidRockTableMetadata::create() const
{
    return new IDOSFluidRockTable(m_category);
}
