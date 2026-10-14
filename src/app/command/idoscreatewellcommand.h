#ifndef IDOS_CREATE_WELL_COMMAND_H
#define IDOS_CREATE_WELL_COMMAND_H

#include <QPointer>

#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSWell;

/**
 * @brief Creates a well in a project.
 */
class APP_EXPORT IDOSCreateWellCommand : public IDOSCommand
{
public:
    IDOSCreateWellCommand(IDOSProject* project,
                          QString name);
    ~IDOSCreateWellCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    QString m_wellName;
    QPointer<IDOSWell> m_well;
};

/**
 * @brief AI-callable metadata for creating wells.
 */
class APP_EXPORT IDOSCreateWellCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSCreateWellCommandMetadata();
    ~IDOSCreateWellCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_CREATE_WELL_COMMAND_H
