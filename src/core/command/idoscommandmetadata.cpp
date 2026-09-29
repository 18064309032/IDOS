#include "command/idoscommandmetadata.h"

#include <utility>

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
                    : QStringLiteral("query"));
    json.insert(QStringLiteral("schema"), schema());
    return json;
}
