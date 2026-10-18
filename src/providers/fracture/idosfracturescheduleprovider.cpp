#include <cmath>
#include <xlsxdocument.h>

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QTextStream>
#include <QVariantMap>

#include "data/fracture/idosfracturescheduleobject.h"

#include "idosfracturescheduleprovider.h"

bool IDOSFractureScheduleProvider::canRead(const QString& filePath) const
{
    if (QFileInfo(filePath).suffix().compare(QStringLiteral("xlsx"), Qt::CaseInsensitive) == 0)
    {
        return canReadSpreadsheet(filePath);
    }
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
    return header == QStringList{QStringLiteral("Time"), QStringLiteral("Treating_Pressure"),
                                 QStringLiteral("Rate"), QStringLiteral("Prop_Conc"),
                                 QStringLiteral("Bottom_Prop_Conc")};
}

bool IDOSFractureScheduleProvider::canReadSpreadsheet(const QString& filePath) const
{
    QXlsx::Document document(filePath);
    if (!document.load())
    {
        return false;
    }
    const QStringList sheetNames = document.sheetNames();
    for (const QString& sheetName : sheetNames)
    {
        if (!document.selectSheet(sheetName))
        {
            continue;
        }
        const QXlsx::CellRange range = document.dimension();
        if (!range.isValid())
        {
            continue;
        }
        QStringList headers;
        for (int column = range.firstColumn(); column <= range.lastColumn(); ++column)
        {
            headers.append(document.read(range.firstRow(), column).toString().trimmed());
        }
        if (supportsSpreadsheetColumns(headers))
        {
            return true;
        }
    }
    return false;
}

bool IDOSFractureScheduleProvider::supportsSpreadsheetColumns(const QStringList& columns) const
{
    const bool standardSchedule =
        columns.contains(QStringLiteral("Pump rate (m3/min)"), Qt::CaseInsensitive) &&
        columns.contains(QStringLiteral("Fluid name"), Qt::CaseInsensitive) &&
        columns.contains(QStringLiteral("Fluid volume (m3)"), Qt::CaseInsensitive);
    const bool operationBulletin =
        columns.contains(QStringLiteral("当前时间"), Qt::CaseInsensitive) &&
        columns.contains(QStringLiteral("套压"), Qt::CaseInsensitive) &&
        columns.contains(QStringLiteral("排出排量"), Qt::CaseInsensitive) &&
        columns.contains(QStringLiteral("合砂浓度"), Qt::CaseInsensitive);
    return standardSchedule || operationBulletin;
}

QList<IDOSDataObject*> IDOSFractureScheduleProvider::readSpreadsheet(const QString& filePath)
{
    QList<IDOSDataObject*> result;
    QXlsx::Document document(filePath);
    if (!document.load())
    {
        setLastError(QObject::tr("Cannot open fracture schedule workbook."));
        return result;
    }
    const QStringList sheetNames = document.sheetNames();
    for (const QString& sheetName : sheetNames)
    {
        if (!document.selectSheet(sheetName))
        {
            continue;
        }
        const QXlsx::CellRange range = document.dimension();
        if (!range.isValid())
        {
            continue;
        }
        QStringList columns;
        for (int column = range.firstColumn(); column <= range.lastColumn(); ++column)
        {
            columns.append(document.read(range.firstRow(), column).toString().trimmed());
        }
        if (!supportsSpreadsheetColumns(columns))
        {
            continue;
        }
        const int timeColumn = columns.indexOf(QStringLiteral("当前时间"));
        QVariantList samples;
        for (int row = range.firstRow() + 1; row <= range.lastRow(); ++row)
        {
            if (timeColumn >= 0)
            {
                const QVariant timeValue = document.read(row, range.firstColumn() + timeColumn);
                bool timeIsNumeric = false;
                timeValue.toDouble(&timeIsNumeric);
                if (!timeValue.toDateTime().isValid() && !timeIsNumeric)
                {
                    continue;
                }
            }
            QVariantMap sample;
            bool hasValue = false;
            for (int index = 0; index < columns.size(); ++index)
            {
                const QVariant value = document.read(row, range.firstColumn() + index);
                sample.insert(columns.at(index), value);
                hasValue = hasValue || (value.isValid() && !value.toString().trimmed().isEmpty());
            }
            if (hasValue)
            {
                samples.append(sample);
            }
        }
        if (samples.isEmpty())
        {
            continue;
        }
        IDOSFractureScheduleObject* data = new IDOSFractureScheduleObject();
        data->setName(QFileInfo(filePath).completeBaseName() + QStringLiteral(" - ") + sheetName);
        data->setSourceFile(filePath);
        data->setColumns(columns);
        data->setSamples(samples);
        result.append(data);
    }
    if (result.isEmpty())
    {
        setLastError(QObject::tr("The workbook contains no supported pump schedule sheets."));
    }
    return result;
}

QList<IDOSDataObject*> IDOSFractureScheduleProvider::read(const QString& filePath)
{
    setLastError(QString());
    if (QFileInfo(filePath).suffix().compare(QStringLiteral("xlsx"), Qt::CaseInsensitive) == 0)
    {
        return readSpreadsheet(filePath);
    }
    QList<IDOSDataObject*> result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open fracture schedule: %1").arg(file.errorString()));
        return result;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    const QStringList columns = stream.readLine().trimmed().split(QLatin1Char('\t'));
    const QStringList expected{QStringLiteral("Time"), QStringLiteral("Treating_Pressure"),
                               QStringLiteral("Rate"), QStringLiteral("Prop_Conc"),
                               QStringLiteral("Bottom_Prop_Conc")};
    if (columns != expected)
    {
        setLastError(QObject::tr("The fracture schedule has an unsupported column header."));
        return result;
    }
    const QStringList units = stream.readLine().trimmed().split(QLatin1Char('\t'));
    if (units.size() != expected.size())
    {
        setLastError(QObject::tr("The fracture schedule has an invalid units row."));
        return result;
    }
    QVariantList samples;
    int lineNumber = 2;
    QDateTime previousTime;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty())
        {
            continue;
        }
        const QStringList fields = line.split(QLatin1Char('\t'));
        if (fields.size() != expected.size())
        {
            setLastError(QObject::tr("Line %1: expected five tab-separated values.").arg(lineNumber));
            return result;
        }
        const QDateTime time = QDateTime::fromString(fields.at(0).trimmed(),
                                                      QStringLiteral("MM/dd/yyyy HH:mm:ss"));
        if (!time.isValid() || (!previousTime.isNull() && time < previousTime))
        {
            setLastError(QObject::tr("Line %1: invalid or decreasing sample time.").arg(lineNumber));
            return result;
        }
        previousTime = time;
        QVariantMap sample;
        sample.insert(QStringLiteral("time"), time);
        const QStringList valueNames{QStringLiteral("treatingPressure"), QStringLiteral("rate"),
                                     QStringLiteral("proppantConcentration"),
                                     QStringLiteral("bottomProppantConcentration")};
        for (int index = 0; index < valueNames.size(); ++index)
        {
            bool valid = false;
            const double value = fields.at(index + 1).trimmed().toDouble(&valid);
            if (!valid || !std::isfinite(value))
            {
                setLastError(QObject::tr("Line %1: invalid numeric sample value.").arg(lineNumber));
                return result;
            }
            sample.insert(valueNames.at(index), value);
        }
        samples.append(sample);
    }
    if (samples.isEmpty())
    {
        setLastError(QObject::tr("The fracture schedule contains no samples."));
        return result;
    }
    IDOSFractureScheduleObject* data = new IDOSFractureScheduleObject();
    data->setName(QFileInfo(filePath).completeBaseName());
    data->setSourceFile(filePath);
    data->setColumns(columns);
    data->setUnits(units);
    data->setSamples(samples);
    result.append(data);
    return result;
}
