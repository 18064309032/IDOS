#ifndef IDOS_IMPORT_COMMANDS_H
#define IDOS_IMPORT_COMMANDS_H

#include <QHash>
#include <QList>
#include <QPointer>
#include <QStringList>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"
#include "idoscaseitemref.h"

class IDOSCaseObject;
class IDOSDataObject;

class APP_EXPORT IDOSImportObjectsCommand : public IDOSCommand
{
public:
    enum class Mode
    {
        Case,
        Grid,
        Property,
        WellData
    };

    IDOSImportObjectsCommand(IDOSProject* project,
                             QString name,
                             QString filePath,
                             Mode mode,
                             QString targetGridId,
                             QString targetCaseId,
                             QString caseNameOverride = QString(),
                             bool replaceExistingGrid = false,
                             QUndoCommand* parent = nullptr);
    ~IDOSImportObjectsCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    bool ensureLoaded(QString& error);
    bool loadObjects(QString& error);
    bool acceptObject(IDOSDataObject* object) const;
    void prepareObject(IDOSDataObject* object);
    bool hasDuplicateWell(QString& error) const;
    void captureTargetCaseReferences();
    void appendTargetCaseReferences();
    void restoreTargetCaseReferences();
    QString referencedGridId() const;
    QStringList collectGridPropertyIds(const QString& gridId) const;
    void captureReplacementObjects();
    void detachReplacementObjects();
    void restoreReplacementObjects();
    void deleteDetachedObjects();

    QString m_filePath;
    Mode m_mode;
    QString m_targetGridId;
    QString m_targetCaseId;
    QString m_caseNameOverride;
    bool m_replaceExistingGrid;
    bool m_loaded;
    QList<QPointer<IDOSDataObject>> m_objects;
    QList<QPointer<IDOSDataObject>> m_replacedObjects;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
};

class APP_EXPORT IDOSImportCaseCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportCaseCommandMetadata();
    ~IDOSImportCaseCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

class APP_EXPORT IDOSImportGridCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportGridCommandMetadata();
    ~IDOSImportGridCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

class APP_EXPORT IDOSImportPropertyCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportPropertyCommandMetadata();
    ~IDOSImportPropertyCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

class APP_EXPORT IDOSImportWellDataCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSImportWellDataCommandMetadata();
    ~IDOSImportWellDataCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_IMPORT_COMMANDS_H
