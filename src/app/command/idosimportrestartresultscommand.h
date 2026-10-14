#ifndef IDOS_IMPORT_RESTART_RESULTS_COMMAND_H
#define IDOS_IMPORT_RESTART_RESULTS_COMMAND_H

#include <QList>
#include <QPointer>
#include <QString>

#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSGridProperty;

class APP_EXPORT IDOSImportRestartResultsCommand : public IDOSCommand
{
public:
    IDOSImportRestartResultsCommand(IDOSProject* project,
                                    QString gridId,
                                    QString filePath);
    ~IDOSImportRestartResultsCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    bool loadProperties(QString& error);
    bool isSupportedKeyword(const QString& keyword) const;
    void deleteDetachedProperties();

    QString m_gridId;
    QString m_filePath;
    int m_firstReportStep;
    int m_lastReportStep;
    int m_reportStepCount;
    bool m_loaded;
    QList<QPointer<IDOSGridProperty>> m_properties;
};

class APP_EXPORT IDOSImportRestartResultsCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportRestartResultsCommandMetadata();
    ~IDOSImportRestartResultsCommandMetadata() override;
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif // IDOS_IMPORT_RESTART_RESULTS_COMMAND_H
