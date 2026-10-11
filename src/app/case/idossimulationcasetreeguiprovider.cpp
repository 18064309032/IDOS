#include <QAction>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QVariant>

#include "command/idoscommandmanager.h"
#include "command/idosimportcommands.h"
#include "idoscasedialog.h"
#include "idoscaseobject.h"
#include "idoscreatecasecommand.h"
#include "idoscreatewellcommand.h"
#include "idosdataobject.h"
#include "idosdataprovider.h"
#include "idosdataproviderregistry.h"
#include "idosdeletecasecommand.h"
#include "idosdeletegridcommand.h"
#include "idosdeletepropertycommand.h"
#include "idosdeletewellcommand.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idossimulationcaseobject.h"
#include "idostree.h"
#include "idostreeitem.h"
#include "idostreeview.h"
#include "idoswell.h"
#include "log/idoslogger.h"

#include "idossimulationcasetreeguiprovider.h"

QString IDOSSimulationCaseTreeGuiProvider::name() const
{
    return QStringLiteral("idos.simulationcase");
}

QAction* IDOSSimulationCaseTreeGuiProvider::addAction(QMenu* menu, IDOSTreeView* view, const QString& text)
{
    QAction* action = menu->addAction(text);
    IDOSTreeItem* item = view->menuItem();
    action->setProperty("dataId", item != nullptr ? item->dataId() : QString());
    action->setProperty("caseId", item != nullptr ? item->caseId() : QString());
    action->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    return action;
}

IDOSTreeView* IDOSSimulationCaseTreeGuiProvider::actionView() const
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMenu* menu = action != nullptr ? qobject_cast<QMenu*>(action->parent()) : nullptr;
    IDOSTreeView* view = menu != nullptr ? qobject_cast<IDOSTreeView*>(menu->parentWidget()) : nullptr;
    if (view == nullptr || view->project() == nullptr ||
        action->property("project").value<QObject*>() != view->project())
    {
        return nullptr;
    }
    return view;
}

bool IDOSSimulationCaseTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    if (item == nullptr)
    {
        return true;
    }
    return item->tree() != nullptr && item->tree()->project() != nullptr &&
           qobject_cast<IDOSCaseObject*>(item->tree()->project()->object(item->dataId())) != nullptr;
}

void IDOSSimulationCaseTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
{
    if (view->project() == nullptr)
    {
        return;
    }
    IDOSTreeItem* item = view->menuItem();

    if (view->tree() == nullptr || view->tree()->name() != QStringLiteral("case"))
    {
        return;
    }
    if (item != nullptr && item->key() == QStringLiteral("case.grids"))
    {
        QAction* import = addAction(menu, view, tr("Import Grid..."));
        import->setObjectName(QStringLiteral("importGridAction"));
        connect(import, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onImportGrid);
        return;
    }
    if (item != nullptr && !item->isGroup())
    {
        QAction* remove = addAction(menu, view, tr("Delete Case..."));
        remove->setObjectName(QStringLiteral("deleteCaseAction"));
        connect(remove, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onDeleteCase);
        return;
    }
    QAction* create = addAction(menu, view, tr("New Case..."));
    create->setObjectName(QStringLiteral("newCaseAction"));
    connect(create, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onNewCase);
    QAction* import = addAction(menu, view, tr("Import Case..."));
    import->setObjectName(QStringLiteral("importCaseAction"));
    connect(import, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onImportCase);
}

void IDOSSimulationCaseTreeGuiProvider::onNewCase()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    Q_UNUSED(action);
    IDOSSimulationCaseTreeGuiProvider::newCase(view->project(), view);
}

void IDOSSimulationCaseTreeGuiProvider::onImportCase()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    Q_UNUSED(action);
    IDOSSimulationCaseTreeGuiProvider::importCase(view->project(), view);
}

void IDOSSimulationCaseTreeGuiProvider::onImportGrid()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    const QString caseId = action->property("caseId").toString();
    if (qobject_cast<IDOSCaseObject*>(view->project()->object(caseId)) != nullptr)
    {
        IDOSSimulationCaseTreeGuiProvider::importGrid(view->project(), caseId, view);
    }
}

void IDOSSimulationCaseTreeGuiProvider::onDeleteCase()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    const QString id = action->property("dataId").toString();
    if (qobject_cast<IDOSCaseObject*>(view->project()->object(id)) != nullptr)
    {
        IDOSSimulationCaseTreeGuiProvider::deleteCase(view->project(), id, view);
    }
}


QStringList IDOSSimulationCaseTreeGuiProvider::collectGridPropertyIds(const IDOSProject* project, const QString& gridId)
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

QString IDOSSimulationCaseTreeGuiProvider::referencedGridId(const IDOSCaseObject* caseObj)
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

bool IDOSSimulationCaseTreeGuiProvider::caseNameExists(const IDOSProject* project, const QString& name)
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

bool IDOSSimulationCaseTreeGuiProvider::newCase(IDOSProject* project, QWidget* parent)
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

bool IDOSSimulationCaseTreeGuiProvider::importCase(IDOSProject* project, QWidget* parent)
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

bool IDOSSimulationCaseTreeGuiProvider::importCase(IDOSProject* project, const QString& filePath, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || filePath.isEmpty())
    {
        return false;
    }
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.case.simulation.eclipse"));
    if (!reader)
    {
        IDOS_ERROR(tr("The simulation case reader is unavailable."));
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
        IDOS_ERROR(tr("Simulation case parsing failed for %1: %2")
                       .arg(filePath, reader->lastError()));
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
        IDOS_WARN(tr("Case import rejected because the name already exists: %1")
                      .arg(dialog.caseName()));
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

    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), filePath);
    arguments.insert(QStringLiteral("caseName"), dialog.caseName());
    IDOSImportObjectsCommand* command =
        new IDOSImportObjectsCommand(target, QStringLiteral("case.import"), arguments);
    const bool success = commandManager->execute(command);
    if (!success)
    {
        QMessageBox::warning(parent, tr("Import Case"), commandManager->error());
    }
    return success;
}

bool IDOSSimulationCaseTreeGuiProvider::deleteCase(IDOSProject* project, const QString& caseId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || caseId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->object(caseId);
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

bool IDOSSimulationCaseTreeGuiProvider::importGrid(IDOSProject* project, const QString& targetCaseId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || targetCaseId.isEmpty())
    {
        return false;
    }
    IDOSCaseObject* caseObj = qobject_cast<IDOSCaseObject*>(target->object(targetCaseId));
    if (caseObj == nullptr)
    {
        return false;
    }
    // 一个工况仅持有一个网格：已有网格时确认替换
    const QString existingGridId = referencedGridId(caseObj);
    bool replacing = false;
    if (!existingGridId.isEmpty())
    {
        IDOSGrid* existingGrid = qobject_cast<IDOSGrid*>(target->object(existingGridId));
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

    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("targetCaseId"), targetCaseId);
    arguments.insert(QStringLiteral("replaceExistingGrid"), replacing);
    IDOSImportObjectsCommand* command =
        new IDOSImportObjectsCommand(target, QStringLiteral("grid.import"), arguments);
    const bool success = commandManager->execute(command);
    if (!success)
    {
        QMessageBox::warning(parent, tr("Import Grid"), commandManager->error());
    }
    return success;
}