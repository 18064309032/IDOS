#include <algorithm>

#include <QObject>

#include "idosproductionhistoryobject.h"

IDOSProductionHistoryObject::IDOSProductionHistoryObject(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

QString IDOSProductionHistoryObject::typeId() const
{
    return QStringLiteral("idos.production.history");
}

QString IDOSProductionHistoryObject::sourceFile() const
{
    return m_sourceFile;
}

void IDOSProductionHistoryObject::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QStringList IDOSProductionHistoryObject::columns() const
{
    return m_columns;
}

void IDOSProductionHistoryObject::setColumns(const QStringList& columns)
{
    if (m_columns.size() == columns.size() && std::equal(m_columns.cbegin(), m_columns.cend(), columns.cbegin()))
    {
        return;
    }
    m_columns = columns;
    emit dataChanged();
}

QVariantList IDOSProductionHistoryObject::records() const
{
    return m_records;
}

void IDOSProductionHistoryObject::setRecords(const QVariantList& records)
{
    if (m_records == records)
    {
        return;
    }
    m_records = records;
    emit dataChanged();
}

bool IDOSProductionHistoryObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSProductionHistoryObject::clone() const
{
    IDOSProductionHistoryObject* copy = new IDOSProductionHistoryObject();
    copy->setName(name());
    copy->m_sourceFile = m_sourceFile;
    copy->m_columns = m_columns;
    copy->m_records = m_records;
    return copy;
}

QVariantMap IDOSProductionHistoryObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("columns"), QVariant::fromValue(m_columns));
    data.insert(QStringLiteral("records"), QVariant::fromValue(m_records));
    return data;
}

bool IDOSProductionHistoryObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    if (!data.contains(QStringLiteral("columns")) || data.value(QStringLiteral("columns")).userType() != qMetaTypeId<QStringList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("columns"));
        return false;
    }
    if (!data.contains(QStringLiteral("records")) || data.value(QStringLiteral("records")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("records"));
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_columns = data.value(QStringLiteral("columns")).value<QStringList>();
    m_records = data.value(QStringLiteral("records")).value<QVariantList>();
    return true;
}
