#ifndef IDOS_DELETE_CASE_COMMAND_H
#define IDOS_DELETE_CASE_COMMAND_H

#include <QList>
#include <QPointer>
#include <QStringList>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"

class IDOSCaseObject;
class IDOSGrid;
class IDOSGridProperty;

class CORE_EXPORT IDOSDeleteCaseCommand : public IDOSCommand
{
public:
    IDOSDeleteCaseCommand(IDOSProject* project,
                          QString caseId,
                          QUndoCommand* parent = nullptr);
    ~IDOSDeleteCaseCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    QStringList collectCaseGridIds() const;
    QStringList collectGridPropertyIds(const QString& gridId) const;
    void clearDetachedObjects();

    QString m_caseId;
    QPointer<IDOSCaseObject> m_caseObject;
    QList<QPointer<IDOSGrid>> m_grids;
    QList<QPointer<IDOSGridProperty>> m_properties;
};

class CORE_EXPORT IDOSDeleteCaseCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSDeleteCaseCommandMetadata();
    ~IDOSDeleteCaseCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_DELETE_CASE_COMMAND_H
