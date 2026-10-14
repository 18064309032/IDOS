#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>
#include <QVariantMap>

#include "idoswell.h"

#include "idoswellpathprovider.h"

bool IDOSWellPathProvider::canRead(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    if (fileInfo.suffix().compare(QStringLiteral("txt"), Qt::CaseInsensitive) == 0 ||
        fileInfo.suffix().compare(QStringLiteral("dat"), Qt::CaseInsensitive) == 0)
    {
        QFile tabularFile(filePath);
        if (tabularFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream tabularStream(&tabularFile);
            tabularStream.setCodec("UTF-8");
            while (!tabularStream.atEnd())
            {
                const QStringList fields = tabularStream.readLine().trimmed().split(
                    QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
                if (fields.size() >= 3 &&
                    fields.first().compare(QStringLiteral("MD"), Qt::CaseInsensitive) == 0)
                {
                    return fields.at(1).compare(QStringLiteral("INCL"), Qt::CaseInsensitive) == 0 &&
                           fields.at(2).compare(QStringLiteral("AZIM"), Qt::CaseInsensitive) == 0;
                }
            }
        }
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    bool hasWellName = false;
    for (int lineNumber = 0; lineNumber < 100 && !stream.atEnd(); ++lineNumber)
    {
        const QString line = stream.readLine().trimmed();
        const QRegularExpression namePattern(QStringLiteral("^#?\\s*WELL(?:\\s+NAME)?\\s*:\\s*.+$"),
                                             QRegularExpression::CaseInsensitiveOption);
        hasWellName = hasWellName || namePattern.match(line).hasMatch();
        QString headerLine = line;
        if (headerLine.startsWith(QLatin1Char('#')))
        {
            headerLine.remove(0, 1);
        }
        const QStringList fields = headerLine.toUpper().split(QRegularExpression(QStringLiteral("\\s+")),
                                                               Qt::SkipEmptyParts);
        QMap<QString, int> columns;
        if (hasWellName && parseHeader(fields, &columns))
        {
            return true;
        }
    }
    return false;
}

bool IDOSWellPathProvider::parseHeader(const QStringList& fields, QMap<QString, int>* columns) const
{
    if (columns == nullptr)
    {
        return false;
    }
    columns->clear();
    for (int index = 0; index < fields.size(); ++index)
    {
        QString name = fields.at(index).trimmed().toUpper();
        if (name == QStringLiteral("AZIM_TN"))
        {
            name = QStringLiteral("AZIM");
        }
        if (!columns->contains(name))
        {
            columns->insert(name, index);
        }
    }
    const QStringList required{QStringLiteral("MD"), QStringLiteral("X"), QStringLiteral("Y"),
                               QStringLiteral("Z"), QStringLiteral("TVD")};
    for (const QString& name : required)
    {
        if (!columns->contains(name))
        {
            return false;
        }
    }
    return true;
}

bool IDOSWellPathProvider::appendWell(const QString& wellName, const QVector<IDOSWellPathPoint>& points,
                                      const QStringList& comments, QList<IDOSDataObject*>* result) const
{
    if (wellName.trimmed().isEmpty() || points.size() < 2 || result == nullptr)
    {
        return false;
    }
    IDOSWellPath path;
    path.setPoints(points);
    path.setSourceComments(comments);
    path.setSpatialReference(spatialReferenceFromComments(comments));
    IDOSWell* well = new IDOSWell();
    well->setName(wellName.trimmed());
    well->setPath(path);
    result->append(well);
    return true;
}

void IDOSWellPathProvider::clearResult(QList<IDOSDataObject*>* result) const
{
    if (result == nullptr)
    {
        return;
    }
    for (IDOSDataObject* object : *result)
    {
        delete object;
    }
    result->clear();
}

QList<IDOSDataObject*> IDOSWellPathProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> tabularResult;
    if (readTabularTrajectory(filePath, &tabularResult))
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
    QList<IDOSDataObject*> result;
    QString wellName;
    QStringList comments;
    QMap<QString, int> columns;
    QVector<IDOSWellPathPoint> points;
    const QRegularExpression namePattern(QStringLiteral("^#\\s*WELL(?:\\s+NAME)?\\s*:\\s*(.*)$"),
                                         QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression plainNamePattern(QStringLiteral("^WELL(?:\\s+NAME)?\\s*:\\s*(.*)$"),
                                              QRegularExpression::CaseInsensitiveOption);
    int lineNumber = 0;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty())
        {
            continue;
        }
        QRegularExpressionMatch nameMatch = namePattern.match(line);
        if (!nameMatch.hasMatch())
        {
            nameMatch = plainNamePattern.match(line);
        }
        if (nameMatch.hasMatch())
        {
            const QString nextWellName = nameMatch.captured(1).trimmed();
            if (line.startsWith(QStringLiteral("WELL "), Qt::CaseInsensitive) &&
                line.contains(QStringLiteral(" MD INCL AZIM"), Qt::CaseInsensitive))
            {
                const QStringList headerFields = line.split(
                    QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
                wellName = headerFields.value(2);
                columns.insert(QStringLiteral("MD"), 0);
                columns.insert(QStringLiteral("INCL"), 1);
                columns.insert(QStringLiteral("AZIM"), 2);
                continue;
            }
            if (!wellName.isEmpty() && nextWellName.compare(wellName, Qt::CaseInsensitive) != 0)
            {
                if (!appendWell(wellName, points, comments, &result))
                {
                    clearResult(&result);
                    setLastError(QObject::tr("Well %1: a trajectory requires at least two valid points.").arg(wellName));
                    return {};
                }
                points.clear();
                comments.clear();
                columns.clear();
            }
            wellName = nextWellName;
            comments.append(line);
            continue;
        }
        if (line.startsWith(QLatin1Char('#')))
        {
            comments.append(line);
        }
        QString headerLine = line;
        if (headerLine.startsWith(QLatin1Char('#')))
        {
            headerLine.remove(0, 1);
        }
        const QStringList fields = headerLine.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        QMap<QString, int> parsedColumns;
        if (parseHeader(fields, &parsedColumns))
        {
            columns = parsedColumns;
            continue;
        }
        if (columns.isEmpty())
        {
            continue;
        }
        const int requiredEnd = qMax(columns.value(QStringLiteral("MD")),
                                     qMax(columns.value(QStringLiteral("X")),
                                          qMax(columns.value(QStringLiteral("Y")),
                                               qMax(columns.value(QStringLiteral("Z")),
                                                    columns.value(QStringLiteral("TVD"))))));
        if (fields.size() <= requiredEnd)
        {
            if (line.startsWith(QLatin1Char('#')))
            {
                continue;
            }
            clearResult(&result);
            setLastError(QObject::tr("Line %1: trajectory row has too few values.").arg(lineNumber));
            return {};
        }
        QMap<QString, double> values;
        for (QMap<QString, int>::const_iterator iterator = columns.constBegin(); iterator != columns.constEnd(); ++iterator)
        {
            bool ok = false;
            const double value = fields.value(iterator.value()).toDouble(&ok);
            if (!ok || !std::isfinite(value))
            {
                if (iterator.key() == QStringLiteral("DLS") || iterator.key() == QStringLiteral("AZIM_GN"))
                {
                    continue;
                }
                clearResult(&result);
                setLastError(QObject::tr("Line %1: invalid number in %2.").arg(lineNumber).arg(iterator.key()));
                return {};
            }
            values.insert(iterator.key(), value);
        }
        if (!points.isEmpty() && values.value(QStringLiteral("MD")) <= points.last().md())
        {
            clearResult(&result);
            setLastError(QObject::tr("Line %1: MD must increase strictly.").arg(lineNumber));
            return {};
        }
        IDOSWellPathPoint point(values.value(QStringLiteral("MD")), values.value(QStringLiteral("X")),
                                values.value(QStringLiteral("Y")), values.value(QStringLiteral("Z")),
                                values.value(QStringLiteral("TVD")), values.value(QStringLiteral("DX")),
                                values.value(QStringLiteral("DY")), values.value(QStringLiteral("AZIM")),
                                values.value(QStringLiteral("INCL")));
        QVariantMap attributes;
        if (values.contains(QStringLiteral("DLS")))
        {
            attributes.insert(QStringLiteral("DLS"), values.value(QStringLiteral("DLS")));
        }
        if (values.contains(QStringLiteral("AZIM_GN")))
        {
            attributes.insert(QStringLiteral("AZIM_GN"), values.value(QStringLiteral("AZIM_GN")));
        }
        point.setAttributes(attributes);
        points.append(point);
    }
    if (!wellName.isEmpty() && !appendWell(wellName, points, comments, &result))
    {
        clearResult(&result);
        setLastError(QObject::tr("Well %1: a trajectory requires at least two valid points.").arg(wellName));
        return {};
    }
    if (result.isEmpty())
    {
        setLastError(QObject::tr("A trajectory file requires well names, recognized columns, and at least two valid points per well."));
        return {};
    }
    return result;
}

bool IDOSWellPathProvider::readTabularTrajectory(const QString& filePath,
                                                QList<IDOSDataObject*>* result)
{
    if (result == nullptr)
    {
        setLastError(QObject::tr("The trajectory result destination is unavailable."));
        return false;
    }
    QFileInfo fileInfo(filePath);
    const QString suffix = fileInfo.suffix().toLower();
    if (suffix != QStringLiteral("txt") && suffix != QStringLiteral("dat") &&
        suffix != QStringLiteral("dev") && suffix != QStringLiteral("trj"))
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
    QString headerLine;
    QString wellName;
    bool foundHeader = false;
    while (!stream.atEnd() && !foundHeader)
    {
        headerLine = stream.readLine().trimmed();
        const QStringList headerFields = headerLine.split(
            QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (headerFields.size() >= 3 &&
            headerFields.first().compare(QStringLiteral("MD"), Qt::CaseInsensitive) == 0 &&
            headerFields.at(1).compare(QStringLiteral("INCL"), Qt::CaseInsensitive) == 0 &&
            headerFields.at(2).compare(QStringLiteral("AZIM"), Qt::CaseInsensitive) == 0)
        {
            foundHeader = true;
        }
        else if (headerFields.size() == 5 &&
                 headerFields.first().compare(QStringLiteral("WELL"), Qt::CaseInsensitive) == 0 &&
                 headerFields.at(1).compare(QStringLiteral("NAME"), Qt::CaseInsensitive) == 0)
        {
            wellName = headerFields.at(2);
        }
    }
    if (!foundHeader)
    {
        return false;
    }
    if (wellName.isEmpty())
    {
        wellName = fileInfo.completeBaseName();
    }
    QVector<IDOSWellPathPoint> points;
    QVector<double> measuredDepths;
    QVector<double> inclinations;
    QVector<double> azimuths;
    int lineNumber = 0;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
        {
            continue;
        }
        const QStringList row = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        bool mdValid = false;
        bool inclinationValid = false;
        bool azimuthValid = false;
        const double md = row.value(0).toDouble(&mdValid);
        const double inclination = row.value(1).toDouble(&inclinationValid);
        const double azimuth = row.value(2).toDouble(&azimuthValid);
        if (row.size() != 3 || !mdValid || !inclinationValid || !azimuthValid ||
            !std::isfinite(md) || !std::isfinite(inclination) || !std::isfinite(azimuth) ||
            md < 0.0 || inclination < 0.0 || inclination > 180.0 || azimuth < 0.0 || azimuth > 360.0 ||
            (!measuredDepths.isEmpty() && md <= measuredDepths.last()))
        {
            setLastError(QObject::tr("Line %1: invalid MD, inclination, or azimuth trajectory record.")
                             .arg(lineNumber + 1));
            return false;
        }
        measuredDepths.append(md);
        inclinations.append(inclination);
        azimuths.append(azimuth);
    }
    if (measuredDepths.size() < 2)
    {
        setLastError(QObject::tr("A tabular trajectory requires at least two survey stations."));
        return false;
    }
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double tvd = 0.0;
    for (int index = 0; index < measuredDepths.size(); ++index)
    {
        if (index > 0)
        {
            const double radiansPerDegree = 3.14159265358979323846 / 180.0;
            const double firstInclination = inclinations.at(index - 1) * radiansPerDegree;
            const double secondInclination = inclinations.at(index) * radiansPerDegree;
            const double firstAzimuth = azimuths.at(index - 1) * radiansPerDegree;
            const double secondAzimuth = azimuths.at(index) * radiansPerDegree;
            const double cosineDogleg = qBound(-1.0,
                std::cos(firstInclination) * std::cos(secondInclination) +
                std::sin(firstInclination) * std::sin(secondInclination) *
                std::cos(secondAzimuth - firstAzimuth), 1.0);
            const double dogleg = std::acos(cosineDogleg);
            const double ratioFactor = dogleg > 1.0e-10
                ? 2.0 * std::tan(dogleg / 2.0) / dogleg : 1.0;
            const double deltaDepth = measuredDepths.at(index) - measuredDepths.at(index - 1);
            const double deltaX = deltaDepth * ratioFactor * 0.5 *
                (std::sin(firstInclination) * std::sin(firstAzimuth) +
                 std::sin(secondInclination) * std::sin(secondAzimuth));
            const double deltaY = deltaDepth * ratioFactor * 0.5 *
                (std::sin(firstInclination) * std::cos(firstAzimuth) +
                 std::sin(secondInclination) * std::cos(secondAzimuth));
            const double deltaTvd = deltaDepth * ratioFactor * 0.5 *
                (std::cos(firstInclination) + std::cos(secondInclination));
            x += deltaX;
            y += deltaY;
            z -= deltaTvd;
            tvd += deltaTvd;
        }
        points.append(IDOSWellPathPoint(measuredDepths.at(index), x, y, z, tvd, 0.0, 0.0,
                                        azimuths.at(index), inclinations.at(index)));
    }
    IDOSWellPath path;
    path.setPoints(points);
    IDOSWell* well = new IDOSWell();
    well->setName(wellName);
    well->setPath(path);
    result->append(well);
    return true;
}

IDOSWellSpatialReference IDOSWellPathProvider::spatialReferenceFromComments(const QStringList& comments) const
{
    const QRegularExpression declarationPattern(
        QStringLiteral("MD\\s+AND\\s+TVD\\s+ARE\\s+REFERENCED\\s*\\(\\s*=\\s*0\\s*\\)\\s+AT\\s+KB"
                       ".*INCREASE\\s+DOWNWARDS"),
        QRegularExpression::CaseInsensitiveOption);
    for (const QString& comment : comments)
    {
        if (declarationPattern.match(comment).hasMatch())
        {
            IDOSWellSpatialReference reference;
            reference.setVerticalDatum(IDOSWellSpatialReference::VerticalDatum::KellyBushing);
            reference.setDepthDirection(IDOSWellSpatialReference::DepthDirection::PositiveDown);
            return reference;
        }
    }
    return IDOSWellSpatialReference();
}
