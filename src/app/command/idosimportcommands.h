#ifndef IDOS_IMPORT_COMMANDS_H
#define IDOS_IMPORT_COMMANDS_H

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QStringList>
#include <QVariantList>

#include "data/case/idoscaseitemref.h"
#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSCaseObject;
class IDOSDataObject;
class IDOSWell;

class APP_EXPORT IDOSImportObjectsCommand : public IDOSCommand
{
public:
    enum class Mode
    {
        Case,
        Grid,
        Property,
        WellData,
        FluidRock,
        ProjectData,
        Summary
    };

    IDOSImportObjectsCommand(IDOSProject* project, const QString& name,
                             const QJsonObject& arguments);
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
    bool isFluidKeyword(const QString& keyword) const;
    bool prepareWells(QString& error);
    bool applyWells(QString& error);
    bool revertWells(QString& error);
    bool validateWells(bool attached, QString& error) const;
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
    QString m_fluidRockCategory;
    QString m_providerId;
    QString m_dataTypeId;
    Mode m_mode;
    QString m_targetGridId;
    QString m_targetCaseId;
    QString m_caseNameOverride;
    bool m_replaceExistingGrid;
    bool m_loaded;
    QString m_targetWellId;
    bool m_wellsPrepared;
    QVariantList m_importedCurveTables;
    QHash<QString, QPointer<IDOSWell>> m_wellTargets;
    QHash<QString, IDOSWell*> m_wellBefore;
    QHash<QString, IDOSWell*> m_wellAfter;
    QStringList m_createdWellIds;
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

class APP_EXPORT IDOSImportFluidRockTablesCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSImportFluidRockTablesCommandMetadata();
    ~IDOSImportFluidRockTablesCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

class APP_EXPORT IDOSAttachFluidRockTableCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSAttachFluidRockTableCommandMetadata();
    ~IDOSAttachFluidRockTableCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

class APP_EXPORT IDOSImportProjectDataCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSImportProjectDataCommandMetadata();
    ~IDOSImportProjectDataCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

class APP_EXPORT IDOSImportSummaryResultsCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSImportSummaryResultsCommandMetadata();
    ~IDOSImportSummaryResultsCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_IMPORT_COMMANDS_H
