#ifndef IDOS_IMPORT_GRID_PROPERTY_COMMAND_H
#define IDOS_IMPORT_GRID_PROPERTY_COMMAND_H

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

class APP_EXPORT IDOSImportGridPropertyCommand : public IDOSCommand
{
public:
    IDOSImportGridPropertyCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSImportGridPropertyCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    bool loadObjects(QString& error);
    void prepareObject(IDOSDataObject* object);
    void captureTargetCaseReferences();
    void appendTargetCaseReferences();
    void restoreTargetCaseReferences();

    QString m_filePath;
    bool m_loaded;
    QString m_targetCaseId;
    QString m_targetGridId;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
    QList<QPointer<IDOSDataObject>> m_objects;
};

class APP_EXPORT IDOSImportGridPropertyCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportGridPropertyCommandMetadata();
    ~IDOSImportGridPropertyCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
