#include "idosdataobjecthandling.h"
#include "idosproject.h"
#include "idoscaseobject.h"
#include "idossimulationcaseobject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idoscasedialog.h"
#include "idoswell.h"
#include "command/idoscommandmanager.h"
#include "command/idoscreatecasecommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosimportcommands.h"
#include "idosproviderregistry.h"
#include "idosdataprovider.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QIcon>
#include <QJsonObject>
#include <QMessageBox>
#include <QPointer>

QStringList IDOSDataObjectHandling::collectGridPropertyIds(const IDOSProject* project, const QString& gridId)
{
    QStringList ids;
    if (project == nullptr || gridId.isEmpty())
    {
        return ids;
    }
    for (IDOSDataObject* object : project->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            ids.append(property->objectId());
        }
    }
    return ids;
}

QString IDOSDataObjectHandling::referencedGridId(const IDOSCaseObject* caseObj)
{
    if (caseObj == nullptr)
    {
        return QString();
    }
    const QList<IDOSCaseItemRef> refs = caseObj->itemRefs();
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.grid"))
        {
            return ref.objectId();
        }
    }
    return QString();
}

int IDOSDataObjectHandling::importWellData(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull())
    {
        return 0;
    }
    const QStringList files = QFileDialog::getOpenFileNames(
        parent, tr("Import Well Data"), QString(),
        tr("Well Data (*.txt *.TXT *.dev *.DEV *.las *.LAS);;All Files (*.*)"));
    if (files.isEmpty() || target.isNull())
    {
        return 0;
    }
    int createdWells = 0;
    int mergedData = 0;
    QStringList errors;
    {
        IDOSProjectUpdateGuard updateGuard(target.data());
        for (const QString& file : files)
        {
            if (target.isNull())
            {
                break;
            }
            const QString suffix = QFileInfo(file).suffix().toLower();
            QString providerId;
            bool isHeader = false;
            if (suffix == QStringLiteral("txt"))
            {
                providerId = QStringLiteral("idos.well.header");
                isHeader = true;
            }
            else if (suffix == QStringLiteral("dev"))
            {
                providerId = QStringLiteral("idos.well.path");
            }
            else if (suffix == QStringLiteral("las"))
            {
                providerId = QStringLiteral("idos.well.las");
            }
            else
            {
                continue;
            }
            std::unique_ptr<IDOSDataProvider> provider =
                IDOSProviderRegistry::instance().createProvider(providerId);
            if (!provider)
            {
                errors.append(tr("Reader unavailable for %1").arg(file));
                continue;
            }
            QList<IDOSDataObject*> objects = provider->read(file);
            for (IDOSDataObject* object : objects)
            {
                IDOSWell* well = qobject_cast<IDOSWell*>(object);
                if (well == nullptr)
                {
                    delete object;
                    continue;
                }
                IDOSWell* existingWell = wellByName(target.data(), well->name());
                if (existingWell == nullptr)
                {
                    well->setVisible(false);
                    target->addObject(well);
                    ++createdWells;
                    continue;
                }

                // 已导入井头不被后续同名井头覆盖；轨迹和测井始终合并到同一井对象。
                if (isHeader && existingWell->hasWellHead())
                {
                    delete well;
                    continue;
                }

                existingWell->mergeFrom(well);
                delete well;
                ++mergedData;
            }
            if (!provider->lastError().isEmpty())
            {
                errors.append(provider->lastError());
            }
        }
    }
    if (!errors.isEmpty())
    {
        QMessageBox::warning(parent, tr("Import Well Data"), errors.join(QStringLiteral("\n")));
    }
    return createdWells + mergedData;
}

int IDOSDataObjectHandling::importWellData(IDOSProject* project, const QString& targetWellId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || targetWellId.isEmpty())
    {
        return 0;
    }
    IDOSDataObject* obj = target->objectById(targetWellId);
    IDOSWell* targetWell = qobject_cast<IDOSWell*>(obj);
    if (targetWell == nullptr)
    {
        return 0;
    }
    const QString file = QFileDialog::getOpenFileName(
        parent, tr("Import Well Data"), QString(),
        tr("Well Data (*.txt *.TXT *.dev *.DEV *.las *.LAS);;All Files (*.*)"));
    if (file.isEmpty() || target.isNull())
    {
        return 0;
    }
    const QString suffix = QFileInfo(file).suffix().toLower();
    QString providerId;
    if (suffix == QStringLiteral("txt"))
    {
        providerId = QStringLiteral("idos.well.header");
    }
    else if (suffix == QStringLiteral("dev"))
    {
        providerId = QStringLiteral("idos.well.path");
    }
    else if (suffix == QStringLiteral("las"))
    {
        providerId = QStringLiteral("idos.well.las");
    }
    else
    {
        return 0;
    }
    std::unique_ptr<IDOSDataProvider> provider =
        IDOSProviderRegistry::instance().createProvider(providerId);
    if (!provider)
    {
        QMessageBox::warning(parent, tr("Import Well Data"), tr("The reader is unavailable."));
        return 0;
    }
    QList<IDOSDataObject*> objects = provider->read(file);
    IDOSWell* sourceWell = nullptr;
    for (IDOSDataObject* object : objects)
    {
        IDOSWell* candidate = qobject_cast<IDOSWell*>(object);
        if (candidate != nullptr)
        {
            sourceWell = candidate;
            break;
        }
    }
    if (sourceWell == nullptr)
    {
        qDeleteAll(objects);
        QMessageBox::warning(parent, tr("Import Well Data"), provider->lastError());
        return 0;
    }
    {
        IDOSProjectUpdateGuard updateGuard(target.data());
        targetWell->mergeFrom(sourceWell);
    }
    qDeleteAll(objects);
    return 1;
}

bool IDOSDataObjectHandling::newWell(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> targetProject(project);
    if (targetProject.isNull())
    {
        return false;
    }
    QInputDialog dialog(parent);
    dialog.setObjectName(QStringLiteral("newWellDialog"));
    dialog.setWindowTitle(tr("New Well"));
    dialog.setWindowIcon(QIcon(QStringLiteral(":/images/gui-well-new.svg")));
    dialog.setLabelText(tr("Well name:"));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setOkButtonText(tr("Create"));
    dialog.setCancelButtonText(tr("Cancel"));
    while (dialog.exec() == QDialog::Accepted)
    {
        if (targetProject.isNull())
        {
            return false;
        }
        const QString name = dialog.textValue().trimmed();
        if (name.isEmpty())
        {
            dialog.setLabelText(tr("Please enter a well name."));
            continue;
        }
        bool exists = false;
        for (IDOSDataObject* object : targetProject->objects())
        {
            IDOSWell* existing = qobject_cast<IDOSWell*>(object);
            if (existing != nullptr && existing->name().compare(name, Qt::CaseInsensitive) == 0)
            {
                exists = true;
                break;
            }
        }
        if (exists)
        {
            dialog.setLabelText(tr("A well named \"%1\" already exists. Please enter a different name.").arg(name));
            continue;
        }
        IDOSCommandManager* commandManager = targetProject->commandManager();
        if (commandManager == nullptr)
        {
            return false;
        }

        return commandManager->execute(new IDOSCreateWellCommand(targetProject, name));
    }
    return false;
}

bool IDOSDataObjectHandling::deleteWell(IDOSProject* project, const QString& wellId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || wellId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->objectById(wellId);
    IDOSWell* well = qobject_cast<IDOSWell*>(obj);
    if (well == nullptr)
    {
        return false;
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(parent, tr("Delete Well"),
                               tr("Are you sure you want to delete well \"%1\"?\n"
                                  "It will be removed from all cases that reference it.")
                                   .arg(well->name()),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeleteWellCommand(target, wellId));
}

bool IDOSDataObjectHandling::deleteGrid(IDOSProject* project, const QString& gridId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || gridId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->objectById(gridId);
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(obj);
    if (grid == nullptr)
    {
        return false;
    }
    const int propertyCount = collectGridPropertyIds(target.data(), gridId).size();
    const QMessageBox::StandardButton btn =
        QMessageBox::question(parent, tr("Delete Grid"),
                               tr("Are you sure you want to delete grid \"%1\"?\n"
                                  "It will also delete %2 propert(ies) under this grid "
                                  "and remove it from its case.")
                                   .arg(grid->name())
                                   .arg(propertyCount),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeleteGridCommand(target, gridId));
}

IDOSWell* IDOSDataObjectHandling::wellByName(const IDOSProject* project, const QString& name)
{
    if (project == nullptr)
    {
        return nullptr;
    }

    const QString normalizedName = name.trimmed();
    for (IDOSDataObject* object : project->objects())
    {
        IDOSWell* well = qobject_cast<IDOSWell*>(object);
        if (well != nullptr && well->name().trimmed().compare(normalizedName, Qt::CaseInsensitive) == 0)
        {
            return well;
        }
    }
    return nullptr;
}

bool IDOSDataObjectHandling::caseNameExists(const IDOSProject* project, const QString& name)
{
    for (IDOSDataObject* object : project->objects())
    {
        if (qobject_cast<IDOSCaseObject*>(object) != nullptr &&
            object->name().trimmed().compare(name, Qt::CaseInsensitive) == 0)
        {
            return true;
        }
    }
    return false;
}

bool IDOSDataObjectHandling::newCase(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull())
    {
        return false;
    }
    IDOSCaseDialog dialog(QString(), QString(), {}, target, parent);
    if (dialog.exec() != QDialog::Accepted || target.isNull())
    {
        return false;
    }
    if (caseNameExists(target, dialog.caseName()))
    {
        QMessageBox::warning(parent, tr("New Case"), tr("A case with this name already exists."));
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSCreateCaseCommand(target, dialog.caseName()));
}

bool IDOSDataObjectHandling::importCase(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull())
    {
        return false;
    }
    const QString file = QFileDialog::getOpenFileName(parent, tr("Select ECLIPSE Case"), QString(),
                                                      tr("ECLIPSE Cases (*.DATA *.data);;All Files (*)"));
    return importCase(target, file, parent);
}

bool IDOSDataObjectHandling::importCase(IDOSProject* project, const QString& filePath, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || filePath.isEmpty())
    {
        return false;
    }
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.case.simulation.eclipse"));
    if (!reader)
    {
        QMessageBox::warning(parent, tr("Import Case"), tr("The simulation case reader is unavailable."));
        return false;
    }
    QList<IDOSDataObject*> objects = reader->read(filePath);
    IDOSSimulationCaseObject* simulationCase = nullptr;
    for (IDOSDataObject* object : objects)
    {
        IDOSSimulationCaseObject* candidate = qobject_cast<IDOSSimulationCaseObject*>(object);
        if (candidate != nullptr)
        {
            simulationCase = candidate;
            break;
        }
    }
    if (simulationCase == nullptr)
    {
        qDeleteAll(objects);
        QMessageBox::warning(parent, tr("Import Case"), reader->lastError());
        return false;
    }
    IDOSCaseDialog dialog(simulationCase->name(), simulationCase->sourceFile(), simulationCase->pendingWellNames(),
                          target, parent);
    if (dialog.exec() != QDialog::Accepted || target.isNull())
    {
        qDeleteAll(objects);
        return false;
    }
    if (caseNameExists(target, dialog.caseName()))
    {
        qDeleteAll(objects);
        QMessageBox::warning(parent, tr("Import Case"), tr("A case with this name already exists."));
        return false;
    }
    qDeleteAll(objects);

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    IDOSImportObjectsCommand* command =
        new IDOSImportObjectsCommand(target,
                                     QStringLiteral("case.import"),
                                     filePath,
                                     IDOSImportObjectsCommand::Mode::Case,
                                     QString(),
                                     QString(),
                                     dialog.caseName());
    const bool success = commandManager->execute(command);
    if (!success)
    {
        QMessageBox::warning(parent, tr("Import Case"), command->errorString());
    }
    return success;
}

bool IDOSDataObjectHandling::deleteCase(IDOSProject* project, const QString& caseId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || caseId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->objectById(caseId);
    IDOSCaseObject* caseObj = qobject_cast<IDOSCaseObject*>(obj);
    if (caseObj == nullptr)
    {
        return false;
    }
    // 网格归工况私有：收集该工况引用的网格及其属性数量用于确认提示
    QStringList gridIds;
    int propertyTotal = 0;
    const QList<IDOSCaseItemRef> refs = caseObj->itemRefs();
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.grid"))
        {
            gridIds.append(ref.objectId());
            propertyTotal += collectGridPropertyIds(target.data(), ref.objectId()).size();
        }
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(parent, tr("Delete Case"),
                               tr("Are you sure you want to delete case \"%1\"?\n"
                                  "Its %2 grid(s) and %3 propert(ies) will also be deleted. "
                                  "Referenced wells are kept.")
                                   .arg(caseObj->name())
                                   .arg(gridIds.size())
                                   .arg(propertyTotal),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeleteCaseCommand(target, caseId));
}

bool IDOSDataObjectHandling::importGrid(IDOSProject* project, const QString& targetCaseId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || targetCaseId.isEmpty())
    {
        return false;
    }
    IDOSCaseObject* caseObj = qobject_cast<IDOSCaseObject*>(target->objectById(targetCaseId));
    if (caseObj == nullptr)
    {
        return false;
    }
    // 一个工况仅持有一个网格：已有网格时确认替换
    const QString existingGridId = referencedGridId(caseObj);
    bool replacing = false;
    if (!existingGridId.isEmpty())
    {
        IDOSGrid* existingGrid = qobject_cast<IDOSGrid*>(target->objectById(existingGridId));
        const QString gridName = existingGrid != nullptr ? existingGrid->name() : existingGridId;
        const int propertyCount = collectGridPropertyIds(target.data(), existingGridId).size();
        const QMessageBox::StandardButton btn = QMessageBox::question(
            parent, tr("Import Grid"),
            tr("Case \"%1\" already has grid \"%2\".\n"
               "Importing a new grid will delete it along with %3 propert(ies). Continue?")
                .arg(caseObj->name())
                .arg(gridName)
                .arg(propertyCount),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn != QMessageBox::Yes || target.isNull())
        {
            return false;
        }
        replacing = true;
    }
    const QString file = QFileDialog::getOpenFileName(
        parent, tr("Import Grid"), QString(),
        tr("ECLIPSE Grids (*.EGRID *.egrid *.GRID *.grid);;All Files (*.*)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    IDOSImportObjectsCommand* command =
        new IDOSImportObjectsCommand(target,
                                     QStringLiteral("grid.import"),
                                     file,
                                     IDOSImportObjectsCommand::Mode::Grid,
                                     QString(),
                                     targetCaseId,
                                     QString(),
                                     replacing);
    const bool success = commandManager->execute(command);
    if (!success)
    {
        QMessageBox::warning(parent, tr("Import Grid"), command->errorString());
    }
    return success;
}

bool IDOSDataObjectHandling::deleteProperty(IDOSProject* project, const QString& propertyId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || propertyId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->objectById(propertyId);
    if (qobject_cast<IDOSGridProperty*>(obj) == nullptr)
    {
        return false;
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(parent, tr("Delete Property"),
                               tr("Are you sure you want to delete property \"%1\"?\n"
                                  "It will be removed from all cases that reference it.")
                                   .arg(obj->name()),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeletePropertyCommand(target, propertyId));
}

bool IDOSDataObjectHandling::importProperty(IDOSProject* project, const QString& targetGridId,
                                            const QString& targetCaseId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull())
    {
        return false;
    }
    // 当前能产出网格属性的可识别格式为 ECLIPSE .DATA；后续新增 .GRDECL/.INIT/.UNRST
    // provider 后在此扩充过滤项
    const QString file = QFileDialog::getOpenFileName(
        parent, tr("Import Grid Property"), QString(),
        tr("ECLIPSE Data Files (*.DATA *.data);;All Files (*)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    IDOSImportObjectsCommand* command =
        new IDOSImportObjectsCommand(target,
                                     QStringLiteral("property.import"),
                                     file,
                                     IDOSImportObjectsCommand::Mode::Property,
                                     targetGridId,
                                     targetCaseId);
    const bool success = commandManager->execute(command);
    if (!success)
    {
        QMessageBox::warning(parent, tr("Import Grid Property"), command->errorString());
        return false;
    }

    const int importedCount = command->result().value(QStringLiteral("objectCount")).toInt();
    QMessageBox::information(parent, tr("Import Grid Property"),
                             tr("Imported %1 properties.").arg(importedCount));
    return true;
}

