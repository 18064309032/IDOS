#ifndef IDOS_IMPORT_WELL_DATA_COMMAND_H
#define IDOS_IMPORT_WELL_DATA_COMMAND_H

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class IDOSDataObject;
class IDOSWellObject;

class APP_EXPORT IDOSImportWellDataCommand : public IDOSCommand
{
public:
    IDOSImportWellDataCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSImportWellDataCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    bool loadObjects(QString& error);
    bool prepareWells(QString& error);
    bool validateWells(bool attached, QString& error) const;

    QString m_filePath;
    bool m_loaded;
    QString m_targetWellId;
    bool m_wellsPrepared;
    QHash<QString, QPointer<IDOSWellObject>> m_wellTargets;
    QHash<QString, IDOSWellObject*> m_wellBefore;
    QHash<QString, IDOSWellObject*> m_wellAfter;
    QStringList m_createdWellIds;
    QList<QPointer<IDOSDataObject>> m_objects;
};

class APP_EXPORT IDOSImportWellDataCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportWellDataCommandMetadata();
    ~IDOSImportWellDataCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
