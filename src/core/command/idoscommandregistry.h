#ifndef IDOS_COMMAND_REGISTRY_H
#define IDOS_COMMAND_REGISTRY_H

#include <map>
#include <memory>

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

#include "idos_core.h"

class IDOSCommand;
class IDOSCommandMetadata;
class IDOSProject;

/**
 * @brief Registered command metadata and factory collection.
 */
class CORE_EXPORT IDOSCommandRegistry : public QObject
{
    Q_OBJECT

public:
    explicit IDOSCommandRegistry(QObject* parent = nullptr);
    ~IDOSCommandRegistry() override;

    IDOSCommandRegistry(const IDOSCommandRegistry&) = delete;
    IDOSCommandRegistry& operator=(const IDOSCommandRegistry&) = delete;

    bool registerMetadata(IDOSCommandMetadata* metadata);
    bool unregisterMetadata(const QString& name);

    std::unique_ptr<IDOSCommand> create(const QString& name,
                                        const QJsonObject& arguments,
                                        IDOSProject* project) const;

    const IDOSCommandMetadata* metadata(const QString& name) const;
    QList<const IDOSCommandMetadata*> metadata() const;
    QJsonArray toJson() const;
    QJsonArray toToolJson() const;

signals:
    void metadataRegistered(const QString& metadataId);
    void metadataUnregistering(const QString& metadataId);
    void metadataUnregistered(const QString& metadataId);

private:
    std::map<QString, std::unique_ptr<IDOSCommandMetadata>> m_metadata;
    bool m_changing;
};

#endif // IDOS_COMMAND_REGISTRY_H
