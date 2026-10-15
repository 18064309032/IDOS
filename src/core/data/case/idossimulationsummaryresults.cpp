#include <QObject>
#include <QVariant>
#include <QVariantList>

#include "idossimulationsummaryresults.h"

IDOSSimulationSummaryResults::IDOSSimulationSummaryResults(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

QString IDOSSimulationSummaryResults::typeId() const
{
    return QStringLiteral("idos.case.summaryResults");
}

QVariantMap IDOSSimulationSummaryResults::projectData() const
{
    QVariantMap data;
    QVariantList values;
    for (const qint64 value : m_timeValues)
    {
        values.append(value);
    }
    data.insert(QStringLiteral("sourceFile"), m_sourceFile);
    data.insert(QStringLiteral("timeValues"), values);
    QVariantList series;
    for (const QVariantMap& item : m_series)
    {
        series.append(item);
    }
    data.insert(QStringLiteral("series"), series);
    return data;
}

bool IDOSSimulationSummaryResults::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>() ||
        !data.contains(QStringLiteral("timeValues")) || data.value(QStringLiteral("timeValues")).userType() != qMetaTypeId<QVariantList>() ||
        !data.contains(QStringLiteral("series")) || data.value(QStringLiteral("series")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("Stored summary results have missing or invalid fields.");
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).toString();
    m_timeValues.clear();
    const QVariantList times = data.value(QStringLiteral("timeValues")).toList();
    for (const QVariant& value : times)
    {
        bool valid = false;
        const qint64 timeValue = value.toLongLong(&valid);
        if (!valid)
        {
            error = QObject::tr("Stored summary results contain an invalid time value.");
            return false;
        }
        m_timeValues.append(timeValue);
    }
    m_series.clear();
    const QVariantList series = data.value(QStringLiteral("series")).toList();
    for (const QVariant& value : series)
    {
        if (value.userType() != qMetaTypeId<QVariantMap>())
        {
            error = QObject::tr("Stored summary results contain an invalid series.");
            return false;
        }
        m_series.append(value.toMap());
    }
    return true;
}

QString IDOSSimulationSummaryResults::sourceFile() const
{
    return m_sourceFile;
}

void IDOSSimulationSummaryResults::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QVector<qint64> IDOSSimulationSummaryResults::timeValues() const
{
    return m_timeValues;
}

void IDOSSimulationSummaryResults::setTimeValues(const QVector<qint64>& timeValues)
{
    m_timeValues = timeValues;
    emit dataChanged();
}

QList<QVariantMap> IDOSSimulationSummaryResults::series() const
{
    return m_series;
}

void IDOSSimulationSummaryResults::setSeries(const QList<QVariantMap>& series)
{
    m_series = series;
    emit dataChanged();
}
