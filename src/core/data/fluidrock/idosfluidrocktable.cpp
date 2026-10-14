#include <QObject>

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

bool IDOSFluidRockTable::canClone() const
{
    return true;
}

IDOSDataObject* IDOSFluidRockTable::clone() const
{
    IDOSFluidRockTable* copy = new IDOSFluidRockTable();
    copy->setName(name());
    copy->setVisible(isVisible());
    copy->m_category = m_category;
    copy->m_tableData = m_tableData;
    copy->m_sourceFile = m_sourceFile;
    return copy;
}

QVariantMap IDOSFluidRockTable::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("category"), static_cast<int>(m_category));
    data.insert(QStringLiteral("tableData"), QVariant::fromValue(m_tableData));
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    return data;
}

bool IDOSFluidRockTable::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("category")) || data.value(QStringLiteral("category")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("category"));
        return false;
    }
    if (data.value(QStringLiteral("category")).toInt() < 0 || data.value(QStringLiteral("category")).toInt() >= 2)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("tableData")) || data.value(QStringLiteral("tableData")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("tableData"));
        return false;
    }
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    m_category = static_cast<Category>(data.value(QStringLiteral("category")).toInt());
    m_tableData = data.value(QStringLiteral("tableData")).value<QVariantMap>();
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    return true;
}
