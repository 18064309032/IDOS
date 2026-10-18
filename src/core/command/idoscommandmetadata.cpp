#include <utility>

#include "idoscommandmetadata.h"

IDOSCommandMetadata::IDOSCommandMetadata(QString name,
                                         QString title,
                                         QString description,
                                         IDOSCommand::Type type)
    : m_name(std::move(name))
    , m_title(std::move(title))
    , m_description(std::move(description))
    , m_type(type)
{
}

IDOSCommandMetadata::~IDOSCommandMetadata() = default;

const QString& IDOSCommandMetadata::name() const
{
    return m_name;
}

const QString& IDOSCommandMetadata::title() const
{
    return m_title;
}

const QString& IDOSCommandMetadata::description() const
{
    return m_description;
}

IDOSCommand::Type IDOSCommandMetadata::type() const
{
    return m_type;
}

QJsonObject IDOSCommandMetadata::toJson() const
{
    QJsonObject json;
    json.insert(QStringLiteral("name"), m_name);
    json.insert(QStringLiteral("title"), m_title);
    json.insert(QStringLiteral("description"), m_description);
    json.insert(QStringLiteral("type"),
                m_type == IDOSCommand::Type::Action
                    ? QStringLiteral("action")
                    :
                    QStringLiteral("query"));
    json.insert(QStringLiteral("schema"), schema());
    return json;
}

QJsonObject IDOSCommandMetadata::stringPropertySchema(const QString& description)
{
    QJsonObject property;
    property.insert(QStringLiteral("type"), QStringLiteral("string"));
    property.insert(QStringLiteral("description"), description);
    return property;
}

QJsonObject IDOSCommandMetadata::schemaWithProperties(const QJsonObject& properties,
                                                               const QJsonArray& required)
{
    QJsonObject schemaObject;
    schemaObject.insert(QStringLiteral("type"), QStringLiteral("object"));
    schemaObject.insert(QStringLiteral("properties"), properties);
    schemaObject.insert(QStringLiteral("required"), required);
    return schemaObject;
}
