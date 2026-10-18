#include <algorithm>

#include <QObject>

#include "idosfracturescheduleobject.h"

IDOSFractureScheduleObject::IDOSFractureScheduleObject(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

QString IDOSFractureScheduleObject::typeId() const
{
    return QStringLiteral("idos.fracture.schedule");
}

QString IDOSFractureScheduleObject::sourceFile() const
{
    return m_sourceFile;
}

void IDOSFractureScheduleObject::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QStringList IDOSFractureScheduleObject::columns() const
{
    return m_columns;
}

void IDOSFractureScheduleObject::setColumns(const QStringList& columns)
{
    if (m_columns.size() == columns.size() && std::equal(m_columns.cbegin(), m_columns.cend(), columns.cbegin()))
    {
        return;
    }
    m_columns = columns;
    emit dataChanged();
}

QStringList IDOSFractureScheduleObject::units() const
{
    return m_units;
}

void IDOSFractureScheduleObject::setUnits(const QStringList& units)
{
    if (m_units.size() == units.size() && std::equal(m_units.cbegin(), m_units.cend(), units.cbegin()))
    {
        return;
    }
    m_units = units;
    emit dataChanged();
}

QVariantList IDOSFractureScheduleObject::samples() const
{
    return m_samples;
}

void IDOSFractureScheduleObject::setSamples(const QVariantList& samples)
{
    if (m_samples == samples)
    {
        return;
    }
    m_samples = samples;
    emit dataChanged();
}

bool IDOSFractureScheduleObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSFractureScheduleObject::clone() const
{
    IDOSFractureScheduleObject* copy = new IDOSFractureScheduleObject();
    copy->setName(name());
    copy->m_sourceFile = m_sourceFile;
    copy->m_columns = m_columns;
    copy->m_units = m_units;
    copy->m_samples = m_samples;
    return copy;
}

QVariantMap IDOSFractureScheduleObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("columns"), QVariant::fromValue(m_columns));
    data.insert(QStringLiteral("units"), QVariant::fromValue(m_units));
    data.insert(QStringLiteral("samples"), QVariant::fromValue(m_samples));
    return data;
}

bool IDOSFractureScheduleObject::restoreProjectData(const QVariantMap& data, QString& error)
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
    if (!data.contains(QStringLiteral("units")) || data.value(QStringLiteral("units")).userType() != qMetaTypeId<QStringList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("units"));
        return false;
    }
    if (!data.contains(QStringLiteral("samples")) || data.value(QStringLiteral("samples")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("samples"));
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_columns = data.value(QStringLiteral("columns")).value<QStringList>();
    m_units = data.value(QStringLiteral("units")).value<QStringList>();
    m_samples = data.value(QStringLiteral("samples")).value<QVariantList>();
    return true;
}
