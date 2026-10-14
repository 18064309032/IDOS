#include <cmath>
#include <limits>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>
#include <QtGlobal>
#include <QVector>

#include "data/grid/idosgridsurface.h"

#include "idosgridsurfaceprovider.h"

bool IDOSGridSurfaceProvider::canRead(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    for (int lineNumber = 0; lineNumber < 30 && !stream.atEnd(); ++lineNumber)
    {
        if (stream.readLine().trimmed().startsWith(QStringLiteral("@Grid HEADER, GRID"), Qt::CaseInsensitive))
        {
            return true;
        }
    }
    return false;
}

QList<IDOSDataObject*> IDOSGridSurfaceProvider::read(const QString& filePath)
{
    setLastError(QString());
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open grid surface file: %1").arg(file.errorString()));
        return {};
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QStringList lines;
    QString coordinateReferenceSystem;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        if (line.startsWith(QStringLiteral("!")))
        {
            const QString prefix = QStringLiteral("! COORDINATE REFERENCE SYSTEM:");
            if (line.startsWith(prefix, Qt::CaseInsensitive))
            {
                coordinateReferenceSystem = line.mid(prefix.size()).trimmed();
            }
            continue;
        }
        if (!line.isEmpty())
        {
            lines.append(line);
        }
    }

    int headerIndex = -1;
    for (int index = 0; index < lines.size(); ++index)
    {
        if (lines.at(index).startsWith(QStringLiteral("@Grid HEADER, GRID"), Qt::CaseInsensitive))
        {
            headerIndex = index;
            break;
        }
    }
    if (headerIndex < 0 || headerIndex + 4 >= lines.size())
    {
        setLastError(QObject::tr("The file does not contain a complete ZIMS grid surface header."));
        return {};
    }

    const QStringList formatFields = lines.at(headerIndex + 1).split(QLatin1Char(','));
    const QStringList dimensionFields = lines.at(headerIndex + 2).split(QLatin1Char(','));
    const QStringList spacingFields = lines.at(headerIndex + 3).split(QLatin1Char(','));
    bool nxValid = false;
    bool nyValid = false;
    bool originXValid = false;
    bool originYValid = false;
    bool spacingXValid = false;
    bool spacingYValid = false;
    bool missingValid = false;
    const int nx = dimensionFields.value(0).trimmed().toInt(&nxValid);
    const int ny = dimensionFields.value(1).trimmed().toInt(&nyValid);
    const double originX = dimensionFields.value(2).trimmed().toDouble(&originXValid);
    const double originY = dimensionFields.value(4).trimmed().toDouble(&originYValid);
    const double spacingX = spacingFields.value(1).trimmed().toDouble(&spacingXValid);
    const double spacingY = spacingFields.value(2).trimmed().toDouble(&spacingYValid);
    const double missingValue = formatFields.value(1).trimmed().toDouble(&missingValid);
    const qint64 valueCount = static_cast<qint64>(nx) * static_cast<qint64>(ny);
    if (!nxValid || !nyValid || !originXValid || !originYValid || !spacingXValid || !spacingYValid ||
        !missingValid || nx <= 0 || ny <= 0 || valueCount <= 0 || valueCount > std::numeric_limits<int>::max() ||
        spacingX == 0.0 || spacingY == 0.0 || headerIndex + 4 >= lines.size() ||
        !lines.at(headerIndex + 4).startsWith(QLatin1Char('@')))
    {
        setLastError(QObject::tr("The ZIMS grid surface dimensions or spacing are invalid."));
        return {};
    }

    QVector<double> values;
    values.reserve(static_cast<int>(valueCount));
    for (int index = headerIndex + 5; index < lines.size(); ++index)
    {
        const QStringList fields = lines.at(index).split(QRegularExpression(QStringLiteral("[,\\s]+")),
                                                          Qt::SkipEmptyParts);
        for (const QString& field : fields)
        {
            bool valueValid = false;
            const double value = field.toDouble(&valueValid);
            if (!valueValid || !std::isfinite(value))
            {
                setLastError(QObject::tr("The ZIMS grid surface contains an invalid value."));
                return {};
            }
            if (values.size() >= valueCount)
            {
                setLastError(QObject::tr("The ZIMS grid surface contains more values than its dimensions."));
                return {};
            }
            values.append(std::abs(value - missingValue) <= std::abs(missingValue) * 1.0e-12
                              ? std::numeric_limits<double>::quiet_NaN() : value);
        }
    }
    if (values.size() != valueCount)
    {
        setLastError(QObject::tr("The ZIMS grid surface contains %1 values; expected %2.")
                         .arg(values.size()).arg(valueCount));
        return {};
    }

    IDOSGridSurface* surface = new IDOSGridSurface();
    surface->setName(QFileInfo(filePath).completeBaseName());
    surface->setSourceFile(filePath);
    surface->setNx(nx);
    surface->setNy(ny);
    surface->setOriginX(originX);
    surface->setOriginY(originY);
    surface->setSpacingX(spacingX);
    surface->setSpacingY(spacingY);
    surface->setMissingValue(missingValue);
    surface->setValues(values);
    surface->setCoordinateReferenceSystem(coordinateReferenceSystem.isEmpty()
                                              ? readCoordinateReferenceSystem(filePath)
                                              : coordinateReferenceSystem);
    return {surface};
}

QString IDOSGridSurfaceProvider::readCoordinateReferenceSystem(const QString& filePath) const
{
    QFile file(filePath + QStringLiteral(".crsmeta.xml"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return QString();
    }
    const QString contents = QString::fromUtf8(file.readAll());
    const QRegularExpression pattern(
        QStringLiteral("IEarlyBoundCoordinateReferenceSystem\\s+name=\"([^\"]+)\""));
    const QRegularExpressionMatch match = pattern.match(contents);
    return match.hasMatch() ? match.captured(1) : QString();
}
