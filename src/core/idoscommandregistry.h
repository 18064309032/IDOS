#ifndef IDOS_COMMAND_REGISTRY_H
#define IDOS_COMMAND_REGISTRY_H

#include <map>
#include <memory>

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>

#include "idos_core.h"

class IDOSCommand;
class IDOSCommandMetadata;
class IDOSProject;

/**
 * @brief Registered command metadata and factory collection.
 */
class CORE_EXPORT IDOSCommandRegistry
{
public:
    IDOSCommandRegistry();
    ~IDOSCommandRegistry();

    IDOSCommandRegistry(const IDOSCommandRegistry&) = delete;
    IDOSCommandRegistry& operator=(const IDOSCommandRegistry&) = delete;

    bool add(std::unique_ptr<IDOSCommandMetadata> metadata);
    bool remove(const QString& name);

    std::unique_ptr<IDOSCommand> create(const QString& name,
                                        const QJsonObject& arguments,
                                        IDOSProject* project) const;

    const IDOSCommandMetadata* find(const QString& name) const;
    QList<const IDOSCommandMetadata*> metadata() const;
    QJsonArray toJson() const;
    QJsonArray toToolJson() const;

private:
    std::map<QString, std::unique_ptr<IDOSCommandMetadata>> m_metadata;
};

#endif // IDOS_COMMAND_REGISTRY_H
