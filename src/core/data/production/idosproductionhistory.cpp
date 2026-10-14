#include "idosproductionhistory.h"

IDOSProductionHistory::IDOSProductionHistory(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

QString IDOSProductionHistory::typeId() const
{
    return QStringLiteral("idos.production.history");
}

QString IDOSProductionHistory::sourceFile() const
{
    return m_sourceFile;
}

void IDOSProductionHistory::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QStringList IDOSProductionHistory::columns() const
{
    return m_columns;
}

void IDOSProductionHistory::setColumns(const QStringList& columns)
{
    if (m_columns == columns)
    {
        return;
    }
    m_columns = columns;
    emit dataChanged();
}

QVariantList IDOSProductionHistory::records() const
{
    return m_records;
}

void IDOSProductionHistory::setRecords(const QVariantList& records)
{
    if (m_records == records)
    {
        return;
    }
    m_records = records;
    emit dataChanged();
}

