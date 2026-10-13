#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QTextStream>

#include "idoswell.h"
#include "idoswellcompletion.h"

#include "idoswellcompletionprovider.h"

bool IDOSWellCompletionProvider::canRead(const QString& filePath) const
{
    if (QFileInfo(filePath).suffix().compare(QStringLiteral("ev"), Qt::CaseInsensitive) == 0 ||
        QFileInfo(filePath).suffix().compare(QStringLiteral("perf"), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (!suffix.isEmpty() && suffix != QStringLiteral("txt") && suffix != QStringLiteral("dat"))
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
        bool valid = false;
        const QString line = stream.readLine().trimmed();
        if (line.startsWith(QLatin1Char('#')) || line.startsWith(QStringLiteral("//")))
        {
            continue;
        }
        const QStringList fields = splitTextFields(line, valid);
        if (valid && fields.size() >= 3 && fields.at(1).compare(QStringLiteral("TOP"), Qt::CaseInsensitive) == 0 &&
            fields.at(2).compare(QStringLiteral("BOTTOM"), Qt::CaseInsensitive) == 0)
        {
            return true;
        }
        if (valid && fields.size() >= 4 && fields.at(1).compare(QStringLiteral("perforation"), Qt::CaseInsensitive) == 0)
        {
            return true;
        }
    }
    return false;
}

QList<IDOSDataObject*> IDOSWellCompletionProvider::read(const QString& filePath)
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
    QMap<QString, QList<IDOSWellCompletion>> wells;
    QString currentWell;
    double scale = 1.0;
    bool unitsDeclared = false;
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
        if (!valid)
        {
            setLastError(QObject::tr("Line %1: invalid perforation record.").arg(lineNumber));
            return {};
        }
        const QString keyword = fields.first().toUpper();
        if (keyword == QStringLiteral("UNITS"))
        {
            const QString unit = fields.value(1).toUpper();
            if (!wells.isEmpty() || fields.size() != 2 ||
                (unit != QStringLiteral("FIELD") && unit != QStringLiteral("METRIC") &&
                 unit != QStringLiteral("FT") && unit != QStringLiteral("M")))
            {
                setLastError(QObject::tr("Perforation units must be declared before records as FIELD, METRIC, FT or M."));
                return {};
            }
            scale = unit == QStringLiteral("FIELD") || unit == QStringLiteral("FT") ? 0.3048 : 1.0;
            unitsDeclared = true;
            continue;
        }
        if (keyword == QStringLiteral("WELLNAME"))
        {
            if (fields.size() != 2 || fields.at(1).trimmed().isEmpty())
            {
                setLastError(QObject::tr("Line %1: invalid well name.").arg(lineNumber));
                return {};
            }
            currentWell = fields.at(1);
            continue;
        }
        if (fields.size() >= 3 && fields.at(1).compare(QStringLiteral("TOP"), Qt::CaseInsensitive) == 0 &&
            fields.at(2).compare(QStringLiteral("BOTTOM"), Qt::CaseInsensitive) == 0)
        {
            continue;
        }
        const bool event = !currentWell.isEmpty();
        const QString wellName = event ? currentWell : fields.first();
        const int topColumn = event ? 2 : 1;
        const int bottomColumn = event ? 3 : 2;
        const QString status = event ? QStringLiteral("OPEN") : fields.value(3, QStringLiteral("OPEN")).toUpper();
        bool topValid = false;
        bool bottomValid = false;
        const double top = fields.value(topColumn).toDouble(&topValid) * scale;
        const double bottom = fields.value(bottomColumn).toDouble(&bottomValid) * scale;
        if (wellName.trimmed().isEmpty() || !topValid || !bottomValid || !std::isfinite(top) || !std::isfinite(bottom) ||
            top < 0.0 || bottom <= top || (status != QStringLiteral("OPEN") && status != QStringLiteral("SHUT")) ||
            (!event && (fields.size() < 3 || fields.size() > 4)) ||
            (event && (!unitsDeclared || fields.value(1).compare(QStringLiteral("perforation"), Qt::CaseInsensitive) != 0)))
        {
            setLastError(QObject::tr("Line %1: invalid perforation record.").arg(lineNumber));
            return {};
        }
        IDOSWellCompletion completion(0, status == QStringLiteral("OPEN"), bottom, top);
        if (event)
        {
            completion.setEventDate(fields.first());
        }
        wells[wellName].append(completion);
    }
    if (wells.isEmpty())
    {
        setLastError(QObject::tr("No valid perforations were found."));
        return {};
    }
    QList<IDOSDataObject*> result;
    for (QMap<QString, QList<IDOSWellCompletion>>::const_iterator iterator = wells.constBegin(); iterator != wells.constEnd(); ++iterator)
    {
        IDOSWell* well = new IDOSWell();
        well->setName(iterator.key());
        well->setCompletions(iterator.value());
        result.append(well);
    }
    return result;
}
