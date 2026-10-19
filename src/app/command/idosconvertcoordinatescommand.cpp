#include <cmath>

#include <QJsonArray>
#include <QObject>
#include <QStringList>
#include <QVector>

#include "idosprojectmetadata.h"

#include "idosconvertcoordinatescommand.h"

IDOSConvertCoordinatesCommand::IDOSConvertCoordinatesCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("coordinates.transform"), Type::Query, project, QObject::tr("Convert coordinates"))
    , m_arguments(arguments)
{
}

bool IDOSConvertCoordinatesCommand::validateCommand(QString& error) const
{
    const QJsonArray values = m_arguments.value(QStringLiteral("xyz")).toArray();
    if (m_arguments.value(QStringLiteral("sourceCrs")).toString().trimmed().isEmpty() ||
        m_arguments.value(QStringLiteral("targetCrs")).toString().trimmed().isEmpty() ||
        !m_arguments.value(QStringLiteral("xyz")).isArray() || values.size() % 3 != 0)
    {
        error = QObject::tr("Specify source CRS, target CRS and XYZ triples.");
        return false;
    }
    for (const QJsonValue& value : values)
    {
        if (!value.isDouble() || !std::isfinite(value.toDouble()))
        {
            error = QObject::tr("Coordinates must contain finite XYZ triples and a valid epoch.");
            return false;
        }
    }
    if (m_arguments.contains(QStringLiteral("decimalYear")) &&
        (!m_arguments.value(QStringLiteral("decimalYear")).isDouble() ||
         !std::isfinite(m_arguments.value(QStringLiteral("decimalYear")).toDouble())))
    {
        error = QObject::tr("Coordinates must contain finite XYZ triples and a valid epoch.");
        return false;
    }
    return true;
}

QJsonObject IDOSConvertCoordinatesCommand::buildPreview() const
{
    return m_arguments;
}

bool IDOSConvertCoordinatesCommand::apply(QString& error)
{
    if (!validateCommand(error))
    {
        return false;
    }
    QVector<double> coordinates;
    const QJsonArray values = m_arguments.value(QStringLiteral("xyz")).toArray();
    coordinates.reserve(values.size());
    for (const QJsonValue& value : values)
    {
        coordinates.append(value.toDouble());
    }
    if (!IDOSProjectMetadata::transformCrs(m_arguments.value(QStringLiteral("sourceCrs")).toString(),
            m_arguments.value(QStringLiteral("targetCrs")).toString(), coordinates, error,
            m_arguments.value(QStringLiteral("decimalYear")).toDouble()))
    {
        return false;
    }
    QJsonArray converted;
    for (double value : coordinates)
    {
        converted.append(value);
    }
    QJsonObject result = m_arguments;
    result.insert(QStringLiteral("xyz"), converted);
    setResult(result);
    return true;
}

bool IDOSConvertCoordinatesCommand::revert(QString& error)
{
    error = QObject::tr("Coordinate conversion queries do not modify project data.");
    return false;
}

IDOSConvertCoordinatesCommandMetadata::IDOSConvertCoordinatesCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("coordinates.transform"), QObject::tr("Convert coordinates"),
        QObject::tr("Convert XYZ coordinates between explicit reference systems using offline PROJ resources."),
        IDOSCommand::Type::Query)
{
}

QJsonObject IDOSConvertCoordinatesCommandMetadata::schema() const
{
    QJsonObject text;
    text.insert(QStringLiteral("type"), QStringLiteral("string"));
    QJsonObject number;
    number.insert(QStringLiteral("type"), QStringLiteral("number"));
    QJsonObject array;
    array.insert(QStringLiteral("type"), QStringLiteral("array"));
    array.insert(QStringLiteral("items"), number);
    QJsonObject properties;
    properties.insert(QStringLiteral("sourceCrs"), text);
    properties.insert(QStringLiteral("targetCrs"), text);
    properties.insert(QStringLiteral("xyz"), array);
    properties.insert(QStringLiteral("decimalYear"), number);
    QJsonObject result;
    result.insert(QStringLiteral("type"), QStringLiteral("object"));
    result.insert(QStringLiteral("properties"), properties);
    result.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("sourceCrs"), QStringLiteral("targetCrs"), QStringLiteral("xyz")});
    return result;
}

IDOSCommand* IDOSConvertCoordinatesCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSConvertCoordinatesCommand(project, arguments);
}
