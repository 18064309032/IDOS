#include <cmath>

#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QTextStream>
#include <QTime>
#include <QVariantMap>
#include <QtGlobal>

#include "data/seismic/idosmicroseismicsetobject.h"

#include "idosmicroseismicprovider.h"

bool IDOSMicroseismicProvider::canRead(const QString& filePath) const
{
    if (QFileInfo(filePath).suffix().compare(QStringLiteral("txt"), Qt::CaseInsensitive) != 0)
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
    const QStringList header = stream.readLine().trimmed().split(QLatin1Char('\t'));
    QStringList normalized;
    for (const QString& column : header)
    {
        normalized.append(column.trimmed().toLower());
    }
    const bool namedCoordinates = normalized.contains(QStringLiteral("east")) &&
        normalized.contains(QStringLiteral("north")) && normalized.contains(QStringLiteral("down"));
    const bool locatedCoordinates = normalized.contains(QStringLiteral("event location x [m]")) &&
        normalized.contains(QStringLiteral("event location y [m]")) &&
        normalized.contains(QStringLiteral("event location z [m]"));
    const bool hasMagnitude = normalized.contains(QStringLiteral("lmag")) ||
        normalized.contains(QStringLiteral("custom_lmag"));
    const bool hasTime = normalized.contains(QStringLiteral("time")) ||
        (normalized.contains(QStringLiteral("eventdate")) && normalized.contains(QStringLiteral("eventtime")));
    return (namedCoordinates || locatedCoordinates) && hasMagnitude && hasTime;
}

QList<IDOSDataObject*> IDOSMicroseismicProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open microseismic file: %1").arg(file.errorString()));
        return result;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    const QStringList header = stream.readLine().trimmed().split(QLatin1Char('\t'));
    QStringList normalized;
    for (const QString& column : header)
    {
        normalized.append(column.trimmed().toLower());
    }
    const int dateIndex = normalized.indexOf(QStringLiteral("eventdate"));
    const int timeIndex = normalized.indexOf(QStringLiteral("eventtime")) >= 0
        ? normalized.indexOf(QStringLiteral("eventtime")) : normalized.indexOf(QStringLiteral("time"));
    int eastIndex = normalized.indexOf(QStringLiteral("east"));
    int northIndex = normalized.indexOf(QStringLiteral("north"));
    int downIndex = normalized.indexOf(QStringLiteral("down"));
    if (eastIndex < 0)
    {
        eastIndex = normalized.indexOf(QStringLiteral("event location x [m]"));
        northIndex = normalized.indexOf(QStringLiteral("event location y [m]"));
        downIndex = normalized.indexOf(QStringLiteral("event location z [m]"));
    }
    const int magnitudeIndex = normalized.indexOf(QStringLiteral("lmag")) >= 0
        ? normalized.indexOf(QStringLiteral("lmag")) : normalized.indexOf(QStringLiteral("custom_lmag"));
    if (dateIndex < 0 && timeIndex < 0)
    {
        setLastError(QObject::tr("The microseismic file has an unsupported column header."));
        return result;
    }
    QVariantList events;
    int lineNumber = 1;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty())
        {
            continue;
        }
        const QStringList fields = line.split(QLatin1Char('\t'));
        const int lastRequiredIndex = qMax(magnitudeIndex, qMax(eastIndex, qMax(northIndex,
            qMax(downIndex, qMax(dateIndex, timeIndex)))));
        if (fields.size() <= lastRequiredIndex)
        {
            setLastError(QObject::tr("Line %1: the event row does not match the column header.").arg(lineNumber));
            return result;
        }
        QVariantMap event;
        if (dateIndex >= 0)
        {
            const QString eventDate = fields.at(dateIndex).trimmed();
            const QString eventTime = fields.at(timeIndex).trimmed();
            if (!QDate::fromString(eventDate, Qt::ISODate).isValid() ||
                !QTime::fromString(eventTime, QStringLiteral("HH:mm:ss")).isValid())
            {
                setLastError(QObject::tr("Line %1: invalid event date or time.").arg(lineNumber));
                return result;
            }
            event.insert(QStringLiteral("eventDate"), eventDate);
            event.insert(QStringLiteral("eventTime"), eventTime);
        }
        else
        {
            bool valid = false;
            const double value = fields.at(timeIndex).trimmed().toDouble(&valid);
            if (!valid || !std::isfinite(value))
            {
                setLastError(QObject::tr("Line %1: invalid event time value.").arg(lineNumber));
                return result;
            }
            event.insert(QStringLiteral("time"), value);
        }
        const QStringList valueNames{QStringLiteral("east"), QStringLiteral("north"),
                                     QStringLiteral("down"), QStringLiteral("magnitude")};
        const QList<int> valueIndexes{eastIndex, northIndex, downIndex, magnitudeIndex};
        for (int index = 0; index < valueIndexes.size(); ++index)
        {
            bool valid = false;
            const double value = fields.at(valueIndexes.at(index)).trimmed().toDouble(&valid);
            if (!valid || !std::isfinite(value))
            {
                setLastError(QObject::tr("Line %1: invalid numeric event value.").arg(lineNumber));
                return result;
            }
            event.insert(valueNames.at(index), value);
        }
        events.append(event);
    }
    if (events.isEmpty())
    {
        setLastError(QObject::tr("The microseismic file contains no events."));
        return result;
    }
    IDOSMicroseismicSetObject* data = new IDOSMicroseismicSetObject();
    data->setName(QFileInfo(filePath).completeBaseName());
    data->setSourceFile(filePath);
    data->setEvents(events);
    result.append(data);
    return result;
}
