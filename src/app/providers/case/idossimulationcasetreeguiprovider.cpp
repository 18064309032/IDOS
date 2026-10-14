#include <QAction>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPointer>
#include <QSet>
#include <QStringList>
#include <QUuid>
#include <QVariant>

#include "command/idoscasereferencecommand.h"
#include "command/idoscreatecasecommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosimportcommands.h"
#include "command/idosrenameobjectcommand.h"
#include "command/idostransferobjectscommand.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/fluidrock/idosfluidrocktable.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"
#include "data/well/idoswell.h"
#include "idoscasedialog.h"
#include "idoscommandmanager.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "logger/idoslogger.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataproviderregistry.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeview.h"

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
           (qobject_cast<IDOSCaseObject*>(item->tree()->project()->object(item->dataId())) != nullptr ||
            qobject_cast<IDOSFluidRockTable*>(item->tree()->project()->object(item->dataId())) != nullptr);
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
    if (item == nullptr)
    {
        QAction* create = addAction(menu, view, tr("New Case..."));
        create->setObjectName(QStringLiteral("newCaseAction"));
        connect(create, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onNewCase);
        QAction* import = addAction(menu, view, tr("Import Case..."));
        import->setObjectName(QStringLiteral("importCaseAction"));
        connect(import, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onImportCase);
        return;
    }
    if (item->isGroup())
    {
        return;
    }
    if (item->key().startsWith(QStringLiteral("case.curveTable:")) ||
        item->key().startsWith(QStringLiteral("case.fluidRockTable:")))
    {
        QAction* viewCurves = addAction(menu, view, tr("View Property Curves..."));
        viewCurves->setObjectName(QStringLiteral("viewPropertyCurvesAction"));
        connect(viewCurves, &QAction::triggered, this,
                &IDOSSimulationCaseTreeGuiProvider::onViewPropertyCurves);
        return;
    }
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(view->project()->object(item->dataId()));
    if (caseObject != nullptr && item->parent() == view->tree()->root())
    {
        QAction* generate = addAction(menu, view, tr("Generate Simulation Input..."));
        generate->setObjectName(QStringLiteral("generateSimulationInputAction"));
        connect(generate, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onGenerateInput);
        QAction* rename = addAction(menu, view, tr("Rename Case..."));
        rename->setObjectName(QStringLiteral("renameCaseAction"));
        connect(rename, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onRenameCase);
        QAction* remove = addAction(menu, view, tr("Delete Case..."));
        remove->setObjectName(QStringLiteral("deleteCaseAction"));
        connect(remove, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onDeleteCase);
        return;
    }
}

void IDOSSimulationCaseTreeGuiProvider::onGenerateInput()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    const QString caseId = action->property("dataId").toString();
    if (qobject_cast<IDOSCaseObject*>(view->project()->object(caseId)) != nullptr)
    {
        emit generateInputRequested(view->project(), caseId);
    }
}

void IDOSSimulationCaseTreeGuiProvider::onViewPropertyCurves()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    const QString caseId = action->property("caseId").toString();
    IDOSSimulationCaseObject* simulationCase =
        qobject_cast<IDOSSimulationCaseObject*>(view->project()->object(caseId));
    if (simulationCase != nullptr && !simulationCase->curveTables().isEmpty())
    {
        emit propertyCurvesRequested(view->project(), caseId);
    }
}

void IDOSSimulationCaseTreeGuiProvider::onRenameCase()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    const QString caseId = action->property("dataId").toString();
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(view->project()->object(caseId));
    if (caseObject == nullptr)
    {
        return;
    }
    QInputDialog dialog(view);
    dialog.setObjectName(QStringLiteral("renameCaseDialog"));
    dialog.setWindowTitle(tr("Rename Case"));
    dialog.setLabelText(tr("Case name:"));
    dialog.setTextValue(caseObject->name());
    dialog.setInputMode(QInputDialog::TextInput);
    QPointer<IDOSProject> targetProject(view->project());
    if (dialog.exec() != QDialog::Accepted || view->project() == nullptr)
    {
        return;
    }
    const QString newName = dialog.textValue().trimmed();
    if (newName.isEmpty())
    {
        QMessageBox::warning(view, tr("Rename Case"), tr("Please enter a case name."));
        return;
    }
    caseObject = targetProject != nullptr
        ? qobject_cast<IDOSCaseObject*>(targetProject->object(caseId)) : nullptr;
    if (caseObject == nullptr)
    {
        return;
    }
    if (newName.compare(caseObject->name(), Qt::CaseInsensitive) != 0 && caseNameExists(view, newName))
    {
        QMessageBox::warning(view, tr("Rename Case"), tr("A case with this name already exists."));
        return;
    }
    IDOSCommandManager* manager = targetProject != nullptr ? targetProject->commandManager() : nullptr;
    if (manager != nullptr && !manager->execute(new IDOSRenameObjectCommand(targetProject.data(), caseId, newName)))
    {
        QMessageBox::warning(view, tr("Rename Case"), manager->error());
    }
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
    newCase(view);
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
    importCase(view);
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
        importGrid(view, caseId);
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
        deleteCase(view, id);
    }
}


int IDOSSimulationCaseTreeGuiProvider::gridPropertyCount(const IDOSTreeView* view, const QString& gridId) const
{
    if (view == nullptr || view->project() == nullptr || gridId.isEmpty())
    {
        return 0;
    }
    int count = 0;
    for (IDOSDataObject* object : view->project()->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            ++count;
        }
    }
    return count;
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

bool IDOSSimulationCaseTreeGuiProvider::caseNameExists(const IDOSTreeView* view, const QString& name) const
{
    if (view == nullptr || view->project() == nullptr)
    {
        return false;
    }
    for (IDOSDataObject* object : view->project()->objects())
    {
        if (qobject_cast<IDOSCaseObject*>(object) != nullptr &&
            object->name().trimmed().compare(name, Qt::CaseInsensitive) == 0)
        {
            return true;
        }
    }
    return false;
}

bool IDOSSimulationCaseTreeGuiProvider::newCase(IDOSTreeView* view)
{
    if (view == nullptr || view->project() == nullptr)
    {
        return false;
    }
    QPointer<IDOSProject> target(view->project());
    IDOSCaseDialog dialog(QString(), QString(), {}, target, view);
    if (dialog.exec() != QDialog::Accepted || target.isNull())
    {
        return false;
    }
    if (caseNameExists(view, dialog.caseName()))
    {
        QMessageBox::warning(view, tr("New Case"), tr("A case with this name already exists."));
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSCreateCaseCommand(target, dialog.caseName()));
}

bool IDOSSimulationCaseTreeGuiProvider::importCase(IDOSTreeView* view)
{
    if (view == nullptr || view->project() == nullptr)
    {
        return false;
    }
    QPointer<IDOSProject> target(view->project());
    const QString file = QFileDialog::getOpenFileName(view, tr("Select ECLIPSE Case"), QString(),
                                                      tr("ECLIPSE Cases (*.DATA *.data);;All Files (*)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.case.simulation.eclipse"));
    if (!reader)
    {
        IDOS_ERROR(tr("The simulation case reader is unavailable."));
        QMessageBox::warning(view, tr("Import Case"), tr("The simulation case reader is unavailable."));
        return false;
    }
    QList<IDOSDataObject*> objects = reader->read(file);
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
                       .arg(file, reader->lastError()));
        qDeleteAll(objects);
        QMessageBox::warning(view, tr("Import Case"), reader->lastError());
        return false;
    }
    IDOSCaseDialog dialog(simulationCase->name(), simulationCase->sourceFile(), simulationCase->pendingWellNames(),
                          target, view);
    if (dialog.exec() != QDialog::Accepted || target.isNull())
    {
        qDeleteAll(objects);
        return false;
    }
    if (caseNameExists(view, dialog.caseName()))
    {
        IDOS_WARN(tr("Case import rejected because the name already exists: %1")
                      .arg(dialog.caseName()));
        qDeleteAll(objects);
        QMessageBox::warning(view, tr("Import Case"), tr("A case with this name already exists."));
        return false;
    }
    qDeleteAll(objects);

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("caseName"), dialog.caseName());
    IDOSImportObjectsCommand* command =
        new IDOSImportObjectsCommand(target, QStringLiteral("case.import"), arguments);
    const bool success = commandManager->execute(command);
    if (!success)
    {
        QMessageBox::warning(view, tr("Import Case"), commandManager->error());
    }
    return success;
}

bool IDOSSimulationCaseTreeGuiProvider::deleteCase(IDOSTreeView* view, const QString& caseId)
{
    if (view == nullptr || view->project() == nullptr || caseId.isEmpty())
    {
        return false;
    }
    QPointer<IDOSProject> target(view->project());
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
            propertyTotal += gridPropertyCount(view, ref.objectId());
        }
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(view, tr("Delete Case"),
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

bool IDOSSimulationCaseTreeGuiProvider::importGrid(IDOSTreeView* view, const QString& targetCaseId)
{
    if (view == nullptr || view->project() == nullptr || targetCaseId.isEmpty())
    {
        return false;
    }
    QPointer<IDOSProject> target(view->project());
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
        const int propertyCount = gridPropertyCount(view, existingGridId);
        const QMessageBox::StandardButton btn = QMessageBox::question(
            view, tr("Import Grid"),
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
        view, tr("Import Grid"), QString(),
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
        QMessageBox::warning(view, tr("Import Grid"), commandManager->error());
    }
    return success;
}

QMimeData* IDOSSimulationCaseTreeGuiProvider::createMimeData(
    const QList<IDOSTreeItem*>& items, const IDOSTreeView* view) const
{
    if (items.isEmpty() || view == nullptr || view->project() == nullptr || view->tree() == nullptr)
    {
        return nullptr;
    }
    QJsonArray records;
    QSet<QString> ids;
    for (const IDOSTreeItem* item : items)
    {
        if (item == nullptr || item->isGroup() || item->tree() != view->tree() ||
            !item->partKey().isEmpty() || !item->itemKey().isEmpty())
        {
            return nullptr;
        }
        IDOSDataObject* object = view->project()->object(item->dataId());
        if (object == nullptr || !object->canClone() || ids.contains(object->objectId()))
        {
            return nullptr;
        }
        ids.insert(object->objectId());
        QString role;
        QString group;
        if (qobject_cast<IDOSWell*>(object) != nullptr)
        {
            role = QStringLiteral("case.well");
            group = QStringLiteral("data.wells");
        }
        else if (qobject_cast<IDOSFluidRockTable*>(object) != nullptr)
        {
            role = QStringLiteral("case.fluidRockTable");
            IDOSFluidRockTable* table = qobject_cast<IDOSFluidRockTable*>(object);
            group = table->category() == IDOSFluidRockTable::Category::Fluid
                ? QStringLiteral("data.fluids") : QStringLiteral("data.rockPhysics");
        }
        else if (qobject_cast<IDOSGrid*>(object) != nullptr)
        {
            role = QStringLiteral("case.grid");
        }
        if (view->tree()->name() == QStringLiteral("data") && item->parent() != nullptr)
        {
            group = item->parent()->key();
        }
        QJsonObject record;
        record.insert(QStringLiteral("objectId"), object->objectId());
        record.insert(QStringLiteral("role"), role);
        record.insert(QStringLiteral("group"), group);
        record.insert(QStringLiteral("sourceCaseId"), item->caseId());
        records.append(record);
    }
    QString session = view->project()->property("idosTreeTransferSession").toString();
    if (session.isEmpty())
    {
        session = QUuid::createUuid().toString(QUuid::WithoutBraces);
        view->project()->setProperty("idosTreeTransferSession", session);
    }
    QJsonObject payload;
    payload.insert(QStringLiteral("version"), 1);
    payload.insert(QStringLiteral("projectSession"), session);
    payload.insert(QStringLiteral("items"), records);
    QMimeData* mime = new QMimeData();
    mime->setData(QStringLiteral("application/x-idos-objects"), QJsonDocument(payload).toJson(QJsonDocument::Compact));
    return mime;
}

QJsonObject IDOSSimulationCaseTreeGuiProvider::readTransfer(const QMimeData* data, const IDOSTreeView* view) const
{
    if (data == nullptr || view == nullptr || view->project() == nullptr)
    {
        return QJsonObject();
    }
    const QJsonObject payload = QJsonDocument::fromJson(
        data->data(QStringLiteral("application/x-idos-objects"))).object();
    const QString session = view->project()->property("idosTreeTransferSession").toString();
    if (session.isEmpty() || payload.value(QStringLiteral("version")).toInt() != 1 ||
        payload.value(QStringLiteral("projectSession")).toString() != session ||
        payload.value(QStringLiteral("items")).toArray().isEmpty())
    {
        return QJsonObject();
    }
    return payload;
}

QJsonObject IDOSSimulationCaseTreeGuiProvider::transferArguments(
    const QMimeData* data, const IDOSTreeItem* target, const IDOSTreeView* view) const
{
    const QJsonObject payload = readTransfer(data, view);
    if (payload.isEmpty() || view->tree() == nullptr || (target != nullptr && target->tree() != view->tree()))
    {
        return QJsonObject();
    }
    const QJsonArray items = payload.value(QStringLiteral("items")).toArray();
    QString targetCaseId;
    QString targetGridId;
    for (const QJsonValue& value : items)
    {
        const QJsonObject record = value.toObject();
        IDOSDataObject* object = view->project()->object(record.value(QStringLiteral("objectId")).toString());
        if (object == nullptr)
        {
            return QJsonObject();
        }
        if (qobject_cast<IDOSCaseObject*>(object) != nullptr)
        {
            if (view->tree()->name() != QStringLiteral("case") ||
                (target != nullptr && target->parent() != view->tree()->root()))
            {
                return QJsonObject();
            }
            continue;
        }
        if (qobject_cast<IDOSGridProperty*>(object) != nullptr || qobject_cast<IDOSGridFilter*>(object) != nullptr)
        {
            IDOSDataObject* destination = target != nullptr ? view->project()->object(target->dataId()) : nullptr;
            IDOSGrid* grid = qobject_cast<IDOSGrid*>(destination);
            IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(destination);
            if (grid == nullptr && property != nullptr)
            {
                grid = qobject_cast<IDOSGrid*>(view->project()->object(property->gridId()));
            }
            if (grid == nullptr || (target != nullptr && target->isGroup() &&
                target->key() != QStringLiteral("grid.staticProperties") && target->key() != QStringLiteral("grid.regions") &&
                target->key() != QStringLiteral("grid.dynamicProperties") && target->key() != QStringLiteral("grid.filters")))
            {
                return QJsonObject();
            }
            IDOSGridProperty* sourceProperty = qobject_cast<IDOSGridProperty*>(object);
            if (sourceProperty != nullptr && target->isGroup() &&
                ((target->key() == QStringLiteral("grid.dynamicProperties") && sourceProperty->kind() != IDOSGridProperty::Kind::Dynamic) ||
                 (target->key() == QStringLiteral("grid.staticProperties") && sourceProperty->kind() != IDOSGridProperty::Kind::Static) ||
                 target->key() == QStringLiteral("grid.filters")))
            {
                return QJsonObject();
            }
            targetGridId = grid->objectId();
            continue;
        }
        if (view->tree()->name() == QStringLiteral("data"))
        {
            const IDOSTreeItem* group = target;
            while (group != nullptr && group->key() != record.value(QStringLiteral("group")).toString())
            {
                group = group->parent();
            }
            if (target != nullptr && (group == nullptr || group->key() != record.value(QStringLiteral("group")).toString()))
            {
                return QJsonObject();
            }
        }
        else if (view->tree()->name() == QStringLiteral("case"))
        {
            if (target == nullptr || record.value(QStringLiteral("role")).toString().isEmpty())
            {
                return QJsonObject();
            }
            const QString role = record.value(QStringLiteral("role")).toString();
            const bool root = target->parent() == view->tree()->root() && !target->isGroup();
            const bool group = target->isGroup() &&
                ((target->key() == QStringLiteral("case.grids") && role == QStringLiteral("case.grid")) ||
                 (target->key() == QStringLiteral("case.schedule") && role == QStringLiteral("case.well")) ||
                 (target->key() == QStringLiteral("case.fluidRock") && role == QStringLiteral("case.fluidRockTable")));
            if (!root && !group)
            {
                return QJsonObject();
            }
            targetCaseId = target->caseId();
        }
        else
        {
            return QJsonObject();
        }
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("items"), items);
    arguments.insert(QStringLiteral("targetCaseId"), targetCaseId);
    arguments.insert(QStringLiteral("targetGridId"), targetGridId);
    return arguments;
}

bool IDOSSimulationCaseTreeGuiProvider::canCut(const QMimeData* data, const IDOSTreeView* view) const
{
    const QJsonArray items = readTransfer(data, view).value(QStringLiteral("items")).toArray();
    if (items.isEmpty())
    {
        return false;
    }
    for (const QJsonValue& value : items)
    {
        const QJsonObject record = value.toObject();
        IDOSDataObject* object = view->project()->object(record.value(QStringLiteral("objectId")).toString());
        if (object == nullptr || qobject_cast<IDOSCaseObject*>(object) != nullptr ||
            (object->containerId().isEmpty() && record.value(QStringLiteral("sourceCaseId")).toString().isEmpty()))
        {
            return false;
        }
    }
    return true;
}

bool IDOSSimulationCaseTreeGuiProvider::canPaste(const QMimeData* data, const IDOSTreeItem* target,
                                                Qt::DropAction action, const IDOSTreeView* view) const
{
    if (action != Qt::CopyAction && action != Qt::MoveAction)
    {
        return false;
    }
    const QJsonObject arguments = transferArguments(data, target, view);
    if (arguments.isEmpty())
    {
        return false;
    }
    IDOSTransferObjectsCommand command(view->project(),
        action == Qt::MoveAction ? QStringLiteral("move") : QStringLiteral("copy"), arguments);
    QString error;
    return command.validate(error);
}

bool IDOSSimulationCaseTreeGuiProvider::paste(const QMimeData* data, IDOSTreeItem* target,
                                             Qt::DropAction action, IDOSTreeView* view)
{
    if (!canPaste(data, target, action, view) || view->project()->commandManager() == nullptr)
    {
        return false;
    }
    const QJsonObject response = view->project()->commandManager()->execute(
        action == Qt::MoveAction ? QStringLiteral("objects.move") : QStringLiteral("objects.copy"),
        transferArguments(data, target, view));
    if (!response.value(QStringLiteral("success")).toBool())
    {
        QMessageBox::warning(view, tr("Transfer Objects"), response.value(QStringLiteral("error")).toString());
        return false;
    }
    return true;
}

void IDOSSimulationCaseTreeGuiProvider::populateSelectionMenu(QMenu* menu, IDOSTreeView* view)
{
    if (view->menuItem() == nullptr)
    {
        return;
    }
    QMimeData* data = createMimeData(view->selectedItems(), view);
    if (data == nullptr)
    {
        return;
    }
    const QJsonArray items = readTransfer(data, view).value(QStringLiteral("items")).toArray();
    delete data;
    bool canAttach = view->tree()->name() == QStringLiteral("data");
    for (const QJsonValue& value : items)
    {
        const QString role = value.toObject().value(QStringLiteral("role")).toString();
        if (role != QStringLiteral("case.well") && role != QStringLiteral("case.fluidRockTable"))
        {
            canAttach = false;
        }
    }
    if (canAttach)
    {
        QAction* attach = addAction(menu, view, tr("Add to Case..."));
        attach->setObjectName(QStringLiteral("addToCaseAction"));
        attach->setProperty("items", items);
        connect(attach, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onAddToCase);
    }
    if (items.size() == 1)
    {
        IDOSDataObject* object = view->project()->object(items.first().toObject().value(QStringLiteral("objectId")).toString());
        // Case and filter menus already provide rename actions.
        if (qobject_cast<IDOSCaseObject*>(object) == nullptr && qobject_cast<IDOSGridFilter*>(object) == nullptr)
        {
            QAction* rename = addAction(menu, view, tr("Rename..."));
            rename->setObjectName(QStringLiteral("renameObjectAction"));
            connect(rename, &QAction::triggered, this, &IDOSSimulationCaseTreeGuiProvider::onRenameObject);
        }
    }
}

void IDOSSimulationCaseTreeGuiProvider::onAddToCase()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(view->project());
    const QJsonArray records = action->property("items").value<QJsonArray>();
    QStringList labels;
    QStringList ids;
    for (IDOSDataObject* object : project->objects())
    {
        if (qobject_cast<IDOSSimulationCaseObject*>(object) != nullptr)
        {
            labels.append(tr("%1 [%2]").arg(object->name(), object->objectId().left(8)));
            ids.append(object->objectId());
        }
    }
    if (ids.isEmpty())
    {
        QMessageBox::information(view, tr("Add to Case"), tr("Create a case before adding data."));
        return;
    }
    bool accepted = false;
    const QString label = QInputDialog::getItem(view, tr("Add to Case"), tr("Destination case:"), labels, 0, false, &accepted);
    if (!accepted || project == nullptr || project.data() != view->project())
    {
        return;
    }
    QJsonArray items;
    for (const QJsonValue& value : records)
    {
        QJsonObject record = value.toObject();
        record.remove(QStringLiteral("sourceCaseId"));
        items.append(record);
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("items"), items);
    arguments.insert(QStringLiteral("targetCaseId"), ids.value(labels.indexOf(label)));
    if (!project->commandManager()->execute(new IDOSCaseReferenceCommand(project.data(), QStringLiteral("copy"), arguments)))
    {
        QMessageBox::warning(view, tr("Add to Case"), project->commandManager()->error());
    }
}

void IDOSSimulationCaseTreeGuiProvider::onRenameObject()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(view->project());
    const QString id = action->property("dataId").toString();
    IDOSDataObject* object = project->object(id);
    if (object == nullptr)
    {
        return;
    }
    bool accepted = false;
    const QString name = QInputDialog::getText(view, tr("Rename Object"), tr("Name:"), QLineEdit::Normal, object->name(), &accepted);
    if (accepted && project != nullptr && project.data() == view->project() &&
        !project->commandManager()->execute(new IDOSRenameObjectCommand(project.data(), id, name)))
    {
        QMessageBox::warning(view, tr("Rename Object"), project->commandManager()->error());
    }
}
