#ifndef IDOS_DELETE_WELL_COMMAND_H
#define IDOS_DELETE_WELL_COMMAND_H

#include <QHash>
#include <QList>
#include <QPointer>

#include "data/case/idoscaseitemref.h"
#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSWell;

/**
 * @brief Deletes a well and restores case references on undo.
 */
class APP_EXPORT IDOSDeleteWellCommand : public IDOSCommand
{
public:
    IDOSDeleteWellCommand(IDOSProject* project,
                          QString wellId);
    ~IDOSDeleteWellCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    void captureCaseReferences();
    void removeCaseReferences();
    void restoreCaseReferences();

    QString m_wellId;
    QPointer<IDOSWell> m_well;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
};

/**
 * @brief AI-callable metadata for deleting wells.
 */
class APP_EXPORT IDOSDeleteWellCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSDeleteWellCommandMetadata();
    ~IDOSDeleteWellCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_DELETE_WELL_COMMAND_H
