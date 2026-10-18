#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

#include "data/well/idoswellhead.h"
#include "data/well/idoswellobject.h"
#include "data/well/idoswellspatialreference.h"

#include "idoswellheaderprovider.h"

QList<IDOSDataObject*> IDOSWellHeaderProvider::read(const QString& filePath)
{
    setLastError(QString());
    QFileInfo fileInfo(filePath);
    if ((fileInfo.suffix().compare(QStringLiteral("txt"), Qt::CaseInsensitive) == 0 ||
         fileInfo.suffix().compare(QStringLiteral("dat"), Qt::CaseInsensitive) == 0) &&
        fileInfo.fileName().compare(QStringLiteral("well header.txt"), Qt::CaseInsensitive) == 0)
    {
        return readWhitespaceHeader(filePath);
    }
    QList<IDOSDataObject*> result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open file: %1").arg(file.errorString()));
        return result;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QStringList lines;
    while (!stream.atEnd())
    {
        lines.append(stream.readLine());
    }

    int firstLineIndex = -1;
    for (int index = 0; index < lines.size(); ++index)
    {
        const QString line = lines.at(index).trimmed();
        if (!line.isEmpty() && !line.startsWith(QLatin1Char('#')))
        {
            firstLineIndex = index;
            break;
        }
    }
    if (firstLineIndex < 0)
    {
        setLastError(QObject::tr("The file contains no well header data."));
        return result;
    }

    const QString firstLine = lines.at(firstLineIndex).trimmed();
    if (firstLine.compare(QStringLiteral("VERSION 1"), Qt::CaseInsensitive) == 0)
    {
        readPetrel(lines, &result);
        if (!lastError().isEmpty())
        {
            qDeleteAll(result);
            result.clear();
        }
        else if (result.isEmpty())
        {
            setLastError(QObject::tr("The file contains no well header data."));
        }
        return result;
    }

    QString header = firstLine;
    int lineNumber = firstLineIndex + 1;
    header.replace(QRegularExpression(QStringLiteral("Bottom\\s+Depth"), QRegularExpression::CaseInsensitiveOption),
                   QStringLiteral("Bottom_Depth"));
    const QStringList columns = header.toLower().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    const QStringList expected = {QStringLiteral("wellname"),     QStringLiteral("x-coord"),
                                  QStringLiteral("y-coord"),      QStringLiteral("top_depth"),
                                  QStringLiteral("bottom_depth"), QStringLiteral("kb"),
                                  QStringLiteral("symbol")};
    if (columns.size() != expected.size())
    {
        setLastError(QObject::tr(
            "Expected seven well header columns: WellName, X-Coord, Y-Coord, Top_Depth, Bottom Depth, KB, Symbol."));
        return result;
    }
    QList<int> indexes;
    for (const QString& name : expected)
    {
        if (columns.count(name) != 1)
        {
            setLastError(QObject::tr("Missing or repeated header column: %1").arg(name));
            return result;
        }
        indexes.append(columns.indexOf(name));
    }
    for (int lineIndex = firstLineIndex + 1; lineIndex < lines.size(); ++lineIndex)
    {
        const QString line = lines.at(lineIndex).trimmed();
        lineNumber = lineIndex + 1;
        if (line.isEmpty())
        {
            continue;
        }
        const QStringList fields = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (fields.size() != 7)
        {
            setLastError(QObject::tr("Line %1: expected seven values.").arg(lineNumber));
            break;
        }
        double values[5] = {};
        bool valid = true;
        for (int i = 0; i < 5; ++i)
        {
            bool ok = false;
            values[i] = fields[indexes[i + 1]].toDouble(&ok);
            if (!ok || !std::isfinite(values[i]))
            {
                setLastError(QObject::tr("Line %1: invalid number in %2.").arg(lineNumber).arg(expected[i + 1]));
                valid = false;
                break;
            }
        }
        if (!valid)
        {
            break;
        }
        bool symbolOk = false;
        const int symbol = fields[indexes[6]].toInt(&symbolOk);
        if (!symbolOk || symbol < 0 || values[2] > values[3])
        {
            setLastError(QObject::tr("Line %1: invalid symbol or top depth exceeds bottom depth.").arg(lineNumber));
            break;
        }
        IDOSWellHead head;
        head.setSurfaceX(values[0]);
        head.setSurfaceY(values[1]);
        head.setTopDepth(values[2]);
        head.setBottomDepth(values[3]);
        head.setKb(values[4]);
        head.setSymbol(symbol);
        IDOSWellObject* well = new IDOSWellObject();
        well->setName(fields[indexes[0]]);
        well->setWellHead(head);
        result.append(well);
    }
    if (!lastError().isEmpty())
    {
        qDeleteAll(result);
        result.clear();
    }
    else if (result.isEmpty())
    {
        setLastError(QObject::tr("The file contains no well header data."));
    }
    return result;
}

QList<IDOSDataObject*> IDOSWellHeaderProvider::readWhitespaceHeader(const QString& filePath)
{
    QList<IDOSDataObject*> result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open file: %1").arg(file.errorString()));
        return result;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    const QStringList header = stream.readLine().trimmed().split(
        QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (header.size() != 6 || header.at(0).compare(QStringLiteral("well"), Qt::CaseInsensitive) != 0 ||
        header.at(1).compare(QStringLiteral("name"), Qt::CaseInsensitive) != 0 ||
        header.at(2).compare(QStringLiteral("x"), Qt::CaseInsensitive) != 0 ||
        header.at(3).compare(QStringLiteral("y"), Qt::CaseInsensitive) != 0 ||
        header.at(4).compare(QStringLiteral("md"), Qt::CaseInsensitive) != 0 ||
        header.at(5).compare(QStringLiteral("kb"), Qt::CaseInsensitive) != 0)
    {
        setLastError(QObject::tr("A tabular well header requires Well Name, X, Y, MD, and KB columns."));
        return result;
    }
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty())
        {
            continue;
        }
        const QStringList fields = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        bool xValid = false;
        bool yValid = false;
        bool mdValid = false;
        bool kbValid = false;
        const double x = fields.value(1).toDouble(&xValid);
        const double y = fields.value(2).toDouble(&yValid);
        const double md = fields.value(3).toDouble(&mdValid);
        const double kb = fields.value(4).toDouble(&kbValid);
        if (fields.size() != 5 || fields.first().isEmpty() || !xValid || !yValid || !mdValid || !kbValid ||
            !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(md) || !std::isfinite(kb) || md < 0.0)
        {
            qDeleteAll(result);
            setLastError(QObject::tr("A tabular well header contains an invalid row."));
            return {};
        }
        IDOSWellHead head;
        head.setSurfaceX(x);
        head.setSurfaceY(y);
        head.setBottomDepth(md);
        head.setKb(kb);
        IDOSWellObject* well = new IDOSWellObject();
        well->setName(fields.first());
        well->setWellHead(head);
        result.append(well);
    }
    return result;
}

bool IDOSWellHeaderProvider::readPetrel(const QStringList& lines, QList<IDOSDataObject*>* result)
{
    if (result == nullptr)
    {
        setLastError(QObject::tr("The Petrel well header destination is unavailable."));
        return false;
    }

    QStringList columns;
    int endHeaderIndex = -1;
    bool inHeader = false;
    for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
    {
        const QString line = lines.at(lineIndex).trimmed();
        if (line.compare(QStringLiteral("BEGIN HEADER"), Qt::CaseInsensitive) == 0)
        {
            inHeader = true;
            continue;
        }
        if (line.compare(QStringLiteral("END HEADER"), Qt::CaseInsensitive) == 0)
        {
            endHeaderIndex = lineIndex;
            break;
        }
        if (!inHeader || line.isEmpty())
        {
            continue;
        }

        QString column = line;
        const int commaIndex = column.indexOf(QLatin1Char(','));
        if (commaIndex >= 0)
        {
            column = column.mid(commaIndex + 1).trimmed();
        }
        column.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
        columns.append(column.toUpper());
    }

    if (columns.isEmpty() || endHeaderIndex < 0)
    {
        setLastError(QObject::tr("The file does not contain a valid Petrel well header."));
        return false;
    }

    const int nameIndex = columns.indexOf(QStringLiteral("NAME"));
    const int surfaceXIndex = columns.indexOf(QStringLiteral("SURFACE X"));
    const int surfaceYIndex = columns.indexOf(QStringLiteral("SURFACE Y"));
    if (nameIndex < 0 || surfaceXIndex < 0 || surfaceYIndex < 0)
    {
        setLastError(QObject::tr("Petrel well header requires Name, Surface X, and Surface Y columns."));
        return false;
    }

    const int symbolIndex = columns.indexOf(QStringLiteral("WELL SYMBOL"));
    const int datumNameIndex = columns.indexOf(QStringLiteral("WELL DATUM NAME"));
    const int datumValueIndex = columns.indexOf(QStringLiteral("WELL DATUM VALUE"));
    const int totalDepthIndex = columns.indexOf(QStringLiteral("TD (MD)"));

    for (int lineIndex = endHeaderIndex + 1; lineIndex < lines.size(); ++lineIndex)
    {
        const QString line = lines.at(lineIndex).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
        {
            continue;
        }

        const QStringList fields = splitQuotedFields(line);

        if (fields.size() < columns.size())
        {
            setLastError(QObject::tr("Line %1: expected at least %2 Petrel well header values.")
                             .arg(lineIndex + 1)
                             .arg(columns.size()));
            return false;
        }

        const QString wellName = fields.at(nameIndex).trimmed();
        bool xOk = false;
        bool yOk = false;
        const double surfaceX = fields.at(surfaceXIndex).toDouble(&xOk);
        const double surfaceY = fields.at(surfaceYIndex).toDouble(&yOk);
        if (wellName.isEmpty() || !xOk || !yOk || !std::isfinite(surfaceX) || !std::isfinite(surfaceY))
        {
            setLastError(QObject::tr("Line %1: invalid well name or surface coordinates.").arg(lineIndex + 1));
            return false;
        }

        IDOSWellHead head;
        head.setSurfaceX(surfaceX);
        head.setSurfaceY(surfaceY);

        if (symbolIndex >= 0 && !fields.at(symbolIndex).trimmed().isEmpty())
        {
            bool symbolOk = false;
            const int symbol = fields.at(symbolIndex).toInt(&symbolOk);
            if (!symbolOk || symbol < 0)
            {
                setLastError(QObject::tr("Line %1: invalid well symbol.").arg(lineIndex + 1));
                return false;
            }
            head.setSymbol(symbol);
        }

        const QString datumName = datumNameIndex >= 0 ? fields.at(datumNameIndex).trimmed().toUpper() : QString();
        if (datumValueIndex >= 0)
        {
            bool datumValueOk = false;
            const double datumValue = fields.at(datumValueIndex).toDouble(&datumValueOk);
            if (datumValueOk && std::isfinite(datumValue) && datumValue != -999.0)
            {
                IDOSWellSpatialReference spatialReference;
                if (datumName.contains(QStringLiteral("KB")))
                {
                    head.setKb(datumValue);
                    head.setSurfaceElevation(datumValue);
                    spatialReference.setVerticalDatum(IDOSWellSpatialReference::VerticalDatum::KellyBushing);
                }
                else if (datumName.contains(QStringLiteral("MSL")) ||
                         datumName.contains(QStringLiteral("MEAN SEA LEVEL")))
                {
                    head.setSurfaceElevation(datumValue);
                    spatialReference.setVerticalDatum(IDOSWellSpatialReference::VerticalDatum::MeanSeaLevel);
                }
                else if (datumName.contains(QStringLiteral("TVDSS")) ||
                         datumName.contains(QStringLiteral("TRUE VERTICAL DEPTH SUBSEA")))
                {
                    head.setSurfaceElevation(datumValue);
                    spatialReference.setVerticalDatum(
                        IDOSWellSpatialReference::VerticalDatum::TrueVerticalDepthSubsea);
                }
                if (spatialReference.verticalDatum() != IDOSWellSpatialReference::VerticalDatum::Unknown)
                {
                    spatialReference.setDepthDirection(IDOSWellSpatialReference::DepthDirection::PositiveDown);
                    head.setSpatialReference(spatialReference);
                }
            }
        }

        if (totalDepthIndex >= 0)
        {
            bool depthOk = false;
            const double totalDepth = fields.at(totalDepthIndex).toDouble(&depthOk);
            if (depthOk && std::isfinite(totalDepth) && totalDepth > 0.0 && totalDepth != -999.0)
            {
                head.setBottomDepth(totalDepth);
            }
        }

        IDOSWellObject* well = new IDOSWellObject();
        well->setName(wellName);
        well->setWellHead(head);
        result->append(well);
    }

    return true;
}

QStringList IDOSWellHeaderProvider::splitQuotedFields(const QString& line) const
{
    QStringList fields;
    const QRegularExpression pattern(QStringLiteral("\"(?:\"\"|[^\"])*\"|[^\\s]+"));
    QRegularExpressionMatchIterator iterator = pattern.globalMatch(line);
    while (iterator.hasNext())
    {
        QString field = iterator.next().captured(0);
        if (field.startsWith(QLatin1Char('"')) && field.endsWith(QLatin1Char('"')))
        {
            field = field.mid(1, field.size() - 2);
            field.replace(QStringLiteral("\"\""), QStringLiteral("\""));
        }
        fields.append(field);
    }
    return fields;
}
