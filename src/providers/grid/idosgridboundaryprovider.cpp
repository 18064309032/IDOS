#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>
#include <QVector>

#include "idosgridboundary.h"

#include "idosgridboundaryprovider.h"

bool IDOSGridBoundaryProvider::canRead(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    int validRows = 0;
    while (!stream.atEnd() && validRows < 3)
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
        {
            continue;
        }
        const QStringList fields = line.split(QRegularExpression(QStringLiteral("[,\\s]+")),
                                               Qt::SkipEmptyParts);
        if (fields.size() != 4)
        {
            return false;
        }
        bool xValid = false;
        bool yValid = false;
        bool groupValid = false;
        bool zValid = false;
        fields.at(0).toDouble(&xValid);
        fields.at(1).toDouble(&yValid);
        fields.at(2).toInt(&groupValid);
        fields.at(3).toDouble(&zValid);
        if (!xValid || !yValid || !groupValid || !zValid)
        {
            return false;
        }
        ++validRows;
    }
    return validRows == 3;
}

QList<IDOSDataObject*> IDOSGridBoundaryProvider::read(const QString& filePath)
{
    setLastError(QString());
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open grid boundary file: %1").arg(file.errorString()));
        return {};
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QVector<double> xCoordinates;
    QVector<double> yCoordinates;
    QVector<double> zCoordinates;
    QVector<int> groupIds;
    int lineNumber = 0;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
        {
            continue;
        }
        const QStringList fields = line.split(QRegularExpression(QStringLiteral("[,\\s]+")),
                                               Qt::SkipEmptyParts);
        bool xValid = false;
        bool yValid = false;
        bool groupValid = false;
        bool zValid = false;
        const double x = fields.value(0).toDouble(&xValid);
        const double y = fields.value(1).toDouble(&yValid);
        const int groupId = fields.value(2).toInt(&groupValid);
        const double z = fields.value(3).toDouble(&zValid);
        if (fields.size() != 4 || !xValid || !yValid || !groupValid || !zValid ||
            !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        {
            setLastError(QObject::tr("Line %1: invalid grid boundary coordinate row.").arg(lineNumber));
            return {};
        }
        xCoordinates.append(x);
        yCoordinates.append(y);
        zCoordinates.append(z);
        groupIds.append(groupId);
    }
    if (xCoordinates.size() < 2)
    {
        setLastError(QObject::tr("The grid boundary file contains fewer than two points."));
        return {};
    }

    IDOSGridBoundary* boundary = new IDOSGridBoundary();
    boundary->setName(QFileInfo(filePath).completeBaseName());
    boundary->setSourceFile(filePath);
    boundary->setXCoordinates(xCoordinates);
    boundary->setYCoordinates(yCoordinates);
    boundary->setZCoordinates(zCoordinates);
    boundary->setGroupIds(groupIds);
    boundary->setCoordinateReferenceSystem(readCoordinateReferenceSystem(filePath));
    return {boundary};
}

QString IDOSGridBoundaryProvider::readCoordinateReferenceSystem(const QString& filePath) const
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

