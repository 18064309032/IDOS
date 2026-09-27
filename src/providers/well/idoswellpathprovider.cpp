#include "idoswellpathprovider.h"
#include "idoswell.h"
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <cmath>

QList<IDOSDataObject*> IDOSWellPathProvider::read(const QString& filePath)
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
    QString wellName;
    QStringList comments;
    QList<int> columns;
    QVector<IDOSWellPathPoint> points;
    const QStringList required = {QStringLiteral("MD"), QStringLiteral("X"),    QStringLiteral("Y"),
                                  QStringLiteral("Z"),  QStringLiteral("TVD"),  QStringLiteral("DX"),
                                  QStringLiteral("DY"), QStringLiteral("AZIM"), QStringLiteral("INCL")};
    const QRegularExpression namePattern(QStringLiteral("^#\\s*WELL NAME\\s*:\\s*(.*)$"),
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
        if (line.startsWith(QLatin1Char('#')))
        {
            comments.append(line);
            const QRegularExpressionMatch match = namePattern.match(line);
            if (match.hasMatch())
            {
                if (!wellName.isEmpty() || match.captured(1).trimmed().isEmpty())
                {
                    setLastError(QObject::tr("Line %1: empty or repeated WELL NAME.").arg(lineNumber));
                    return {};
                }
                wellName = match.captured(1).trimmed();
            }
            continue;
        }
        const QStringList fields = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (columns.isEmpty())
        {
            const QStringList header =
                line.toUpper().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
            if (header.size() != required.size())
            {
                setLastError(QObject::tr("Line %1: expected MD X Y Z TVD DX DY AZIM INCL columns.").arg(lineNumber));
                return {};
            }
            for (const QString& name : required)
            {
                if (header.count(name) != 1)
                {
                    setLastError(QObject::tr("Missing or repeated header column: %1").arg(name));
                    return {};
                }
                columns.append(header.indexOf(name));
            }
            continue;
        }
        if (fields.size() != required.size())
        {
            setLastError(QObject::tr("Line %1: expected nine trajectory values.").arg(lineNumber));
            return {};
        }
        double values[9] = {};
        for (int i = 0; i < 9; ++i)
        {
            bool ok = false;
            values[i] = fields[columns[i]].toDouble(&ok);
            if (!ok || !std::isfinite(values[i]))
            {
                setLastError(QObject::tr("Line %1: invalid number in %2.").arg(lineNumber).arg(required[i]));
                return {};
            }
        }
        if (!points.isEmpty() && values[0] <= points.last().md())
        {
            setLastError(QObject::tr("Line %1: MD must increase strictly.").arg(lineNumber));
            return {};
        }
        points.append(IDOSWellPathPoint(values[0], values[1], values[2], values[3], values[4], values[5], values[6],
                                        values[7], values[8]));
    }
    if (wellName.isEmpty() || points.size() < 2)
    {
        setLastError(QObject::tr("A trajectory requires WELL NAME and at least two valid points."));
        return {};
    }
    IDOSWellPath path;
    path.setPoints(points);
    path.setSourceComments(comments);
    IDOSWell* well = new IDOSWell();
    well->setName(wellName);
    well->setPath(path);
    return {well};
}
