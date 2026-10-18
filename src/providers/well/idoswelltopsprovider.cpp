#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>

#include "data/well/idoswellmarker.h"
#include "data/well/idoswellmarkerset.h"
#include "data/well/idoswellobject.h"

#include "idoswelltopsprovider.h"

bool IDOSWellTopsProvider::canRead(const QString& filePath) const
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == QStringLiteral("txt") || suffix == QStringLiteral("dat"))
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            const QStringList fields = stream.readLine().trimmed().split(
                QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
            if (fields.size() >= 3 &&
                fields.at(0).compare(QStringLiteral("WELL"), Qt::CaseInsensitive) == 0 &&
                fields.at(1).compare(QStringLiteral("SURFACE"), Qt::CaseInsensitive) == 0 &&
                fields.at(2).compare(QStringLiteral("MD"), Qt::CaseInsensitive) == 0)
            {
                return true;
            }
        }
    }
    if (!suffix.isEmpty() && !QStringList{QStringLiteral("prn"), QStringLiteral("tops"),
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
        if (line.contains(QStringLiteral("Petrel Well Tops"), Qt::CaseInsensitive))
        {
            return true;
        }
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) || line.startsWith(QStringLiteral("//")))
        {
            continue;
        }
        bool valid = false;
        const QStringList fields = splitTextFields(line, valid);
        if (!valid || fields.size() < 3)
        {
            continue;
        }
        bool depthValid = false;
        bool firstNumeric = false;
        bool secondNumeric = false;
        fields.first().toDouble(&firstNumeric);
        fields.at(1).toDouble(&secondNumeric);
        fields.at(2).toDouble(&depthValid);
        return !firstNumeric && !secondNumeric && (depthValid || fields.at(2).compare(QStringLiteral("MD"), Qt::CaseInsensitive) == 0);
    }
    return false;
}

QList<IDOSDataObject*> IDOSWellTopsProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> tabularResult;
    if (readTabularWellTops(filePath, &tabularResult))
    {
        return tabularResult;
    }
    if (!lastError().isEmpty())
    {
        return {};
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open file: %1").arg(file.errorString()));
        return {};
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QMap<QString, IDOSWellMarkerSet> wells;
    QStringList header;
    bool inHeader = false;
    bool blockHeader = false;
    int wellColumn = 0;
    int markerColumn = 1;
    int depthColumn = 2;
    int lineNumber = 0;
    double scale = 1.0;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) || line.startsWith(QStringLiteral("//")))
        {
            continue;
        }
        if (line.compare(QStringLiteral("BEGIN HEADER"), Qt::CaseInsensitive) == 0)
        {
            inHeader = true;
            blockHeader = true;
            header.clear();
            wellColumn = -1;
            markerColumn = -1;
            depthColumn = -1;
            continue;
        }
        if (line.compare(QStringLiteral("END HEADER"), Qt::CaseInsensitive) == 0)
        {
            inHeader = false;
            if (wellColumn < 0 || markerColumn < 0 || depthColumn < 0)
            {
                setLastError(QObject::tr("Well tops require Well, Horizon and Measured Depth columns."));
                return {};
            }
            continue;
        }
        if (inHeader)
        {
            const QString name = line.toLower().remove(QRegularExpression(QStringLiteral("^(string|real|integer)\\s+")))
                .remove(QRegularExpression(QStringLiteral("[^a-z0-9]")));
            const int column = header.size();
            header.append(name);
            if (name == QStringLiteral("well") || name == QStringLiteral("wellname"))
            {
                wellColumn = column;
            }
            if (name == QStringLiteral("horizon") || name == QStringLiteral("horizonname") || name == QStringLiteral("surface"))
            {
                markerColumn = column;
            }
            if (name == QStringLiteral("md") || name == QStringLiteral("measureddepth"))
            {
                depthColumn = column;
            }
            continue;
        }
        bool valid = false;
        const QStringList fields = splitTextFields(line, valid);
        if (fields.first().compare(QStringLiteral("VERSION"), Qt::CaseInsensitive) == 0)
        {
            continue;
        }
        if (fields.first().compare(QStringLiteral("UNITS"), Qt::CaseInsensitive) == 0)
        {
            const QString unit = fields.value(1).toUpper();
            if (!wells.isEmpty() || fields.size() != 2 || (unit != QStringLiteral("M") && unit != QStringLiteral("FT")))
            {
                setLastError(QObject::tr("Depth units must be M or FT."));
                return {};
            }
            scale = unit == QStringLiteral("FT") ? 0.3048 : 1.0;
            continue;
        }
        if (!blockHeader && fields.size() >= 3 && fields.at(2).compare(QStringLiteral("MD"), Qt::CaseInsensitive) == 0)
        {
            continue;
        }
        const int requiredColumn = qMax(wellColumn, qMax(markerColumn, depthColumn));
        bool depthValid = false;
        const double depth = fields.value(depthColumn).toDouble(&depthValid) * scale;
        if (!valid || fields.size() <= requiredColumn || (blockHeader && fields.size() != header.size()) ||
            fields.value(wellColumn).trimmed().isEmpty() || fields.value(markerColumn).trimmed().isEmpty() ||
            !depthValid || !std::isfinite(depth) || depth < 0.0)
        {
            setLastError(QObject::tr("Line %1: invalid well top record.").arg(lineNumber));
            return {};
        }
        wells[fields.at(wellColumn)].addMarker(IDOSWellMarker(fields.at(markerColumn), depth));
    }
    if (inHeader || wells.isEmpty())
    {
        setLastError(QObject::tr("No valid well tops were found or the header is incomplete."));
        return {};
    }
    QList<IDOSDataObject*> result;
    for (QMap<QString, IDOSWellMarkerSet>::const_iterator iterator = wells.constBegin(); iterator != wells.constEnd(); ++iterator)
    {
        IDOSWellObject* well = new IDOSWellObject();
        well->setName(iterator.key());
        well->setMarkers(iterator.value());
        result.append(well);
    }
    return result;
}

bool IDOSWellTopsProvider::readTabularWellTops(const QString& filePath,
                                              QList<IDOSDataObject*>* result)
{
    if (result == nullptr)
    {
        setLastError(QObject::tr("The well tops result destination is unavailable."));
        return false;
    }
    QFileInfo fileInfo(filePath);
    const QString suffix = fileInfo.suffix().toLower();
    if (suffix != QStringLiteral("txt") && suffix != QStringLiteral("dat"))
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
    QStringList firstFields;
    while (!stream.atEnd() && firstFields.isEmpty())
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) ||
            line.startsWith(QStringLiteral("//")))
        {
            continue;
        }
        firstFields = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    }
    if (firstFields.size() < 3 || firstFields.at(0).compare(QStringLiteral("WELL"), Qt::CaseInsensitive) != 0 ||
        firstFields.at(1).compare(QStringLiteral("SURFACE"), Qt::CaseInsensitive) != 0 ||
        firstFields.at(2).compare(QStringLiteral("MD"), Qt::CaseInsensitive) != 0)
    {
        return false;
    }
    QMap<QString, IDOSWellMarkerSet> wells;
    int lineNumber = 1;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty())
        {
            continue;
        }
        const QStringList fields = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        bool depthValid = false;
        const double depth = fields.value(2).toDouble(&depthValid);
        if (fields.size() != 3 || fields.at(0).isEmpty() || fields.at(1).isEmpty() || !depthValid ||
            !std::isfinite(depth) || depth < 0.0)
        {
            setLastError(QObject::tr("Line %1: invalid well top record.").arg(lineNumber));
            return false;
        }
        wells[fields.at(0)].addMarker(IDOSWellMarker(fields.at(1), depth));
    }
    if (wells.isEmpty())
    {
        setLastError(QObject::tr("The file contains no well top records."));
        return false;
    }
    for (QMap<QString, IDOSWellMarkerSet>::const_iterator iterator = wells.constBegin();
         iterator != wells.constEnd(); ++iterator)
    {
        IDOSWellObject* well = new IDOSWellObject();
        well->setName(iterator.key());
        well->setMarkers(iterator.value());
        result->append(well);
    }
    return true;
}
