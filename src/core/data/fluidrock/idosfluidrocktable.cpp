#include "idosfluidrocktable.h"

IDOSFluidRockTable::IDOSFluidRockTable(Category category, QObject* parent)
    :
    IDOSDataObject(parent)
    , m_category(category)
{
}

IDOSFluidRockTable::~IDOSFluidRockTable()
{
}

QString IDOSFluidRockTable::typeId() const
{
    return m_category == Category::Fluid
        ? QStringLiteral("idos.fluid.table")
        : QStringLiteral("idos.rockPhysics.table");
}

IDOSFluidRockTable::Category IDOSFluidRockTable::category() const
{
    return m_category;
}

QVariantMap IDOSFluidRockTable::tableData() const
{
    return m_tableData;
}

void IDOSFluidRockTable::setTableData(const QVariantMap& data)
{
    m_tableData = data;
    emit dataChanged();
}

QString IDOSFluidRockTable::sourceFile() const
{
    return m_sourceFile;
}

void IDOSFluidRockTable::setSourceFile(const QString& path)
{
    m_sourceFile = path;
    emit dataChanged();
}
