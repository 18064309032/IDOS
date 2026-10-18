#ifndef IDOS_IMPORT_SUMMARY_RESULTS_COMMAND_H
#define IDOS_IMPORT_SUMMARY_RESULTS_COMMAND_H

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QString>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "data/case/idoscaseitemref.h"
#include "idos_app.h"

class IDOSDataObject;

class APP_EXPORT IDOSImportSummaryResultsCommand : public IDOSCommand
{
public:
    IDOSImportSummaryResultsCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSImportSummaryResultsCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    bool loadObjects(QString& error);
    void captureTargetCaseReferences();
    void appendTargetCaseReferences();
    void restoreTargetCaseReferences();

    QString m_filePath;
    bool m_loaded;
    QString m_targetCaseId;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
    QList<QPointer<IDOSDataObject>> m_objects;
};

class APP_EXPORT IDOSImportSummaryResultsCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportSummaryResultsCommandMetadata();
    ~IDOSImportSummaryResultsCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
