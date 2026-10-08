#include <cmath>

#include <QFile>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>

#include "idoswell.h"
#include "idoswellhead.h"

#include "idoswellheaderprovider.h"

QList<IDOSDataObject*> IDOSWellHeaderProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open file: %1").arg(file.errorString()));
        return result;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString header;
    int lineNumber = 0;
    while (!stream.atEnd() && header.isEmpty())
    {
        header = stream.readLine().trimmed();
        ++lineNumber;
    }
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
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
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
        IDOSWell* well = new IDOSWell();
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
