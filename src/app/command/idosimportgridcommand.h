#ifndef IDOS_IMPORT_GRID_COMMAND_H
#define IDOS_IMPORT_GRID_COMMAND_H

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "data/case/idoscaseitemref.h"
#include "idos_app.h"

class IDOSDataObject;

class APP_EXPORT IDOSImportGridCommand : public IDOSCommand
{
public:
    IDOSImportGridCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSImportGridCommand() override;

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
    QString referencedGridId() const;
    QStringList collectGridPropertyIds(const QString& gridId) const;
    void captureReplacementObjects();
    void detachReplacementObjects();
    void restoreReplacementObjects();

    QString m_filePath;
    bool m_loaded;
    QString m_targetCaseId;
    bool m_replaceExistingGrid;
    QList<QPointer<IDOSDataObject>> m_replacedObjects;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
    QList<QPointer<IDOSDataObject>> m_objects;
};

class APP_EXPORT IDOSImportGridCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportGridCommandMetadata();
    ~IDOSImportGridCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
