#include <algorithm>

#include <QObject>

#include "idosfracturematerialcatalog.h"

IDOSFractureMaterialCatalog::IDOSFractureMaterialCatalog(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_kind(Kind::Fluid)
{
}

QString IDOSFractureMaterialCatalog::typeId() const
{
    if (m_kind == Kind::Proppant)
    {
        return QStringLiteral("idos.fracture.proppantCatalog");
    }
    return QStringLiteral("idos.fracture.fluidCatalog");
}

IDOSFractureMaterialCatalog::Kind IDOSFractureMaterialCatalog::kind() const
{
    return m_kind;
}

void IDOSFractureMaterialCatalog::setKind(Kind kind)
{
    if (m_kind == kind)
    {
        return;
    }
    m_kind = kind;
    emit dataChanged();
}

QString IDOSFractureMaterialCatalog::sourceFile() const
{
    return m_sourceFile;
}

void IDOSFractureMaterialCatalog::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QStringList IDOSFractureMaterialCatalog::columns() const
{
    return m_columns;
}

void IDOSFractureMaterialCatalog::setColumns(const QStringList& columns)
{
    if (m_columns.size() == columns.size() && std::equal(m_columns.cbegin(), m_columns.cend(), columns.cbegin()))
    {
        return;
    }
    m_columns = columns;
    emit dataChanged();
}

QVariantList IDOSFractureMaterialCatalog::records() const
{
    return m_records;
}

void IDOSFractureMaterialCatalog::setRecords(const QVariantList& records)
{
    if (m_records == records)
    {
        return;
    }
    m_records = records;
    emit dataChanged();
}

bool IDOSFractureMaterialCatalog::canClone() const
{
    return true;
}

IDOSDataObject* IDOSFractureMaterialCatalog::clone() const
{
    IDOSFractureMaterialCatalog* copy = new IDOSFractureMaterialCatalog();
    copy->setName(name());
    copy->setVisible(isVisible());
    copy->m_kind = m_kind;
    copy->m_sourceFile = m_sourceFile;
    copy->m_columns = m_columns;
    copy->m_records = m_records;
    return copy;
}

QVariantMap IDOSFractureMaterialCatalog::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("kind"), static_cast<int>(m_kind));
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("columns"), QVariant::fromValue(m_columns));
    data.insert(QStringLiteral("records"), QVariant::fromValue(m_records));
    return data;
}

bool IDOSFractureMaterialCatalog::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("kind")) || data.value(QStringLiteral("kind")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("kind"));
        return false;
    }
    if (data.value(QStringLiteral("kind")).toInt() < 0 || data.value(QStringLiteral("kind")).toInt() >= 2)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
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
    m_kind = static_cast<Kind>(data.value(QStringLiteral("kind")).toInt());
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_columns = data.value(QStringLiteral("columns")).value<QStringList>();
    m_records = data.value(QStringLiteral("records")).value<QVariantList>();
    return true;
}
