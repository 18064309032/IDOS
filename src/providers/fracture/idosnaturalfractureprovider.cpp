#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>
#include <QVariantMap>

#include "data/fracture/idosnaturalfractureset.h"

#include "idosnaturalfractureprovider.h"

bool IDOSNaturalFractureProvider::canRead(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    return stream.readLine().trimmed() == QStringLiteral("BEGIN FORMAT");
}

QList<IDOSDataObject*> IDOSNaturalFractureProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open natural fracture file: %1").arg(file.errorString()));
        return result;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QStringList lines;
    while (!stream.atEnd())
    {
        lines.append(stream.readLine().trimmed());
    }
    int lineIndex = 0;
    if (lines.isEmpty() || lines.at(lineIndex++) != QStringLiteral("BEGIN FORMAT"))
    {
        setLastError(QObject::tr("The natural fracture file has an invalid format header."));
        return result;
    }
    QString lengthUnit;
    double coordinateScale = 1.0;
    int expectedFractures = -1;
    int expectedProperties = -1;
    const QRegularExpression assignmentPattern(QStringLiteral("^([^=]+)=\\s*(.*)$"));
    while (lineIndex < lines.size() && lines.at(lineIndex) != QStringLiteral("END FORMAT"))
    {
        const QRegularExpressionMatch match = assignmentPattern.match(lines.at(lineIndex));
        if (match.hasMatch())
        {
            const QString key = match.captured(1).trimmed();
            const QString value = match.captured(2).trimmed();
            if (key == QStringLiteral("Format") && value.compare(QStringLiteral("Ascii"), Qt::CaseInsensitive) != 0)
            {
                setLastError(QObject::tr("Only ASCII natural fracture files are supported."));
                return result;
            }
            if (key == QStringLiteral("Length_Unit"))
            {
                lengthUnit = value;
            }
            else if (key == QStringLiteral("Scale"))
            {
                bool valid = false;
                coordinateScale = value.toDouble(&valid);
                if (!valid || !std::isfinite(coordinateScale) || coordinateScale <= 0.0)
                {
                    setLastError(QObject::tr("The natural fracture coordinate scale is invalid."));
                    return result;
                }
            }
            else if (key == QStringLiteral("No_Fractures"))
            {
                bool valid = false;
                expectedFractures = value.toInt(&valid);
                if (!valid || expectedFractures < 0)
                {
                    setLastError(QObject::tr("The natural fracture count is invalid."));
                    return result;
                }
            }
            else if (key == QStringLiteral("No_Properties"))
            {
                bool valid = false;
                expectedProperties = value.toInt(&valid);
                if (!valid || expectedProperties < 0)
                {
                    setLastError(QObject::tr("The natural fracture property count is invalid."));
                    return result;
                }
            }
        }
        ++lineIndex;
    }
    if (lineIndex >= lines.size() || expectedFractures < 0 || expectedProperties < 0)
    {
        setLastError(QObject::tr("The natural fracture format section is incomplete."));
        return result;
    }
    ++lineIndex;

    QStringList propertyNames;
    while (lineIndex < lines.size() && lines.at(lineIndex) != QStringLiteral("END PROPERTIES"))
    {
        const QRegularExpressionMatch match = assignmentPattern.match(lines.at(lineIndex));
        if (match.hasMatch())
        {
            const QString propertyDefinition = match.captured(2);
            const int firstQuote = propertyDefinition.indexOf(QLatin1Char('"'));
            const int lastQuote = propertyDefinition.lastIndexOf(QLatin1Char('"'));
            if (firstQuote >= 0 && lastQuote > firstQuote)
            {
                propertyNames.append(propertyDefinition.mid(firstQuote + 1,
                                                            lastQuote - firstQuote - 1));
            }
        }
        ++lineIndex;
    }
    if (propertyNames.size() != expectedProperties || lineIndex >= lines.size())
    {
        setLastError(QObject::tr("The natural fracture property definitions do not match the format header."));
        return result;
    }
    while (lineIndex < lines.size() && lines.at(lineIndex) != QStringLiteral("BEGIN FRACTURE"))
    {
        ++lineIndex;
    }
    if (lineIndex >= lines.size())
    {
        setLastError(QObject::tr("The natural fracture geometry section is missing."));
        return result;
    }
    ++lineIndex;
    QVariantList fractures;
    const QRegularExpression whitespacePattern(QStringLiteral("\\s+"));
    while (lineIndex < lines.size() && lines.at(lineIndex) != QStringLiteral("END FRACTURE"))
    {
        if (lines.at(lineIndex).isEmpty())
        {
            ++lineIndex;
            continue;
        }
        const QStringList header = lines.at(lineIndex).split(whitespacePattern, Qt::SkipEmptyParts);
        ++lineIndex;
        if (header.size() != expectedProperties + 3)
        {
            setLastError(QObject::tr("Line %1: invalid natural fracture record.").arg(lineIndex));
            return result;
        }
        bool fractureIdValid = false;
        bool vertexCountValid = false;
        bool setIdValid = false;
        const int fractureId = header.at(0).toInt(&fractureIdValid);
        const int vertexCount = header.at(1).toInt(&vertexCountValid);
        const int setId = header.at(2).toInt(&setIdValid);
        if (!fractureIdValid || !vertexCountValid || !setIdValid || vertexCount < 3)
        {
            setLastError(QObject::tr("Line %1: invalid fracture ID, vertex count, or set ID.").arg(lineIndex));
            return result;
        }
        QVariantMap fracture;
        fracture.insert(QStringLiteral("id"), fractureId);
        fracture.insert(QStringLiteral("setId"), setId);
        QVariantMap properties;
        for (int index = 0; index < expectedProperties; ++index)
        {
            bool valid = false;
            const double value = header.at(index + 3).toDouble(&valid);
            if (!valid || !std::isfinite(value))
            {
                setLastError(QObject::tr("Line %1: invalid fracture property value.").arg(lineIndex));
                return result;
            }
            properties.insert(propertyNames.at(index), value);
        }
        fracture.insert(QStringLiteral("properties"), properties);
        QVariantList vertices;
        for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            if (lineIndex >= lines.size())
            {
                setLastError(QObject::tr("The natural fracture geometry is truncated."));
                return result;
            }
            const QStringList values = lines.at(lineIndex).split(whitespacePattern, Qt::SkipEmptyParts);
            ++lineIndex;
            if (values.size() != 4)
            {
                setLastError(QObject::tr("Line %1: expected a vertex index and three coordinates.").arg(lineIndex));
                return result;
            }
            QVariantMap vertex;
            const QStringList coordinateNames{QStringLiteral("east"), QStringLiteral("north"),
                                              QStringLiteral("down")};
            for (int coordinateIndex = 0; coordinateIndex < coordinateNames.size(); ++coordinateIndex)
            {
                bool valid = false;
                const double value = values.at(coordinateIndex + 1).toDouble(&valid);
                if (!valid || !std::isfinite(value))
                {
                    setLastError(QObject::tr("Line %1: invalid fracture coordinate.").arg(lineIndex));
                    return result;
                }
                vertex.insert(coordinateNames.at(coordinateIndex), value);
            }
            vertices.append(vertex);
        }
        if (lineIndex >= lines.size())
        {
            setLastError(QObject::tr("The natural fracture geometry is truncated."));
            return result;
        }
        const QStringList normalValues = lines.at(lineIndex).split(whitespacePattern, Qt::SkipEmptyParts);
        ++lineIndex;
        if (normalValues.size() != 4)
        {
            setLastError(QObject::tr("Line %1: expected a fracture normal vector.").arg(lineIndex));
            return result;
        }
        QVariantList normal;
        for (int index = 1; index < normalValues.size(); ++index)
        {
            bool valid = false;
            const double value = normalValues.at(index).toDouble(&valid);
            if (!valid || !std::isfinite(value))
            {
                setLastError(QObject::tr("Line %1: invalid fracture normal vector.").arg(lineIndex));
                return result;
            }
            normal.append(value);
        }
        fracture.insert(QStringLiteral("vertices"), vertices);
        fracture.insert(QStringLiteral("normal"), normal);
        fractures.append(fracture);
    }
    if (lineIndex >= lines.size() || fractures.size() != expectedFractures)
    {
        setLastError(QObject::tr("The natural fracture geometry count does not match the format header."));
        return result;
    }
    IDOSNaturalFractureSet* data = new IDOSNaturalFractureSet();
    data->setName(QFileInfo(filePath).completeBaseName());
    if (data->name().isEmpty())
    {
        data->setName(QFileInfo(filePath).fileName());
    }
    data->setSourceFile(filePath);
    data->setLengthUnit(lengthUnit);
    data->setCoordinateScale(coordinateScale);
    data->setFractures(fractures);
    result.append(data);
    return result;
}
