#ifndef IDOS_CREATE_CASE_COMMAND_H
#define IDOS_CREATE_CASE_COMMAND_H

#include <QPointer>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"

class IDOSSimulationCaseObject;

/**
 * @brief Creates a simulation case in a project.
 */
class CORE_EXPORT IDOSCreateCaseCommand : public IDOSCommand
{
public:
    IDOSCreateCaseCommand(IDOSProject* project,
                          QString name,
                          QUndoCommand* parent = nullptr);
    ~IDOSCreateCaseCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    QString m_caseName;
    QPointer<IDOSSimulationCaseObject> m_caseObject;
};

/**
 * @brief AI-callable metadata for creating simulation cases.
 */
class CORE_EXPORT IDOSCreateCaseCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSCreateCaseCommandMetadata();
    ~IDOSCreateCaseCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_CREATE_CASE_COMMAND_H
