#include "idosdataobjecthandling.h"
#include "idosproject.h"
#include "idossimulationcaseobject.h"
#include "idoscasedialog.h"
#include "idoswell.h"
#include "idoswellimportdialog.h"
#include "idoswellpathimportdialog.h"
#include "idosproviderregistry.h"
#include "idosdataprovider.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QIcon>
#include <QMessageBox>
#include <QPointer>

int IDOSDataObjectHandling::importWellHeaders(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> targetProject(project);
    if (targetProject.isNull())
    {
        return 0;
    }
    const QString path = QFileDialog::getOpenFileName(parent, tr("Select Well Header File"), QString(),
                                                      tr("All Files (*);;Text Files (*.txt)"));
    if (path.isEmpty() || targetProject.isNull())
    {
        return 0;
    }
    std::unique_ptr<IDOSDataProvider> provider =
        IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.well.header"));
    if (!provider)
    {
        QMessageBox::warning(parent, tr("Import Well Data"), tr("The well header reader is unavailable."));
        return 0;
    }
    QList<IDOSDataObject*> objects = provider->read(path);
    if (objects.isEmpty())
    {
        QMessageBox::warning(parent, tr("Import Well Data"), provider->lastError());
        return 0;
    }
    QStringList names;
    for (IDOSDataObject* object : targetProject->objects())
    {
        if (qobject_cast<IDOSWell*>(object) != nullptr)
        {
            names.append(object->name());
        }
    }
    int importedCount = 0;
    IDOSWellImportDialog dialog(path, objects, names, parent);
    if (dialog.exec() == QDialog::Accepted && !targetProject.isNull())
    {
        const QList<int> rows = dialog.selectedRows();
        // 批量加入选中井：数据树逐行插入，但引用聚合只做一次
        IDOSProjectUpdateGuard updateGuard(targetProject.data());
        for (int row : rows)
        {
            targetProject->addObject(objects[row]);
            objects[row] = nullptr;
            ++importedCount;
        }
        QMessageBox::information(
            parent, tr("Import Well Data"),
            tr("Created %1 wells. Skipped %2 duplicate rows.").arg(importedCount).arg(objects.size() - importedCount));
    }
    qDeleteAll(objects);
    return importedCount;
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
        IDOSWell* well = new IDOSWell();
        well->setName(name);
        targetProject->addObject(well);
        return true;
    }
    return false;
}

int IDOSDataObjectHandling::importWellData(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull())
    {
        return 0;
    }
    QInputDialog choice(parent);
    choice.setObjectName(QStringLiteral("wellImportTypeDialog"));
    choice.setWindowTitle(tr("Import Well Data"));
    choice.setLabelText(tr("Data type:"));
    choice.setComboBoxItems({tr("Well Headers"), tr("Well Trajectories (.dev)")});
    choice.setComboBoxEditable(false);
    choice.setOkButtonText(tr("Next"));
    choice.setCancelButtonText(tr("Cancel"));
    if (choice.exec() != QDialog::Accepted || target.isNull())
    {
        return 0;
    }
    if (choice.textValue() == tr("Well Headers"))
    {
        return importWellHeaders(target, parent);
    }
    const QStringList files = QFileDialog::getOpenFileNames(parent, tr("Select Well Trajectory Files"), QString(),
                                                            tr("Well Trajectories (*.dev *.DEV);;All Files (*)"));
    return importWellPaths(target, files, parent);
}

int IDOSDataObjectHandling::importWellPaths(IDOSProject* project, const QStringList& files, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || files.isEmpty())
    {
        return 0;
    }
    std::unique_ptr<IDOSDataProvider> provider =
        IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.well.path"));
    if (!provider)
    {
        QMessageBox::warning(parent, tr("Import Well Data"), tr("The trajectory reader is unavailable."));
        return 0;
    }
    QList<IDOSWell*> sources;
    QStringList errors;
    for (const QString& file : files)
    {
        QList<IDOSDataObject*> objects = provider->read(file);
        IDOSWell* source = objects.size() == 1 ? qobject_cast<IDOSWell*>(objects.first()) : nullptr;
        sources.append(source);
        errors.append(provider->lastError());
        if (source == nullptr)
        {
            qDeleteAll(objects);
        }
    }
    int imported = 0;
    IDOSWellPathImportDialog dialog(files, sources, errors, target, parent);
    if (dialog.exec() == QDialog::Accepted && !target.isNull())
    {
        const QStringList ids = dialog.targetIds();
        for (int row = 0; row < ids.size(); ++row)
        {
            if (target.isNull())
            {
                break;
            }
            if (ids[row].isEmpty() || sources[row] == nullptr)
            {
                continue;
            }
            IDOSWell* well = qobject_cast<IDOSWell*>(target->objectById(ids[row]));
            if (well == nullptr || well->hasPath() || !well->path().isEmpty() ||
                well->name().trimmed().toCaseFolded() != sources[row]->name().trimmed().toCaseFolded())
            {
                continue;
            }
            well->setPath(sources[row]->path());
            ++imported;
        }
        QMessageBox::information(
            parent, tr("Import Well Data"),
            tr("Imported %1 trajectories. Skipped %2 files.").arg(imported).arg(files.size() - imported));
    }
    qDeleteAll(sources);
    return imported;
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
    IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
    simulationCase->setName(dialog.caseName());
    target->addObject(simulationCase);
    return true;
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
    simulationCase->setName(dialog.caseName());
    {
        // 网格/属性/井与工况在同一批加入，工况分支只在最后统一构建一次
        IDOSProjectUpdateGuard updateGuard(target.data());
        for (IDOSDataObject* object : objects)
        {
            if (object == nullptr || object == simulationCase)
            {
                continue;
            }
            target->addObject(object);
        }
        simulationCase->resolveReferences(target);
        target->addObject(simulationCase);
    }
    objects.clear();
    return true;
}

