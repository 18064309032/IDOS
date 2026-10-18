#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QTextStream>

#include "data/well/idoswellcheckshot.h"
#include "data/well/idoswellobject.h"

#include "idoswellcheckshotprovider.h"

bool IDOSWellCheckshotProvider::canRead(const QString& filePath) const
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (!suffix.isEmpty() && !QStringList{QStringLiteral("prn"), QStringLiteral("checkshot"), QStringLiteral("cs"),
        QStringLiteral("txt"), QStringLiteral("dat")}.contains(suffix))
    {
        return false;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    for (int index = 0; index < 30 && !stream.atEnd(); ++index)
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) || line.startsWith(QStringLiteral("//")))
        {
            continue;
        }
        bool valid = false;
        const QStringList fields = splitTextFields(line, valid);
        if (!valid || fields.size() != 3)
        {
            continue;
        }
        bool firstValid = false;
        bool secondValid = false;
        fields.at(0).toDouble(&firstValid);
        fields.at(1).toDouble(&secondValid);
        if (firstValid && secondValid)
        {
            bool numericWell = false;
            fields.at(2).toDouble(&numericWell);
            return !numericWell;
        }
        if (fields.at(0).compare(QStringLiteral("TVD"), Qt::CaseInsensitive) == 0 ||
            fields.at(0).compare(QStringLiteral("DEPTH"), Qt::CaseInsensitive) == 0)
        {
            return true;
        }
    }
    return false;
}

QList<IDOSDataObject*> IDOSWellCheckshotProvider::read(const QString& filePath)
{
    setLastError(QString());
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open file: %1").arg(file.errorString()));
        return {};
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QMap<QString, QVector<double>> depths;
    QMap<QString, QVector<double>> times;
    QString depthUnit;
    QString timeUnit;
    QString timeType;
    QString depthType;
    int lineNumber = 0;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) || line.startsWith(QStringLiteral("//")))
        {
            continue;
        }
        bool valid = false;
        const QStringList fields = splitTextFields(line, valid);
        const QString first = fields.first().toUpper();
        if (first == QStringLiteral("UNITS") && valid && fields.size() == 3 && depths.isEmpty())
        {
            depthUnit = fields.at(1).toUpper();
            timeUnit = fields.at(2).toUpper();
            if ((depthUnit != QStringLiteral("M") && depthUnit != QStringLiteral("FT")) ||
                (timeUnit != QStringLiteral("MS") && timeUnit != QStringLiteral("S")))
            {
                setLastError(QObject::tr("Checkshot units must specify M or FT and MS or S."));
                return {};
            }
            continue;
        }
        if ((first == QStringLiteral("TVD") || first == QStringLiteral("DEPTH")) && depths.isEmpty())
        {
            timeType = fields.value(1).toUpper();
            depthType = first;
            if (!valid || fields.size() != 3 || fields.at(2).compare(QStringLiteral("WELL"), Qt::CaseInsensitive) != 0 ||
                (timeType != QStringLiteral("TWT") && timeType != QStringLiteral("OWT") && timeType != QStringLiteral("TIME")))
            {
                setLastError(QObject::tr("Checkshots require Depth, TWT/OWT/Time and Well columns."));
                return {};
            }
            continue;
        }
        bool depthValid = false;
        bool timeValid = false;
        const double depth = fields.value(0).toDouble(&depthValid);
        const double time = fields.value(1).toDouble(&timeValid);
        if (!valid || fields.size() != 3 || fields.at(2).trimmed().isEmpty() || !depthValid || !timeValid ||
            !std::isfinite(depth) || !std::isfinite(time))
        {
            setLastError(QObject::tr("Line %1: invalid checkshot record.").arg(lineNumber));
            return {};
        }
        depths[fields.at(2)].append(depth);
        times[fields.at(2)].append(time);
    }
    QMap<QString, IDOSWellCheckshot> samples;
    for (QMap<QString, QVector<double>>::const_iterator iterator = depths.constBegin(); iterator != depths.constEnd(); ++iterator)
    {
        QMap<double, double> ordered;
        const QVector<double>& sourceDepths = iterator.value();
        const QVector<double> sourceTimes = times.value(iterator.key());
        for (int index = 0; index < sourceDepths.size(); ++index)
        {
            const double depth = sourceDepths.at(index);
            const double time = sourceTimes.at(index);
            if (ordered.contains(depth) && ordered.value(depth) != time)
            {
                setLastError(QObject::tr("Well %1: conflicting times at the same checkshot depth.").arg(iterator.key()));
                return {};
            }
            ordered.insert(depth, time);
        }
        QVector<double> sortedDepths;
        QVector<double> sortedTimes;
        for (QMap<double, double>::const_iterator sample = ordered.constBegin(); sample != ordered.constEnd(); ++sample)
        {
            sortedDepths.append(sample.key());
            sortedTimes.append(sample.value());
        }
        IDOSWellCheckshot checkshot;
        if (!checkshot.setSamples(sortedDepths, sortedTimes))
        {
            setLastError(QObject::tr("Well %1: checkshots require at least two finite, strictly monotonic depth and time samples.").arg(iterator.key()));
            return {};
        }
        checkshot.setUnits(depthUnit, timeUnit);
        checkshot.setTimeType(timeType);
        checkshot.setDepthType(depthType);
        samples.insert(iterator.key(), checkshot);
    }
    if (samples.isEmpty())
    {
        setLastError(QObject::tr("No valid checkshots were found."));
        return {};
    }
    QList<IDOSDataObject*> result;
    for (QMap<QString, IDOSWellCheckshot>::const_iterator iterator = samples.constBegin(); iterator != samples.constEnd(); ++iterator)
    {
        IDOSWellObject* well = new IDOSWellObject();
        well->setName(iterator.key());
        well->setCheckshot(iterator.value());
        result.append(well);
    }
    return result;
}
