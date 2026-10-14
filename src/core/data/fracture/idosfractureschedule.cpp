#include "idosfractureschedule.h"

IDOSFractureSchedule::IDOSFractureSchedule(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

QString IDOSFractureSchedule::typeId() const
{
    return QStringLiteral("idos.fracture.schedule");
}

QString IDOSFractureSchedule::sourceFile() const
{
    return m_sourceFile;
}

void IDOSFractureSchedule::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QStringList IDOSFractureSchedule::columns() const
{
    return m_columns;
}

void IDOSFractureSchedule::setColumns(const QStringList& columns)
{
    if (m_columns == columns)
    {
        return;
    }
    m_columns = columns;
    emit dataChanged();
}

QStringList IDOSFractureSchedule::units() const
{
    return m_units;
}

void IDOSFractureSchedule::setUnits(const QStringList& units)
{
    if (m_units == units)
    {
        return;
    }
    m_units = units;
    emit dataChanged();
}

QVariantList IDOSFractureSchedule::samples() const
{
    return m_samples;
}

void IDOSFractureSchedule::setSamples(const QVariantList& samples)
{
    if (m_samples == samples)
    {
        return;
    }
    m_samples = samples;
    emit dataChanged();
}
