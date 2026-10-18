#include <cmath>

#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>
#include <QTime>
#include <QVariantMap>

#include "data/production/idosproductionhistoryobject.h"

#include "idosproductionhistoryprovider.h"

bool IDOSProductionHistoryProvider::canRead(const QString& filePath) const
{
    if (QFileInfo(filePath).suffix().compare(QStringLiteral("vol"), Qt::CaseInsensitive) != 0)
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
    return stream.readLine().trimmed() == QStringLiteral("*FIELD");
}

QList<IDOSDataObject*> IDOSProductionHistoryProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open production history file: %1").arg(file.errorString()));
        return result;
    }
    const QStringList expectedColumns{QStringLiteral("DAY"), QStringLiteral("MONTH"),
                                      QStringLiteral("YEAR"), QStringLiteral("HOUR"),
                                      QStringLiteral("MINUTE"), QStringLiteral("SECOND"),
                                      QStringLiteral("ALQ_VALUE"), QStringLiteral("BHP"),
                                      QStringLiteral("GAS"), QStringLiteral("OIL"),
                                      QStringLiteral("WATER"), QStringLiteral("THP")};
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QStringList columns;
    QVariantList records;
    QString currentWell;
    int lineNumber = 0;
    const QRegularExpression whitespacePattern(QStringLiteral("\\s+"));
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty() || line.startsWith(QStringLiteral("--")))
        {
            continue;
        }
        if (line.startsWith(QStringLiteral("*NAME"), Qt::CaseInsensitive))
        {
            currentWell = line.mid(5).trimmed();
            if (currentWell.isEmpty())
            {
                setLastError(QObject::tr("Line %1: production series has an empty well name.").arg(lineNumber));
                return result;
            }
            continue;
        }
        if (line.startsWith(QLatin1Char('*')))
        {
            const QStringList tokens = line.split(whitespacePattern, Qt::SkipEmptyParts);
            QStringList candidateColumns;
            for (const QString& token : tokens)
            {
                if (token.startsWith(QLatin1Char('*')))
                {
                    candidateColumns.append(token.mid(1).toUpper());
                }
            }
            if (candidateColumns.size() == expectedColumns.size())
            {
                if (candidateColumns != expectedColumns)
                {
                    setLastError(QObject::tr("Line %1: unsupported production history columns.").arg(lineNumber));
                    return result;
                }
                columns = candidateColumns;
            }
            continue;
        }
        if (columns.isEmpty() || currentWell.isEmpty())
        {
            continue;
        }
        const QStringList fields = line.split(whitespacePattern, Qt::SkipEmptyParts);
        if (fields.size() != columns.size())
        {
            setLastError(QObject::tr("Line %1: expected %2 production values.")
                             .arg(lineNumber).arg(columns.size()));
            return result;
        }
        int dateParts[6] = {};
        for (int index = 0; index < 6; ++index)
        {
            bool valid = false;
            dateParts[index] = fields.at(index).toInt(&valid);
            if (!valid)
            {
                setLastError(QObject::tr("Line %1: invalid production date or time.").arg(lineNumber));
                return result;
            }
        }
        const QDate date(dateParts[2], dateParts[1], dateParts[0]);
        const QTime time(dateParts[3], dateParts[4], dateParts[5]);
        if (!date.isValid() || !time.isValid())
        {
            setLastError(QObject::tr("Line %1: invalid production date or time.").arg(lineNumber));
            return result;
        }
        QVariantMap record;
        record.insert(QStringLiteral("wellName"), currentWell);
        record.insert(QStringLiteral("time"), QDateTime(date, time));
        for (int index = 0; index < 6; ++index)
        {
            record.insert(columns.at(index), dateParts[index]);
        }
        for (int index = 6; index < fields.size(); ++index)
        {
            bool valid = false;
            const double value = fields.at(index).toDouble(&valid);
            if (!valid || !std::isfinite(value))
            {
                setLastError(QObject::tr("Line %1: invalid production measurement.").arg(lineNumber));
                return result;
            }
            record.insert(columns.at(index), value);
        }
        records.append(record);
    }
    if (columns.isEmpty() || records.isEmpty())
    {
        setLastError(QObject::tr("The VOL file contains no supported production history."));
        return result;
    }
    IDOSProductionHistoryObject* history = new IDOSProductionHistoryObject();
    history->setName(QFileInfo(filePath).completeBaseName());
    history->setSourceFile(filePath);
    history->setColumns(columns);
    history->setRecords(records);
    result.append(history);
    return result;
}
