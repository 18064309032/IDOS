#include "idoscommand.h"
#include "idoscommandmetadata.h"

#include "idoscommandregistry.h"

IDOSCommandRegistry::IDOSCommandRegistry(QObject* parent)
    :
    QObject(parent)
    , m_changing(false)
{
}

IDOSCommandRegistry::~IDOSCommandRegistry() = default;

bool IDOSCommandRegistry::registerMetadata(IDOSCommandMetadata* metadata)
{
    if (m_changing || metadata == nullptr || metadata->name().trimmed().isEmpty()
        || m_metadata.find(metadata->name()) != m_metadata.end())
    {
        return false;
    }

    const QString name = metadata->name();
    m_changing = true;
    m_metadata.emplace(name, std::unique_ptr<IDOSCommandMetadata>(metadata));
    emit metadataRegistered(name);
    m_changing = false;
    return true;
}

bool IDOSCommandRegistry::unregisterMetadata(const QString& name)
{
    if (m_changing || m_metadata.find(name) == m_metadata.end())
    {
        return false;
    }
    m_changing = true;
    emit metadataUnregistering(name);
    m_metadata.erase(name);
    emit metadataUnregistered(name);
    m_changing = false;
    return true;
}

std::unique_ptr<IDOSCommand> IDOSCommandRegistry::create(
    const QString& name,
    const QJsonObject& arguments,
    IDOSProject* project) const
{
    const IDOSCommandMetadata* commandMetadata = metadata(name);

    if (commandMetadata == nullptr)
    {
        return nullptr;
    }

    return std::unique_ptr<IDOSCommand>(commandMetadata->create(arguments, project));
}

const IDOSCommandMetadata* IDOSCommandRegistry::metadata(const QString& name) const
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
