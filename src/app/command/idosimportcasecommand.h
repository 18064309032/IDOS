#ifndef IDOS_IMPORT_CASE_COMMAND_H
#define IDOS_IMPORT_CASE_COMMAND_H

#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QString>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class IDOSDataObject;

class APP_EXPORT IDOSImportCaseCommand : public IDOSCommand
{
public:
    IDOSImportCaseCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSImportCaseCommand() override;
    // Transfers parsed parentless objects on success; otherwise ownership stays with the caller.
    bool adoptParsedObjects(QList<IDOSDataObject*>& objects);

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    bool loadObjects(QString& error);
    void prepareObject(IDOSDataObject* object);
    bool isFluidKeyword(const QString& keyword) const;

    QString m_filePath;
    bool m_loaded;
    QString m_caseNameOverride;
    QList<QPointer<IDOSDataObject>> m_objects;
};

class APP_EXPORT IDOSImportCaseCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportCaseCommandMetadata();
    ~IDOSImportCaseCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
