#ifndef IDOS_IMPORT_DATA_COMMAND_H
#define IDOS_IMPORT_DATA_COMMAND_H

#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QString>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class IDOSDataObject;

class APP_EXPORT IDOSImportDataCommand : public IDOSCommand
{
public:
    IDOSImportDataCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSImportDataCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    bool loadObjects(QString& error);

    QString m_filePath;
    bool m_loaded;
    QString m_providerId;
    QString m_dataTypeId;
    QList<QPointer<IDOSDataObject>> m_objects;
};

class APP_EXPORT IDOSImportDataCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportDataCommandMetadata();
    ~IDOSImportDataCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
