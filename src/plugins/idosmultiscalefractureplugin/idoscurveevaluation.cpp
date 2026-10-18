#include <algorithm>
#include <cmath>

#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QObject>
#include <QTextStream>
#include <QTime>
#include <QVariantMap>

#include "data/case/idossimulationsummaryresultsobject.h"
#include "idosproject.h"

#include "idoscurveevaluation.h"

QString IDOSCurveEvaluation::normalizedUnit(const QString& unit)
{
    return unit.trimmed().toUpper().remove(QLatin1Char(' '));
}

bool IDOSCurveEvaluation::splitRow(const QString& line, QStringList& fields)
{
    fields.clear();
    QString field;
    bool quoted = false;
    bool closed = false;
    for (int index = 0; index < line.size(); ++index)
    {
        const QChar character = line.at(index);
        if (quoted)
        {
            if (character == QLatin1Char('"'))
            {
                if (index + 1 < line.size() && line.at(index + 1) == QLatin1Char('"'))
                {
                    field.append(character);
                    ++index;
                }
                else
                {
                    quoted = false;
                    closed = true;
                }
            }
            else
            {
                field.append(character);
            }
        }
        else if (character == QLatin1Char(','))
        {
            fields.append(field.trimmed());
            field.clear();
            closed = false;
        }
        else if (character == QLatin1Char('"'))
        {
            if (!field.trimmed().isEmpty() || closed)
            {
                return false;
            }
            field.clear();
            quoted = true;
        }
        else
        {
            if (closed && !character.isSpace())
            {
                return false;
            }
            field.append(character);
        }
    }
    fields.append(field.trimmed());
    return !quoted;
}

bool IDOSCurveEvaluation::insertPoint(QMap<double, double>& points, double time, double value, QString& error)
{
    if (!std::isfinite(time) || !std::isfinite(value) || std::abs(time) > 1.0e8)
    {
        error = QObject::tr("Curve values and times must be finite and within the supported calendar range.");
        return false;
    }
    time = std::round(time * 86400000.0) / 86400000.0;
    if (points.contains(time))
    {
        error = QObject::tr("A curve contains duplicate timestamps. Resolve them before evaluation.");
        return false;
    }
    if (points.size() >= 200000)
    {
        error = QObject::tr("A curve exceeds the 200000-point evaluation limit.");
        return false;
    }
    points.insert(time, value);
    return true;
}

bool IDOSCurveEvaluation::readCsv(const QJsonObject& arguments, const QString& prefix,
                                QMap<double, double>& points, QString& error)
{
    const QJsonValue timeValue = arguments.value(prefix + QStringLiteral("TimeColumn"));
    const QJsonValue columnValue = arguments.value(prefix + QStringLiteral("ValueColumn"));
    if (!timeValue.isDouble() || !columnValue.isDouble() || timeValue.toDouble() != timeValue.toInt()
        || columnValue.toDouble() != columnValue.toInt() || timeValue.toInt() < 1 || columnValue.toInt() < 1
        || timeValue.toInt() > 10000 || columnValue.toInt() > 10000 || timeValue.toInt() == columnValue.toInt())
    {
        error = QObject::tr("Select distinct time and value columns using 1-based indices.");
        return false;
    }
    const QString mode = arguments.value(QStringLiteral("timeMode")).toString();
    QDateTime origin = QDateTime::fromString(arguments.value(QStringLiteral("origin")).toString(), Qt::ISODate);
    if ((mode != QStringLiteral("days") && mode != QStringLiteral("calendar"))
        || (mode == QStringLiteral("days") && (!origin.isValid() || origin.timeSpec() != Qt::UTC)))
    {
        error = QObject::tr("Select calendar time or elapsed days with a valid UTC day-zero timestamp.");
        return false;
    }
    QFile file(arguments.value(prefix + QStringLiteral("File")).toString());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        error = QObject::tr("Cannot open curve CSV: %1").arg(file.errorString());
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    int lineNumber = 0;
    bool header = arguments.value(QStringLiteral("header")).toBool(true);
    while (!stream.atEnd())
    {
        const QString line = stream.readLine();
        ++lineNumber;
        if (line.trimmed().isEmpty())
        {
            continue;
        }
        QStringList fields;
        if (!splitRow(line, fields) || fields.size() < std::max(timeValue.toInt(), columnValue.toInt()))
        {
            error = QObject::tr("CSV line %1 has invalid quoting or missing columns.").arg(lineNumber);
            return false;
        }
        if (header)
        {
            header = false;
            continue;
        }
        bool valid = false;
        const double value = fields.at(columnValue.toInt() - 1).toDouble(&valid);
        const QString timeText = fields.at(timeValue.toInt() - 1);
        double time = 0.0;
        bool timeValid = false;
        if (mode == QStringLiteral("days"))
        {
            const double days = timeText.toDouble(&timeValid);
            time = origin.toMSecsSinceEpoch() / 86400000.0 + days;
        }
        else
        {
            QDateTime dateTime = QDateTime::fromString(timeText, Qt::ISODateWithMs);
            if (!dateTime.isValid())
            {
                const QDate date = QDate::fromString(timeText, Qt::ISODate);
                dateTime = QDateTime(date, QTime(0, 0), Qt::UTC);
            }
            else if (dateTime.timeSpec() == Qt::LocalTime)
            {
                dateTime.setTimeSpec(Qt::UTC);
            }
            timeValid = dateTime.isValid();
            time = dateTime.toMSecsSinceEpoch() / 86400000.0;
        }
        if (!valid || !timeValid || !insertPoint(points, time, value, error))
        {
            if (error.isEmpty())
            {
                error = QObject::tr("CSV line %1 has an invalid time or value.").arg(lineNumber);
            }
            return false;
        }
    }
    return true;
}

bool IDOSCurveEvaluation::readSource(IDOSProject* project, const QJsonObject& arguments, const QString& prefix,
                                   QMap<double, double>& points, QString& error)
{
    points.clear();
    const QString id = arguments.value(prefix + QStringLiteral("Id")).toString();
    if (id.isEmpty())
    {
        return readCsv(arguments, prefix, points, error);
    }
    const IDOSSimulationSummaryResultsObject* source = qobject_cast<IDOSSimulationSummaryResultsObject*>(project->object(id));
    const QJsonValue selected = arguments.value(prefix + QStringLiteral("Series"));
    if (source == nullptr || !selected.isDouble() || selected.toDouble() != selected.toInt(-1)
        || selected.toInt(-1) < 0 || selected.toInt() >= source->series().size())
    {
        error = QObject::tr("Select an existing summary result and series.");
        return false;
    }
    const QVariantMap series = source->series().at(selected.toInt());
    if (normalizedUnit(series.value(QStringLiteral("unit")).toString())
        != normalizedUnit(arguments.value(QStringLiteral("unit")).toString()))
    {
        error = QObject::tr("The summary series unit does not match the declared common unit. No conversion is performed.");
        return false;
    }
    const QVector<qint64> times = source->timeValues();
    const QVector<double> values = series.value(QStringLiteral("values")).value<QVector<double>>();
    if (times.size() != values.size())
    {
        error = QObject::tr("Summary timestamps and values have different lengths.");
        return false;
    }
    for (int index = 0; index < times.size(); ++index)
    {
        if (!insertPoint(points, times.at(index) / 86400000.0, values.at(index), error))
        {
            return false;
        }
    }
    return true;
}

bool IDOSCurveEvaluation::evaluate(IDOSProject* project, const QJsonObject& arguments, QJsonObject& result, QString& error)
{
    result = QJsonObject();
    error.clear();
    if (project == nullptr || arguments.value(QStringLiteral("unit")).toString().trimmed().isEmpty()
        || !arguments.value(QStringLiteral("confirmUnits")).toBool())
    {
        error = QObject::tr("Declare a common value unit and confirm matching quantities and units for the selected curves.");
        return false;
    }
    QMap<double, double> simulated;
    QMap<double, double> observed;
    if (!readSource(project, arguments, QStringLiteral("simulated"), simulated, error)
        || !readSource(project, arguments, QStringLiteral("observed"), observed, error))
    {
        return false;
    }
    if (simulated.isEmpty() || observed.isEmpty())
    {
        error = QObject::tr("Both curves must contain at least one sample.");
        return false;
    }
    QJsonArray rows;
    double scale = 0.0;
    double rmse = 0.0;
    double mae = 0.0;
    double mape = 0.0;
    int mapeCount = 0;
    int outside = 0;
    for (QMap<double, double>::const_iterator observation = observed.constBegin(); observation != observed.constEnd(); ++observation)
    {
        if (observation.key() < simulated.firstKey() || observation.key() > simulated.lastKey())
        {
            ++outside;
            continue;
        }
        QMap<double, double>::const_iterator upper = simulated.lowerBound(observation.key());
        double value = upper.value();
        if (upper.key() != observation.key())
        {
            QMap<double, double>::const_iterator lower = upper;
            --lower;
            const double fraction = (observation.key() - lower.key()) / (upper.key() - lower.key());
            value = lower.value() * (1.0 - fraction) + upper.value() * fraction;
        }
        const double difference = value - observation.value();
        if (!std::isfinite(difference))
        {
            error = QObject::tr("A curve metric overflowed. Rescale the input values.");
            return false;
        }
        const double count = rows.size() + 1.0;
        rmse = std::hypot(rmse * std::sqrt((count - 1.0) / count), difference / std::sqrt(count));
        mae = mae * ((count - 1.0) / count) + std::abs(difference) / count;
        if (observation.value() != 0.0)
        {
            ++mapeCount;
            mape = mape * ((mapeCount - 1.0) / mapeCount) + std::abs(difference / observation.value()) / mapeCount;
        }
        scale = std::max(scale, std::max(std::abs(value), std::abs(observation.value())));
        rows.append(QJsonObject{{QStringLiteral("time"), observation.key()}, {QStringLiteral("simulated"), value},
            {QStringLiteral("observed"), observation.value()}, {QStringLiteral("error"), difference}});
    }
    if (rows.isEmpty())
    {
        error = QObject::tr("The curves have no overlapping timestamps. Index-based pairing and extrapolation are not used.");
        return false;
    }
    if (!std::isfinite(rmse) || !std::isfinite(mae) || !std::isfinite(mape * 100.0))
    {
        error = QObject::tr("A curve metric overflowed. Rescale the input values.");
        return false;
    }
    double mean = 0.0;
    for (const QJsonValue& row : rows)
    {
        mean += scale > 0.0 ? row.toObject().value(QStringLiteral("observed")).toDouble() / scale / rows.size() : 0.0;
    }
    double residual = 0.0;
    double total = 0.0;
    bool observationsVary = false;
    const double firstObservation = rows.first().toObject().value(QStringLiteral("observed")).toDouble();
    for (const QJsonValue& row : rows)
    {
        const QJsonObject point = row.toObject();
        observationsVary = observationsVary || point.value(QStringLiteral("observed")).toDouble() != firstObservation;
        const double observation = scale > 0.0 ? point.value(QStringLiteral("observed")).toDouble() / scale : 0.0;
        const double prediction = scale > 0.0 ? point.value(QStringLiteral("simulated")).toDouble() / scale : 0.0;
        residual += (prediction - observation) * (prediction - observation);
        total += (observation - mean) * (observation - mean);
    }
    const double r2 = total > 0.0 ? 1.0 - residual / total : 0.0;
    result = QJsonObject{{QStringLiteral("samples"), rows.size()}, {QStringLiteral("rmse"), rmse},
        {QStringLiteral("mae"), mae}, {QStringLiteral("mapeSamples"), mapeCount},
        {QStringLiteral("mape"), mapeCount > 0 ? QJsonValue(mape * 100.0) : QJsonValue(QJsonValue::Null)},
        {QStringLiteral("r2"), observationsVary && total > 0.0 && std::isfinite(r2) ? QJsonValue(r2) : QJsonValue(QJsonValue::Null)},
        {QStringLiteral("outsideOverlap"), outside}, {QStringLiteral("rows"), rows}, {QStringLiteral("inputs"), arguments}};
    return true;
}
