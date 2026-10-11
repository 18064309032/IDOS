#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

#include "idoswell.h"
#include "idoswellhead.h"
#include "idoswelllogchannel.h"
#include "idoswelllogset.h"

#include "idoswelllasprovider.h"

IDOSWellLasProvider::CurveDefinition::CurveDefinition()
    : name()
    , unit()
{
}

IDOSWellLasProvider::CurveDefinition::CurveDefinition(const QString& name, const QString& unit)
    : name(name)
    , unit(unit)
{
}

QList<IDOSDataObject*> IDOSWellLasProvider::read(const QString& filePath)
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
    Section section = Section::None;
    QString wellName;
    double surfaceX = 0.0;
    double surfaceY = 0.0;
    double surfaceElevation = 0.0;
    double kb = 0.0;
    double topDepth = 0.0;
    double bottomDepth = 0.0;
    double referenceDepth = 0.0;
    double nullValue = 0.0;
    bool hasSurfaceX = false;
    bool hasSurfaceY = false;
    bool hasSurfaceElevation = false;
    bool hasKb = false;
    bool hasTopDepth = false;
    bool hasBottomDepth = false;
    bool hasReferenceDepth = false;
    bool hasNullValue = false;
    QVector<CurveDefinition> curves;
    QVector<QVector<double>> values;

    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
        {
            continue;
        }
        if (line.startsWith(QLatin1Char('~')))
        {
            section = sectionForLine(line);
            continue;
        }

        if (section == Section::Well)
        {
            QString mnemonic;
            QString unit;
            QString value;
            if (!parseEntry(line, &mnemonic, &unit, &value))
            {
                continue;
            }
            bool validNumber = false;
            double number = 0.0;
            if (mnemonic != QStringLiteral("WELL"))
            {
                validNumber = parseNumber(value, &number);
            }
            if (mnemonic == QStringLiteral("WELL"))
            {
                wellName = value;
            }
            else if (mnemonic == QStringLiteral("XCOORD") && validNumber)
            {
                surfaceX = number;
                hasSurfaceX = true;
            }
            else if (mnemonic == QStringLiteral("YCOORD") && validNumber)
            {
                surfaceY = number;
                hasSurfaceY = true;
            }
            else if (mnemonic == QStringLiteral("ELEV") && validNumber)
            {
                surfaceElevation = number;
                hasSurfaceElevation = true;
            }
            else if (mnemonic == QStringLiteral("KB") && validNumber)
            {
                kb = number;
                hasKb = true;
            }
            else if (mnemonic == QStringLiteral("STRT") && validNumber)
            {
                topDepth = number;
                hasTopDepth = true;
            }
            else if (mnemonic == QStringLiteral("STOP") && validNumber)
            {
                bottomDepth = number;
                hasBottomDepth = true;
            }
            else if (mnemonic == QStringLiteral("DREF") && validNumber)
            {
                referenceDepth = number;
                hasReferenceDepth = true;
            }
            else if (mnemonic == QStringLiteral("NULL") && validNumber)
            {
                nullValue = number;
                hasNullValue = true;
            }
            continue;
        }

        if (section == Section::Curve)
        {
            QString mnemonic;
            QString unit;
            QString value;
            if (parseEntry(line, &mnemonic, &unit, &value))
            {
                curves.append(CurveDefinition(mnemonic, unit));
            }
            continue;
        }

        if (section == Section::Ascii)
        {
            const QStringList fields = splitValues(line);
            if (fields.size() != curves.size())
            {
                setLastError(QObject::tr("Log data row has an invalid column count."));
                return {};
            }
            QVector<double> row;
            row.reserve(fields.size());
            for (const QString& field : fields)
            {
                double number = 0.0;
                if (!parseNumber(field, &number))
                {
                    setLastError(QObject::tr("Log data contains an invalid number."));
                    return {};
                }
                row.append(hasNullValue && number == nullValue ? qQNaN() : number);
            }
            values.append(row);
        }
    }

    if (curves.isEmpty() || values.isEmpty())
    {
        setLastError(QObject::tr("The LAS file contains no curve data."));
        return {};
    }

    int depthColumn = -1;
    for (int column = 0; column < curves.size(); ++column)
    {
        if (curves[column].name == QStringLiteral("DEPT") || curves[column].name == QStringLiteral("DEPTH"))
        {
            depthColumn = column;
            break;
        }
    }
    if (depthColumn < 0)
    {
        setLastError(QObject::tr("The LAS file has no depth curve."));
        return {};
    }

    QVector<double> depths;
    depths.reserve(values.size());
    for (const QVector<double>& row : values)
    {
        depths.append(row[depthColumn]);
    }

    IDOSWellLogSet logs;
    for (int column = 0; column < curves.size(); ++column)
    {
        if (column == depthColumn)
        {
            continue;
        }
        QVector<double> samples;
        samples.reserve(values.size());
        for (const QVector<double>& row : values)
        {
            samples.append(row[column]);
        }
        IDOSWellLogChannel channel(curves[column].name);
        channel.setUnit(curves[column].unit);
        channel.setData(depths, samples);
        logs.addChannel(channel);
    }

    IDOSWell* well = new IDOSWell();
    if (wellName.isEmpty())
    {
        const QFileInfo fileInfo(filePath);
        wellName = fileInfo.completeBaseName();
    }
    well->setName(wellName);
    well->setLogs(logs);

    if (hasSurfaceX || hasSurfaceY || hasSurfaceElevation || hasKb || hasTopDepth || hasBottomDepth)
    {
        IDOSWellHead head;
        if (hasSurfaceX)
        {
            head.setSurfaceX(surfaceX);
        }
        if (hasSurfaceY)
        {
            head.setSurfaceY(surfaceY);
        }
        if (hasSurfaceElevation)
        {
            head.setSurfaceElevation(surfaceElevation);
        }
        if (hasKb)
        {
            head.setKb(kb);
        }
        if (hasTopDepth)
        {
            head.setTopDepth(topDepth);
        }
        if (hasBottomDepth)
        {
            head.setBottomDepth(bottomDepth);
        }
        well->setWellHead(head);
    }
    if (hasReferenceDepth)
    {
        well->setReferenceDepth(referenceDepth);
    }
    return {well};
}

IDOSWellLasProvider::Section IDOSWellLasProvider::sectionForLine(const QString& line) const
{
    const QString sectionName = line.mid(1).trimmed().left(1).toUpper();
    if (sectionName == QStringLiteral("V"))
    {
        return Section::Version;
    }
    if (sectionName == QStringLiteral("W"))
    {
        return Section::Well;
    }
    if (sectionName == QStringLiteral("C"))
    {
        return Section::Curve;
    }
    if (sectionName == QStringLiteral("A"))
    {
        return Section::Ascii;
    }
    return Section::Other;
}

bool IDOSWellLasProvider::parseEntry(const QString& line, QString* mnemonic, QString* unit, QString* value) const
{
    const int colonIndex = line.indexOf(QLatin1Char(':'));
    const QString left = colonIndex >= 0 ? line.left(colonIndex).trimmed() : line.trimmed();
    const QString right = colonIndex >= 0 ? line.mid(colonIndex + 1).trimmed() : QString();
    const QRegularExpression pattern(QStringLiteral("^([^\\s.]+)(?:\\.([^\\s]*))?\\s*(.*)$"));
    const QRegularExpressionMatch match = pattern.match(left);
    if (!match.hasMatch())
    {
        return false;
    }
    *mnemonic = match.captured(1).toUpper();
    *unit = match.captured(2);
    *value = match.captured(3).trimmed();
    if (value->isEmpty())
    {
        *value = right;
    }
    return !mnemonic->isEmpty() && !value->isEmpty();
}

QStringList IDOSWellLasProvider::splitValues(const QString& line) const
{
    return line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
}

bool IDOSWellLasProvider::parseNumber(const QString& text, double* value) const
{
    bool valid = false;
    const double number = text.toDouble(&valid);
    if (!valid || !std::isfinite(number))
    {
        return false;
    }
    *value = number;
    return true;
}
