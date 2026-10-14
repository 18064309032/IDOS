#ifndef IDOS_DELETE_CASE_COMMAND_H
#define IDOS_DELETE_CASE_COMMAND_H

#include <QList>
#include <QPointer>
#include <QStringList>

#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSCaseObject;
class IDOSGrid;
class IDOSDataObject;

class APP_EXPORT IDOSDeleteCaseCommand : public IDOSCommand
{
public:
    IDOSDeleteCaseCommand(IDOSProject* project,
                          QString caseId);
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
    QList<QPointer<IDOSDataObject>> m_properties;
};

class APP_EXPORT IDOSDeleteCaseCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSDeleteCaseCommandMetadata();
    ~IDOSDeleteCaseCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_DELETE_CASE_COMMAND_H
