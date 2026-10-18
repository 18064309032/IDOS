#ifndef IDOS_IMPORT_FLUID_ROCK_TABLES_COMMAND_H
#define IDOS_IMPORT_FLUID_ROCK_TABLES_COMMAND_H

#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QString>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class IDOSDataObject;

class APP_EXPORT IDOSImportFluidRockTablesCommand : public IDOSCommand
{
public:
    IDOSImportFluidRockTablesCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSImportFluidRockTablesCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    bool loadObjects(QString& error);
    bool isFluidKeyword(const QString& keyword) const;

    QString m_filePath;
    bool m_loaded;
    QString m_fluidRockCategory;
    QList<QPointer<IDOSDataObject>> m_objects;
};

class APP_EXPORT IDOSImportFluidRockTablesCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportFluidRockTablesCommandMetadata();
    ~IDOSImportFluidRockTablesCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
