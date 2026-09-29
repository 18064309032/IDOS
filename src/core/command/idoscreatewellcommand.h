#ifndef IDOS_CREATE_WELL_COMMAND_H
#define IDOS_CREATE_WELL_COMMAND_H

#include <QPointer>

#include "command/idoscommand.h"

class IDOSWell;

/**
 * @brief Creates a well in a project.
 */
class CORE_EXPORT IDOSCreateWellCommand : public IDOSCommand
{
public:
    IDOSCreateWellCommand(IDOSProject& project,
                          QString name,
                          QUndoCommand* parent = nullptr);
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

#endif // IDOS_CREATE_WELL_COMMAND_H
