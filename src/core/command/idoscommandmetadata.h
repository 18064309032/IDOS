#ifndef IDOS_COMMAND_METADATA_H
#define IDOS_COMMAND_METADATA_H

#include <QJsonObject>
#include <QString>

#include "command/idoscommand.h"
#include "idos_core.h"

class IDOSProject;

/**
 * @brief Serializable command capability description.
 */
class CORE_EXPORT IDOSCommandMetadata
{
public:
    IDOSCommandMetadata(QString name,
                        QString title,
                        QString description,
                        IDOSCommand::Type type);
    virtual ~IDOSCommandMetadata();

    const QString& name() const;
    const QString& title() const;
    const QString& description() const;
    IDOSCommand::Type type() const;

    QJsonObject toJson() const;
    virtual QJsonObject schema() const = 0;
    virtual IDOSCommand* create(const QJsonObject& arguments,
                                IDOSProject& project) const = 0;

private:
    QString m_name;
    QString m_title;
    QString m_description;
    IDOSCommand::Type m_type;
};

#endif // IDOS_COMMAND_METADATA_H
