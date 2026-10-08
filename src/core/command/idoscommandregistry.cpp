#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"

#include "command/idoscommandregistry.h"

IDOSCommandRegistry::IDOSCommandRegistry() = default;

IDOSCommandRegistry::~IDOSCommandRegistry() = default;

bool IDOSCommandRegistry::add(std::unique_ptr<IDOSCommandMetadata> metadata)
{
    if (!metadata || metadata->name().trimmed().isEmpty())
    {
        return false;
    }

    const QString name = metadata->name();
    return m_metadata.emplace(name, std::move(metadata)).second;
}

bool IDOSCommandRegistry::remove(const QString& name)
{
    return m_metadata.erase(name) > 0;
}

std::unique_ptr<IDOSCommand> IDOSCommandRegistry::create(
    const QString& name,
    const QJsonObject& arguments,
    IDOSProject* project) const
{
    const IDOSCommandMetadata* commandMetadata = find(name);

    if (commandMetadata == nullptr)
    {
        return nullptr;
    }

    return std::unique_ptr<IDOSCommand>(commandMetadata->create(arguments, project));
}

const IDOSCommandMetadata* IDOSCommandRegistry::find(const QString& name) const
{
    const std::map<QString, std::unique_ptr<IDOSCommandMetadata>>::const_iterator iterator =
        m_metadata.find(name);

    if (iterator == m_metadata.end())
    {
        return nullptr;
    }

    return iterator->second.get();
}

QList<const IDOSCommandMetadata*> IDOSCommandRegistry::metadata() const
{
    QList<const IDOSCommandMetadata*> result;
    result.reserve(static_cast<int>(m_metadata.size()));

    for (const std::pair<const QString, std::unique_ptr<IDOSCommandMetadata>>& entry : m_metadata)
    {
        result.append(entry.second.get());
    }

    return result;
}

QJsonArray IDOSCommandRegistry::toJson() const
{
    QJsonArray json;

    for (const std::pair<const QString, std::unique_ptr<IDOSCommandMetadata>>& entry : m_metadata)
    {
        json.append(entry.second->toJson());
    }

    return json;
}

QJsonArray IDOSCommandRegistry::toToolJson() const
{
    QJsonArray json;

    for (const std::pair<const QString, std::unique_ptr<IDOSCommandMetadata>>& entry : m_metadata)
    {
        const IDOSCommandMetadata* commandMetadata = entry.second.get();

        QJsonObject functionObject;
        functionObject.insert(QStringLiteral("name"), commandMetadata->name());
        functionObject.insert(QStringLiteral("description"), commandMetadata->description());
        functionObject.insert(QStringLiteral("parameters"), commandMetadata->schema());

        QJsonObject toolObject;
        toolObject.insert(QStringLiteral("type"), QStringLiteral("function"));
        toolObject.insert(QStringLiteral("function"), functionObject);
        json.append(toolObject);
    }

    return json;
}
