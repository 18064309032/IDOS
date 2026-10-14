#include <cstring>
#include <memory>

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QScopedPointer>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

#include <vtk_hdf5.h>

#include <DockManager.h>
#include <DockWidget.h>

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
#include "data/case/idoscaseitemref.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/fluidrock/idosfluidrocktable.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"
#include "data/seismic/idosseismicvolume.h"
#include "data/well/idoswell.h"
#include "data/well/idoswelllogset.h"
#include "data/well/idoswellmetadata.h"
#include "idosapplication.h"
#include "idoscommandmanager.h"
#include "idoscommandmetadata.h"
#include "idoscommandregistry.h"
#include "idosdataobject.h"
#include "idosmainwindow.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderserver.h"
#include "idosrenderview.h"
#include "providers/grid/idosgridrenderprovider.h"
#include "providers/idosdatametadata.h"
#include "providers/idosdataregistry.h"
#include "providers/seismic/idosseismicvolumerenderprovider.h"
#include "providers/well/idoswellrenderprovider.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"
#include "tree/idostreeview.h"
#include "well/idoswelllogtrackview.h"

#include "command_registry_test.h"

void CommandRegistryTest::onDataRegistryOwnsRegisteredMetadata()
{
    IDOSDataRegistry registry;
    QSignalSpy added(&registry, &IDOSDataRegistry::added);
    QSignalSpy removed(&registry, &IDOSDataRegistry::removed);
    QVERIFY(!registry.add(nullptr));
    QVERIFY(registry.add(new IDOSWellMetadata()));
    QCOMPARE(added.count(), 1);
    IDOSWellMetadata* duplicate = new IDOSWellMetadata();
    QVERIFY(!registry.add(duplicate));
    delete duplicate;
    const IDOSDataMetadata* metadata = registry.metadata(QStringLiteral("idos.well"));
    QVERIFY(metadata != nullptr);
    IDOSDataObject* data = metadata->create();
    QVERIFY(data != nullptr);
    QCOMPARE(data->typeId(), QStringLiteral("idos.well"));
    delete data;
    QVERIFY(registry.remove(QStringLiteral("idos.well")));
    QCOMPARE(removed.count(), 1);
    QCOMPARE(registry.metadata(QStringLiteral("idos.well")), nullptr);
    QVERIFY(!registry.remove(QStringLiteral("idos.well")));
}

void CommandRegistryTest::onFailedCommandPreservesRedo()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerAllCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    QJsonObject arguments;
    arguments.insert(QStringLiteral("name"), QStringLiteral("History Well"));
    IDOSCommandManager* manager = project.commandManager();
    QVERIFY(manager->execute(QStringLiteral("well.create"), arguments)
                .value(QStringLiteral("success")).toBool());
    manager->undo();
    QVERIFY(manager->canRedo());
    QVERIFY(manager->isClean());
    arguments.insert(QStringLiteral("name"), QString());
    QVERIFY(!manager->execute(QStringLiteral("well.create"), arguments)
                 .value(QStringLiteral("success")).toBool());
    QVERIFY(manager->canRedo());
    QVERIFY(!manager->canUndo());
    QVERIFY(manager->isClean());
    QVERIFY(!manager->error().isEmpty());
    manager->redo();
    QVERIFY(manager->canUndo());
    QVERIFY(!manager->canRedo());
    QVERIFY(project.objectByName(QStringLiteral("History Well")) != nullptr);
}

void CommandRegistryTest::onDefaultRegistryContainsCreateWell()
{
    std::unique_ptr<IDOSCommandRegistry> registry(new IDOSCommandRegistry());
    QVERIFY(registry != nullptr);
    QCOMPARE(registry->toToolJson().size(), 0);
    registerAllCommands(registry.get());

    const IDOSCommandMetadata* metadata = registry->find(QStringLiteral("well.create"));
    QVERIFY(metadata != nullptr);
    QCOMPARE(metadata->name(), QStringLiteral("well.create"));
    QCOMPARE(metadata->type(), IDOSCommand::Type::Action);

    const QJsonObject schema = metadata->schema();
    QCOMPARE(schema.value(QStringLiteral("type")).toString(), QStringLiteral("object"));
    const QJsonObject properties = schema.value(QStringLiteral("properties")).toObject();
    QVERIFY(properties.contains(QStringLiteral("name")));
    const QJsonArray required = schema.value(QStringLiteral("required")).toArray();
    QVERIFY(required.contains(QStringLiteral("name")));

    QJsonArray tools = registry->toToolJson();
    QCOMPARE(tools.size(), 14);

    bool foundDeleteCase = false;
    bool foundDeleteGrid = false;
    bool foundDeleteProperty = false;
    bool foundRenameObject = false;
    bool foundImportCase = false;
    bool foundImportGrid = false;
    bool foundImportProperty = false;
    bool foundImportWellData = false;
    bool foundCreateWell = false;
    bool foundCreateCase = false;
    bool foundDeleteWell = false;
    for (QJsonArray::const_iterator iterator = tools.constBegin();
         iterator != tools.constEnd();
         ++iterator)
    {
        const QJsonObject tool = iterator->toObject();
        QCOMPARE(tool.value(QStringLiteral("type")).toString(), QStringLiteral("function"));
        const QJsonObject functionObject = tool.value(QStringLiteral("function")).toObject();
        QVERIFY(functionObject.contains(QStringLiteral("parameters")));
        const QString toolName = functionObject.value(QStringLiteral("name")).toString();
        if (toolName == QStringLiteral("well.create"))
        {
            foundCreateWell = true;
        }
        else if (toolName == QStringLiteral("case.create"))
        {
            foundCreateCase = true;
        }
        else if (toolName == QStringLiteral("well.delete"))
        {
            foundDeleteWell = true;
        }
        else if (toolName == QStringLiteral("property.delete"))
        {
            foundDeleteProperty = true;
        }
        else if (toolName == QStringLiteral("grid.delete"))
        {
            foundDeleteGrid = true;
        }
        else if (toolName == QStringLiteral("case.delete"))
        {
            foundDeleteCase = true;
        }
        else if (toolName == QStringLiteral("object.rename"))
        {
            foundRenameObject = true;
        }
        else if (toolName == QStringLiteral("case.import"))
        {
            foundImportCase = true;
        }
        else if (toolName == QStringLiteral("grid.import"))
        {
            foundImportGrid = true;
        }
        else if (toolName == QStringLiteral("property.import"))
        {
            foundImportProperty = true;
        }
        else if (toolName == QStringLiteral("wellData.import"))
        {
            foundImportWellData = true;
        }
    }
    QVERIFY(foundCreateWell);
    QVERIFY(foundCreateCase);
    QVERIFY(foundDeleteWell);
    QVERIFY(foundDeleteProperty);
    QVERIFY(foundDeleteGrid);
    QVERIFY(foundDeleteCase);
    QVERIFY(foundRenameObject);
    QVERIFY(foundImportCase);
    QVERIFY(foundImportGrid);
    QVERIFY(foundImportProperty);
    QVERIFY(foundImportWellData);
}

void CommandRegistryTest::onExecuteCreateWellFromJson()
{
    IDOSProject project;
    std::unique_ptr<IDOSCommandRegistry> registry(new IDOSCommandRegistry());
    registerAllCommands(registry.get());
    project.commandManager()->setRegistry(registry.get());

    QJsonObject arguments;
    arguments.insert(QStringLiteral("name"), QStringLiteral("A10"));

    QJsonObject response =
        project.commandManager()->execute(
                                          QStringLiteral("well.create"),
                                          arguments);

    QCOMPARE(response.value(QStringLiteral("success")).toBool(), true);
    const QJsonObject result = response.value(QStringLiteral("result")).toObject();
    const QString objectId = result.value(QStringLiteral("objectId")).toString();
    QVERIFY(!objectId.isEmpty());

    IDOSWell* well = qobject_cast<IDOSWell*>(project.object(objectId));
    QVERIFY(well != nullptr);
    QCOMPARE(well->name(), QStringLiteral("A10"));
    QVERIFY(project.commandManager()->canUndo());

    project.commandManager()->undo();
    QCOMPARE(project.object(objectId), nullptr);
    QVERIFY(project.commandManager()->canRedo());

    project.commandManager()->redo();
    well = qobject_cast<IDOSWell*>(project.object(objectId));
    QVERIFY(well != nullptr);
    QCOMPARE(well->name(), QStringLiteral("A10"));
}

void CommandRegistryTest::onExecuteCreateCaseFromJson()
{
    IDOSProject project;
    std::unique_ptr<IDOSCommandRegistry> registry(new IDOSCommandRegistry());
    registerAllCommands(registry.get());
    project.commandManager()->setRegistry(registry.get());

    QJsonObject arguments;
    arguments.insert(QStringLiteral("name"), QStringLiteral("Base Case"));

    QJsonObject response =
        project.commandManager()->execute(
                                          QStringLiteral("case.create"),
                                          arguments);

    QCOMPARE(response.value(QStringLiteral("success")).toBool(), true);
    const QJsonObject result = response.value(QStringLiteral("result")).toObject();
    const QString objectId = result.value(QStringLiteral("objectId")).toString();
    QVERIFY(!objectId.isEmpty());

    IDOSSimulationCaseObject* caseObject =
        qobject_cast<IDOSSimulationCaseObject*>(project.object(objectId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->name(), QStringLiteral("Base Case"));
    QCOMPARE(caseObject->typeId(), QStringLiteral("idos.case.simulation"));

    project.commandManager()->undo();
    QCOMPARE(project.object(objectId), nullptr);

    project.commandManager()->redo();
    caseObject = qobject_cast<IDOSSimulationCaseObject*>(project.object(objectId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->name(), QStringLiteral("Base Case"));
}

void CommandRegistryTest::onExecuteDeleteWellFromJson()
{
    IDOSProject project;
    std::unique_ptr<IDOSCommandRegistry> registry(new IDOSCommandRegistry());
    registerAllCommands(registry.get());
    project.commandManager()->setRegistry(registry.get());

    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A10"));
    const QString wellId = well->objectId();
    project.add(well);

    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.well"), wellId));
    const QString caseId = caseObject->objectId();
    project.add(caseObject);

    QJsonObject arguments;
    arguments.insert(QStringLiteral("wellId"), wellId);

    QJsonObject response =
        project.commandManager()->execute(
                                          QStringLiteral("well.delete"),
                                          arguments);

    QCOMPARE(response.value(QStringLiteral("success")).toBool(), true);
    QCOMPARE(project.object(wellId), nullptr);

    caseObject = qobject_cast<IDOSSimulationCaseObject*>(project.object(caseId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 0);

    project.commandManager()->undo();
    well = qobject_cast<IDOSWell*>(project.object(wellId));
    QVERIFY(well != nullptr);
    caseObject = qobject_cast<IDOSSimulationCaseObject*>(project.object(caseId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 1);
    QCOMPARE(caseObject->itemRefs().first().objectId(), wellId);

    project.commandManager()->redo();
    QCOMPARE(project.object(wellId), nullptr);
    caseObject = qobject_cast<IDOSSimulationCaseObject*>(project.object(caseId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 0);
}

void CommandRegistryTest::onExecuteDeletePropertyFromJson()
{
    IDOSProject project;
    std::unique_ptr<IDOSCommandRegistry> registry(new IDOSCommandRegistry());
    registerAllCommands(registry.get());
    project.commandManager()->setRegistry(registry.get());

    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(QStringLiteral("PORO"));
    const QString propertyId = property->objectId();
    project.add(property);

    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.gridProperty"), propertyId));
    const QString caseId = caseObject->objectId();
    project.add(caseObject);

    QJsonObject arguments;
    arguments.insert(QStringLiteral("propertyId"), propertyId);

    QJsonObject response =
        project.commandManager()->execute(
                                          QStringLiteral("property.delete"),
                                          arguments);

    QCOMPARE(response.value(QStringLiteral("success")).toBool(), true);
    QCOMPARE(project.object(propertyId), nullptr);
    caseObject = qobject_cast<IDOSSimulationCaseObject*>(project.object(caseId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 0);

    project.commandManager()->undo();
    property = qobject_cast<IDOSGridProperty*>(project.object(propertyId));
    QVERIFY(property != nullptr);
    caseObject = qobject_cast<IDOSSimulationCaseObject*>(project.object(caseId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 1);
    QCOMPARE(caseObject->itemRefs().first().objectId(), propertyId);
}

void CommandRegistryTest::onExecuteDeleteGridFromJson()
{
    IDOSProject project;
    std::unique_ptr<IDOSCommandRegistry> registry(new IDOSCommandRegistry());
    registerAllCommands(registry.get());
    project.commandManager()->setRegistry(registry.get());

    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    const QString gridId = grid->objectId();
    project.add(grid);

    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(QStringLiteral("PORO"));
    property->setGridId(gridId);
    const QString propertyId = property->objectId();
    project.add(property);

    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), gridId));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.gridProperty"), propertyId));
    const QString caseId = caseObject->objectId();
    project.add(caseObject);

    QJsonObject arguments;
    arguments.insert(QStringLiteral("gridId"), gridId);

    QJsonObject response =
        project.commandManager()->execute(
                                          QStringLiteral("grid.delete"),
                                          arguments);

    QCOMPARE(response.value(QStringLiteral("success")).toBool(), true);
    QCOMPARE(project.object(gridId), nullptr);
    QCOMPARE(project.object(propertyId), nullptr);
    caseObject = qobject_cast<IDOSSimulationCaseObject*>(project.object(caseId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 0);

    project.commandManager()->undo();
    QVERIFY(qobject_cast<IDOSGrid*>(project.object(gridId)) != nullptr);
    QVERIFY(qobject_cast<IDOSGridProperty*>(project.object(propertyId)) != nullptr);
    caseObject = qobject_cast<IDOSSimulationCaseObject*>(project.object(caseId));
    QVERIFY(caseObject != nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 2);
}

void CommandRegistryTest::onExecuteDeleteCaseFromJson()
{
    IDOSProject project;
    std::unique_ptr<IDOSCommandRegistry> registry(new IDOSCommandRegistry());
    registerAllCommands(registry.get());
    project.commandManager()->setRegistry(registry.get());

    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    const QString gridId = grid->objectId();
    project.add(grid);

    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(QStringLiteral("PORO"));
    property->setGridId(gridId);
    const QString propertyId = property->objectId();
    project.add(property);

    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), gridId));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.gridProperty"), propertyId));
    const QString caseId = caseObject->objectId();
    project.add(caseObject);

    QJsonObject arguments;
    arguments.insert(QStringLiteral("caseId"), caseId);

    QJsonObject response =
        project.commandManager()->execute(
                                          QStringLiteral("case.delete"),
                                          arguments);

    QCOMPARE(response.value(QStringLiteral("success")).toBool(), true);
    QCOMPARE(project.object(caseId), nullptr);
    QCOMPARE(project.object(gridId), nullptr);
    QCOMPARE(project.object(propertyId), nullptr);

    project.commandManager()->undo();
    QVERIFY(qobject_cast<IDOSSimulationCaseObject*>(project.object(caseId)) != nullptr);
    QVERIFY(qobject_cast<IDOSGrid*>(project.object(gridId)) != nullptr);
    QVERIFY(qobject_cast<IDOSGridProperty*>(project.object(propertyId)) != nullptr);

    project.commandManager()->redo();
    QCOMPARE(project.object(caseId), nullptr);
    QCOMPARE(project.object(gridId), nullptr);
    QCOMPARE(project.object(propertyId), nullptr);
}

void CommandRegistryTest::onExecuteRenameObjectFromJson()
{
    IDOSProject project;
    std::unique_ptr<IDOSCommandRegistry> registry(new IDOSCommandRegistry());
    registerAllCommands(registry.get());
    project.commandManager()->setRegistry(registry.get());

    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A10"));
    const QString wellId = well->objectId();
    project.add(well);

    QJsonObject arguments;
    arguments.insert(QStringLiteral("objectId"), wellId);
    arguments.insert(QStringLiteral("name"), QStringLiteral("B20"));

    QJsonObject response =
        project.commandManager()->execute(
                                          QStringLiteral("object.rename"),
                                          arguments);

    QCOMPARE(response.value(QStringLiteral("success")).toBool(), true);
    well = qobject_cast<IDOSWell*>(project.object(wellId));
    QVERIFY(well != nullptr);
    QCOMPARE(well->name(), QStringLiteral("B20"));

    const QJsonObject result = response.value(QStringLiteral("result")).toObject();
    QCOMPARE(result.value(QStringLiteral("objectId")).toString(), wellId);
    QCOMPARE(result.value(QStringLiteral("oldName")).toString(), QStringLiteral("A10"));
    QCOMPARE(result.value(QStringLiteral("name")).toString(), QStringLiteral("B20"));

    project.commandManager()->undo();
    well = qobject_cast<IDOSWell*>(project.object(wellId));
    QVERIFY(well != nullptr);
    QCOMPARE(well->name(), QStringLiteral("A10"));

    project.commandManager()->redo();
    well = qobject_cast<IDOSWell*>(project.object(wellId));
    QVERIFY(well != nullptr);
    QCOMPARE(well->name(), QStringLiteral("B20"));
}

void CommandRegistryTest::registerAllCommands(IDOSCommandRegistry* registry) const
{
    QVERIFY(registry != nullptr);

    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCaseReferenceCommandMetadata(QStringLiteral("copy"))));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCaseReferenceCommandMetadata(QStringLiteral("move"))));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCaseReferenceCommandMetadata(QStringLiteral("remove"))));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateCaseCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateWellCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteCaseCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteGridCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeletePropertyCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteWellCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSRenameObjectCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportCaseCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportGridCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportPropertyCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportWellDataCommandMetadata()));
}

QTEST_MAIN(CommandRegistryTest)

void CommandRegistryTest::initMain()
{
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
}

CommandRegistryTest::CommandRegistryTest()
    :
    m_observedSource(nullptr)
    , m_observedTarget(nullptr)
    , m_referencesAtomic(true)
    , m_referenceNotifications(0)
{
    IDOSApplication::initProviders();
}

void CommandRegistryTest::onCaseReferencesAtomic()
{
    IDOSCommandRegistry registry;
    registerAllCommands(&registry);
    IDOSProject project;
    project.commandManager()->setRegistry(&registry);
    IDOSWell* well = new IDOSWell();
    IDOSFluidRockTable* table = new IDOSFluidRockTable();
    IDOSSimulationCaseObject* first = new IDOSSimulationCaseObject();
    IDOSSimulationCaseObject* second = new IDOSSimulationCaseObject();
    project.add(well);
    project.add(table);
    project.add(first);
    project.add(second);
    QJsonObject wellItem{{QStringLiteral("objectId"), well->objectId()},
                         {QStringLiteral("role"), QStringLiteral("case.well")}};
    QJsonObject tableItem{{QStringLiteral("objectId"), table->objectId()},
                          {QStringLiteral("role"), QStringLiteral("case.fluidRockTable")}};
    QJsonObject arguments{{QStringLiteral("targetCaseId"), first->objectId()},
                          {QStringLiteral("items"), QJsonArray{wellItem, tableItem}}};
    QVERIFY(project.commandManager()->execute(QStringLiteral("case.references.copy"), arguments)
                .value(QStringLiteral("success")).toBool());
    QCOMPARE(first->itemRefs().size(), 2);
    QCOMPARE(project.objects().size(), 4);
    QVERIFY(!first->itemRefs().first().refId().isEmpty());
    const QString wellReferenceId = first->itemRefs().first().refId();
    project.commandManager()->undo();
    QVERIFY(first->itemRefs().isEmpty());
    QVERIFY(project.commandManager()->canRedo());
    QJsonObject invalid = tableItem;
    invalid.insert(QStringLiteral("objectId"), QStringLiteral("missing"));
    arguments.insert(QStringLiteral("items"), QJsonArray{wellItem, invalid});
    QVERIFY(!project.commandManager()->execute(QStringLiteral("case.references.copy"), arguments)
                 .value(QStringLiteral("success")).toBool());
    QVERIFY(first->itemRefs().isEmpty());
    QVERIFY(project.commandManager()->canRedo());
    project.commandManager()->redo();
    QCOMPARE(first->itemRefs().first().refId(), wellReferenceId);
    wellItem.insert(QStringLiteral("sourceCaseId"), first->objectId());
    wellItem.insert(QStringLiteral("refId"), wellReferenceId);
    tableItem.insert(QStringLiteral("sourceCaseId"), first->objectId());
    tableItem.insert(QStringLiteral("refId"), first->itemRefs().last().refId());
    arguments.insert(QStringLiteral("targetCaseId"), second->objectId());
    invalid = tableItem;
    invalid.insert(QStringLiteral("refId"), QStringLiteral("stale-reference"));
    arguments.insert(QStringLiteral("items"), QJsonArray{wellItem, invalid});
    QVERIFY(!project.commandManager()->execute(QStringLiteral("case.references.move"), arguments)
                 .value(QStringLiteral("success")).toBool());
    QCOMPARE(first->itemRefs().size(), 2);
    QVERIFY(second->itemRefs().isEmpty());
    arguments.insert(QStringLiteral("items"), QJsonArray{wellItem, tableItem});
    m_observedSource = first;
    m_observedTarget = second;
    m_referencesAtomic = true;
    m_referenceNotifications = 0;
    connect(first, &IDOSDataObject::dataChanged, this, &CommandRegistryTest::onReferencesPublished);
    connect(second, &IDOSDataObject::dataChanged, this, &CommandRegistryTest::onReferencesPublished);
    QVERIFY(project.commandManager()->execute(QStringLiteral("case.references.move"), arguments)
                .value(QStringLiteral("success")).toBool());
    disconnect(first, &IDOSDataObject::dataChanged, this, &CommandRegistryTest::onReferencesPublished);
    disconnect(second, &IDOSDataObject::dataChanged, this, &CommandRegistryTest::onReferencesPublished);
    QVERIFY(m_referencesAtomic);
    QCOMPARE(m_referenceNotifications, 2);
    m_observedSource = nullptr;
    m_observedTarget = nullptr;
    QVERIFY(first->itemRefs().isEmpty());
    QCOMPARE(second->itemRefs().size(), 2);
    QCOMPARE(second->itemRefs().first().refId(), wellReferenceId);
    project.commandManager()->undo();
    QCOMPARE(first->itemRefs().size(), 2);
    QVERIFY(second->itemRefs().isEmpty());
    QVERIFY(project.commandManager()->canRedo());
    arguments.insert(QStringLiteral("targetCaseId"), first->objectId());
    QVERIFY(!project.commandManager()->execute(QStringLiteral("case.references.copy"), arguments)
                 .value(QStringLiteral("success")).toBool());
    QVERIFY(project.commandManager()->canRedo());
    project.commandManager()->redo();
    wellItem.insert(QStringLiteral("sourceCaseId"), second->objectId());
    tableItem.insert(QStringLiteral("sourceCaseId"), second->objectId());
    arguments = QJsonObject{{QStringLiteral("items"), QJsonArray{wellItem, tableItem}}};
    QVERIFY(project.commandManager()->execute(QStringLiteral("case.references.remove"), arguments)
                .value(QStringLiteral("success")).toBool());
    QVERIFY(second->itemRefs().isEmpty());
    QCOMPARE(project.objects().size(), 4);
    project.commandManager()->undo();
    QCOMPARE(second->itemRefs().size(), 2);
    QVERIFY(project.object(well->objectId()) == well);
    QVERIFY(project.object(table->objectId()) == table);
}

void CommandRegistryTest::onCaseReferenceParts()
{
    IDOSCommandRegistry registry;
    registerAllCommands(&registry);
    IDOSProject project;
    project.commandManager()->setRegistry(&registry);
    IDOSWell* well = new IDOSWell();
    IDOSSimulationCaseObject* source = new IDOSSimulationCaseObject();
    IDOSSimulationCaseObject* target = new IDOSSimulationCaseObject();
    project.add(well);
    project.add(source);
    project.add(target);
    IDOSCaseItemRef first(QStringLiteral("case.well"), well->objectId());
    first.setPartKey(QStringLiteral("idos.well.logs"));
    first.setItemKey(QStringLiteral("GR"));
    first.setRefId(QStringLiteral("source-gr"));
    IDOSCaseItemRef second = first;
    second.setItemKey(QStringLiteral("RHOB"));
    second.setRefId(QStringLiteral("source-rhob"));
    source->setItemRefs(QList<IDOSCaseItemRef>{first, second});
    QJsonObject record{{QStringLiteral("objectId"), well->objectId()},
                       {QStringLiteral("role"), QStringLiteral("case.well")},
                       {QStringLiteral("sourceCaseId"), source->objectId()},
                       {QStringLiteral("refId"), first.refId()},
                       {QStringLiteral("partKey"), first.partKey()},
                       {QStringLiteral("itemKey"), first.itemKey()}};
    QJsonObject arguments{{QStringLiteral("targetCaseId"), target->objectId()},
                          {QStringLiteral("items"), QJsonArray{record}}};
    QVERIFY(project.commandManager()->execute(QStringLiteral("case.references.copy"), arguments)
                .value(QStringLiteral("success")).toBool());
    QCOMPARE(target->itemRefs().size(), 1);
    QCOMPARE(target->itemRefs().first().itemKey(), QStringLiteral("GR"));
    QVERIFY(target->itemRefs().first().refId() != first.refId());
    QCOMPARE(source->itemRefs().size(), 2);
    arguments.insert(QStringLiteral("items"), QJsonArray{record, record});
    QVERIFY(!project.commandManager()->execute(QStringLiteral("case.references.move"), arguments)
                 .value(QStringLiteral("success")).toBool());
    QCOMPARE(source->itemRefs().size(), 2);
    QCOMPARE(target->itemRefs().size(), 1);
    const QList<IDOSCaseItemRef> expected = target->itemRefs();
    target->clearItemRefs();
    project.commandManager()->undo();
    QVERIFY(!project.commandManager()->error().isEmpty());
    QVERIFY(project.commandManager()->canUndo());
    target->setItemRefs(expected);
    project.commandManager()->undo();
    QVERIFY(target->itemRefs().isEmpty());
    QVERIFY(!project.commandManager()->canUndo());
    const QString wellId = well->objectId();
    project.remove(wellId);
    project.commandManager()->redo();
    QVERIFY(!project.commandManager()->error().isEmpty());
    QVERIFY(target->itemRefs().isEmpty());
    QVERIFY(project.commandManager()->canRedo());
}

void CommandRegistryTest::onReferencesPublished()
{
    ++m_referenceNotifications;
    if (m_observedSource == nullptr || m_observedTarget == nullptr ||
        !m_observedSource->itemRefs().isEmpty() || m_observedTarget->itemRefs().size() != 2)
    {
        m_referencesAtomic = false;
    }
}
void CommandRegistryTest::onObjectCopies()
{
    IDOSProject project;
    IDOSSimulationCaseObject* source = new IDOSSimulationCaseObject();
    source->setName(QStringLiteral("Source"));
    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Grid"));
    grid->setDimensions(1, 1, 1);
    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(QStringLiteral("PORO"));
    property->setKeyword(QStringLiteral("PORO"));
    property->setGridId(grid->objectId());
    property->setDimensions(1, 1, 1);
    QVERIFY(property->setValues(QVector<double>{0.2}));
    IDOSGridFilter* filter = new IDOSGridFilter();
    filter->setGridId(grid->objectId());
    QVariantMap condition;
    condition.insert(QStringLiteral("propertyId"), property->objectId());
    condition.insert(QStringLiteral("minimum"), 0.0);
    condition.insert(QStringLiteral("maximum"), 1.0);
    condition.insert(QStringLiteral("operation"), QStringLiteral("between"));
    QVariantList conditions;
    conditions.append(condition);
    QVariantMap criteria;
    criteria.insert(QStringLiteral("conditions"), conditions);
    QVERIFY(filter->setCriteria(criteria));
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("Well"));
    well->setReferenceDepth(123.0);
    project.add(source);
    project.add(grid);
    project.add(property);
    project.add(filter);
    project.add(well);
    source->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    source->addItemRef(IDOSCaseItemRef(QStringLiteral("case.well"), well->objectId()));
    QVariantMap run;
    run.insert(QStringLiteral("outputDirectory"), QStringLiteral("old-output"));
    run.insert(QStringLiteral("status"), QStringLiteral("running"));
    source->setRunInfo(run);
    QJsonObject item;
    item.insert(QStringLiteral("objectId"), source->objectId());
    QJsonArray items;
    items.append(item);
    QJsonObject arguments;
    arguments.insert(QStringLiteral("items"), items);
    QVERIFY(project.commandManager()->execute(new IDOSTransferObjectsCommand(&project, QStringLiteral("copy"), arguments)));
    QCOMPARE(project.objects().size(), 10);
    IDOSSimulationCaseObject* copy = nullptr;
    for (IDOSDataObject* object : project.objects())
    {
        if (object != source && qobject_cast<IDOSSimulationCaseObject*>(object) != nullptr)
        {
            copy = qobject_cast<IDOSSimulationCaseObject*>(object);
        }
    }
    QVERIFY(copy != nullptr);
    QVERIFY(copy->objectId() != source->objectId());
    QVERIFY(copy->runInfo().isEmpty());
    const QString copyId = copy->objectId();
    QString copyGridId;
    QString copyWellId;
    for (const IDOSCaseItemRef& ref : copy->itemRefs())
    {
        if (ref.role() == QStringLiteral("case.grid"))
        {
            copyGridId = ref.objectId();
        }
        else if (ref.role() == QStringLiteral("case.well"))
        {
            copyWellId = ref.objectId();
        }
        QVERIFY(!ref.refId().isEmpty());
    }
    QVERIFY(copyGridId != grid->objectId());
    QVERIFY(copyWellId != well->objectId());
    IDOSGridProperty* copiedProperty = nullptr;
    IDOSGridFilter* copiedFilter = nullptr;
    for (IDOSDataObject* object : project.objects())
    {
        if (object->containerId() == copyGridId)
        {
            if (qobject_cast<IDOSGridProperty*>(object) != nullptr)
            {
                copiedProperty = qobject_cast<IDOSGridProperty*>(object);
            }
            else if (qobject_cast<IDOSGridFilter*>(object) != nullptr)
            {
                copiedFilter = qobject_cast<IDOSGridFilter*>(object);
            }
        }
    }
    QVERIFY(copiedProperty != nullptr && copiedFilter != nullptr);
    const QStringList copiedPropertyIds = IDOSGridFilter::propertyIds(copiedFilter->criteria());
    QCOMPARE(copiedPropertyIds.size(), 1);
    QCOMPARE(copiedPropertyIds.first(), copiedProperty->objectId());
    copiedProperty->setValueAt(0, 0, 0, 0.8);
    QCOMPARE(property->valueAt(0, 0, 0), 0.2);
    IDOSWell* copiedWell = qobject_cast<IDOSWell*>(project.object(copyWellId));
    QVERIFY(copiedWell != nullptr);
    copiedWell->setReferenceDepth(999.0);
    QCOMPARE(well->referenceDepth(), 123.0);
    project.commandManager()->undo();
    QCOMPARE(project.objects().size(), 5);
    project.commandManager()->redo();
    QCOMPARE(project.objects().size(), 10);
    QVERIFY(project.object(copyId) != nullptr);
    // Deleting a case also removes filters under its private copied grid.
    QVERIFY(project.commandManager()->execute(new IDOSDeleteCaseCommand(&project, copyId)));
    QVERIFY(project.object(copiedFilter->objectId()) == nullptr);
    QVERIFY(project.object(grid->objectId()) == grid);
    project.commandManager()->undo();
    QCOMPARE(project.objects().size(), 10);
}

void CommandRegistryTest::onPropertyMoves()
{
    IDOSProject project;
    IDOSGrid* first = new IDOSGrid();
    IDOSGrid* second = new IDOSGrid();
    IDOSGrid* incompatible = new IDOSGrid();
    first->setDimensions(1, 1, 1);
    second->setDimensions(1, 1, 1);
    incompatible->setDimensions(2, 1, 1);
    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(QStringLiteral("Property"));
    property->setGridId(first->objectId());
    property->setDimensions(1, 1, 1);
    property->setValues(QVector<double>{0.2});
    project.add(first);
    project.add(second);
    project.add(incompatible);
    project.add(property);
    QJsonObject item;
    item.insert(QStringLiteral("objectId"), property->objectId());
    QJsonArray items;
    items.append(item);
    QJsonObject arguments;
    arguments.insert(QStringLiteral("items"), items);
    arguments.insert(QStringLiteral("targetGridId"), second->objectId());
    QVERIFY(project.commandManager()->execute(new IDOSTransferObjectsCommand(&project, QStringLiteral("move"), arguments)));
    QCOMPARE(property->gridId(), second->objectId());
    QCOMPARE(project.objects().size(), 4);
    project.commandManager()->undo();
    QCOMPARE(property->gridId(), first->objectId());
    arguments.insert(QStringLiteral("targetGridId"), incompatible->objectId());
    QVERIFY(!project.commandManager()->execute(new IDOSTransferObjectsCommand(&project, QStringLiteral("move"), arguments)));
    QVERIFY(project.commandManager()->canRedo());
    QCOMPARE(property->gridId(), first->objectId());
    project.commandManager()->redo();
    QCOMPARE(property->gridId(), second->objectId());
    QVERIFY(project.commandManager()->execute(new IDOSRenameObjectCommand(&project, property->objectId(), QStringLiteral("Renamed"))));
    QCOMPARE(property->name(), QStringLiteral("Renamed"));
    project.commandManager()->undo();
    QCOMPARE(property->name(), QStringLiteral("Property"));
}
bool CommandRegistryTest::sameProjectValue(const QVariant& first, const QVariant& second) const
{
    if (first.userType() != second.userType())
    {
        return false;
    }
    if (first.type() == QVariant::Map)
    {
        const QVariantMap a = first.toMap();
        const QVariantMap b = second.toMap();
        if (a.size() != b.size())
        {
            return false;
        }
        for (QVariantMap::const_iterator it = a.constBegin(); it != a.constEnd(); ++it)
        {
            if (!b.contains(it.key()) || !sameProjectValue(it.value(), b.value(it.key())))
            {
                return false;
            }
        }
        return true;
    }
    if (first.type() == QVariant::List)
    {
        const QVariantList a = first.toList();
        const QVariantList b = second.toList();
        if (a.size() != b.size())
        {
            return false;
        }
        for (int index = 0; index < a.size(); ++index)
        {
            if (!sameProjectValue(a.at(index), b.at(index)))
            {
                return false;
            }
        }
        return true;
    }
    if (first.userType() == qMetaTypeId<QVector<double>>())
    {
        const QVector<double> a = first.value<QVector<double>>();
        const QVector<double> b = second.value<QVector<double>>();
        return a.size() == b.size() && (a.isEmpty() || std::memcmp(a.constData(), b.constData(), a.size() * sizeof(double)) == 0);
    }
    if (first.userType() == qMetaTypeId<QVector<float>>())
    {
        const QVector<float> a = first.value<QVector<float>>();
        const QVector<float> b = second.value<QVector<float>>();
        return a.size() == b.size() && (a.isEmpty() || std::memcmp(a.constData(), b.constData(), a.size() * sizeof(float)) == 0);
    }
    if (first.userType() == qMetaTypeId<QVector<int>>())
    {
        const QVector<int> a = first.value<QVector<int>>();
        const QVector<int> b = second.value<QVector<int>>();
        return a.size() == b.size() && (a.isEmpty() || std::memcmp(a.constData(), b.constData(), a.size() * sizeof(int)) == 0);
    }
    if (first.userType() == qMetaTypeId<QVector<qint64>>())
    {
        const QVector<qint64> a = first.value<QVector<qint64>>();
        const QVector<qint64> b = second.value<QVector<qint64>>();
        return a.size() == b.size() && (a.isEmpty() || std::memcmp(a.constData(), b.constData(), a.size() * sizeof(qint64)) == 0);
    }
    return first == second;
}

void CommandRegistryTest::onProjectStorageRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QString::fromUtf8("\xE5\xB7\xA5\xE7\xA8\x8B.idos"));
    IDOSDataRegistry registry;
    IDOSApplication::initDataRegistry(&registry);
    IDOSProject project;
    IDOSProjectMetadata metadata;
    metadata.setName(QStringLiteral("Storage round trip"));
    metadata.setDescription(QStringLiteral("All platform data types, no source file dependency"));
    metadata.setCoordinateType(IDOSProjectMetadata::CoordinateType::Custom);
    metadata.setCoordinateReference(QStringLiteral("EPSG:32650"));
    QVERIFY(project.setMetadata(metadata));
    QHash<QString, IDOSDataObject*> objects;
    for (const IDOSDataMetadata* dataMetadata : registry.metadata())
    {
        IDOSDataObject* object = dataMetadata->create();
        object->setName(dataMetadata->typeId());
        QVariantMap fields = object->projectData();
        if (fields.contains(QStringLiteral("sourceFile")))
        {
            fields.insert(QStringLiteral("sourceFile"), directory.filePath(QStringLiteral("nonexistent-original.dat")));
        }
        QString error;
        QVERIFY2(object->restoreProjectData(fields, error), qPrintable(error));
        project.add(object);
        objects.insert(object->typeId(), object);
    }
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(objects.value(QStringLiteral("idos.grid")));
    QVERIFY(grid != nullptr);
    grid->setCornerPointData(1, 1, 1, QVector<double>(24, 0.0), QVector<double>{0, 0, 0, 0, 1, 1, 1, 1}, QVector<int>{1});
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(objects.value(QStringLiteral("idos.gridproperty")));
    QVERIFY(property != nullptr);
    property->setGridId(grid->objectId());
    property->setDimensions(1, 1, 1);
    property->setKeyword(QStringLiteral("PORO"));
    QVERIFY(property->setValues(QVector<double>{0.25}));
    IDOSGridProperty* dynamic = new IDOSGridProperty();
    dynamic->setName(QStringLiteral("Pressure"));
    dynamic->setKind(IDOSGridProperty::Kind::Dynamic);
    dynamic->setGridId(grid->objectId());
    dynamic->setDimensions(1, 1, 1);
    QVERIFY(dynamic->setReportStepValues(0, QVector<double>{100.0}));
    QVERIFY(dynamic->setReportStepValues(10, QVector<double>{200.0}));
    QVERIFY(dynamic->setCurrentReportStep(10));
    project.add(dynamic);
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(objects.value(QStringLiteral("idos.grid.filter")));
    filter->setGridId(grid->objectId());
    QVariantMap condition;
    condition.insert(QStringLiteral("propertyId"), property->objectId());
    condition.insert(QStringLiteral("operation"), QStringLiteral("between"));
    condition.insert(QStringLiteral("minimum"), 0.0);
    condition.insert(QStringLiteral("maximum"), 1.0);
    QVariantList conditions;
    conditions.append(condition);
    QVariantMap criteria;
    criteria.insert(QStringLiteral("conditions"), conditions);
    QVERIFY(filter->setCriteria(criteria));
    IDOSWell* well = qobject_cast<IDOSWell*>(objects.value(QStringLiteral("idos.well")));
    IDOSWellHead head;
    head.setSurfaceX(123.5);
    head.setSurfaceY(456.25);
    well->setWellHead(head);
    IDOSWellPath wellPath;
    wellPath.setPoints(QVector<IDOSWellPathPoint>{IDOSWellPathPoint(0.0, 123.5, 456.25, 10.0, 0.0),
        IDOSWellPathPoint(100.0, 130.0, 460.0, -90.0, 100.0)});
    wellPath.setSourceComments(QStringList{QStringLiteral("First comment"), QStringLiteral("Second comment")});
    well->setPath(wellPath);
    IDOSWellLogChannel channel(QStringLiteral("GR"));
    channel.setData(QVector<double>{1.0, 2.0}, QVector<double>{qQNaN(), 12.5});
    IDOSWellLogSet logs;
    logs.addChannel(channel);
    well->setLogs(logs);
    IDOSSeismicVolume* seismic = qobject_cast<IDOSSeismicVolume*>(objects.value(QStringLiteral("idos.seismic.volume")));
    QVERIFY(seismic != nullptr);
    seismic->setTraceCount(1);
    seismic->setTraceSampleOffsets(QVector<qint64>{0});
    seismic->setTraceSampleCounts(QVector<int>{4096});
    seismic->setTraceInlineNumbers(QVector<int>{10});
    seismic->setTraceCrosslineNumbers(QVector<int>{20});
    QVector<float> samples(4096, 2.5f);
    samples[0] = 1.25f;
    samples[1] = qQNaN();
    samples[4095] = 3.5f;
    seismic->setSampleValues(samples);
    IDOSSimulationCaseObject* simulation = qobject_cast<IDOSSimulationCaseObject*>(objects.value(QStringLiteral("idos.case.simulation")));
    QVERIFY(simulation != nullptr);
    simulation->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    simulation->addItemRef(IDOSCaseItemRef(QStringLiteral("case.well"), well->objectId()));
    QVariantMap sourceData;
    sourceData.insert(QStringLiteral("emptyList"), QVariantList());
    sourceData.insert(QStringLiteral("time"), QDateTime::fromMSecsSinceEpoch(123456789, Qt::UTC));
    sourceData.insert(QStringLiteral("bytes"), QByteArray("a\0b", 3));
    sourceData.insert(QStringLiteral("largeInteger"), QVariant::fromValue<quint64>(18446744073709551615ULL));
    simulation->setParsedSourceData(sourceData);
    project.setCurrentCaseId(simulation->objectId());
    project.setCurrentObject(well);
    QString error;
    QVERIFY2(project.save(path, &registry, error), qPrintable(error));
    QVERIFY(!project.isModified());
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.read(8), QByteArray::fromHex("894844460d0a1a0a"));
    file.close();
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(path, &registry, error));
    QVERIFY2(loaded != nullptr, qPrintable(error));
    QCOMPARE(loaded->objects().size(), project.objects().size());
    QCOMPARE(loaded->currentCaseId(), simulation->objectId());
    QCOMPARE(loaded->currentObject()->objectId(), well->objectId());
    QVERIFY(sameProjectValue(loaded->metadata().projectData(), project.metadata().projectData()));
    for (IDOSDataObject* original : project.objects())
    {
        IDOSDataObject* restored = loaded->object(original->objectId());
        QVERIFY(restored != nullptr);
        QCOMPARE(restored->typeId(), original->typeId());
        QCOMPARE(restored->name(), original->name());
        QCOMPARE(restored->isVisible(), original->isVisible());
        QVERIFY2(sameProjectValue(restored->projectData(), original->projectData()), qPrintable(original->typeId()));
    }
    QVERIFY(!loaded->commandManager()->canUndo());
    QVERIFY(!loaded->isModified());
    loaded->object(well->objectId())->setName(QStringLiteral("Renamed after reopening"));
    QVERIFY(loaded->isModified());
    QVERIFY2(loaded->save(path, &registry, error), qPrintable(error));
}

void CommandRegistryTest::onProjectStorageFailureProtection()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("project.idos"));
    IDOSDataRegistry registry;
    IDOSApplication::initDataRegistry(&registry);
    IDOSProject project;
    IDOSSimulationCaseObject* simulation = new IDOSSimulationCaseObject();
    project.add(simulation);
    QString error;
    QVERIFY2(project.save(path, &registry, error), qPrintable(error));
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray original = file.readAll();
    file.close();
    QVariantMap invalid;
    invalid.insert(QStringLiteral("unsupported"), QVariant::fromValue<QObject*>(&project));
    simulation->setParsedSourceData(invalid);
    QVERIFY(!project.save(path, &registry, error));
    QVERIFY(!error.isEmpty());
    QVERIFY(project.isModified());
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), original);
    file.close();
    IDOSDataRegistry emptyRegistry;
    QVERIFY(IDOSProject::load(path, &emptyRegistry, error) == nullptr);
    QCOMPARE(project.objects().size(), 1);
    const QString brokenPath = directory.filePath(QStringLiteral("broken.idos"));
    QFile broken(brokenPath);
    QVERIFY(broken.open(QIODevice::WriteOnly));
    broken.write(original.left(original.size() / 2));
    broken.close();
    QVERIFY(IDOSProject::load(brokenPath, &registry, error) == nullptr);
    QVERIFY(!error.isEmpty());
    const QString futurePath = directory.filePath(QStringLiteral("future.idos"));
    const hid_t future = H5Fcreate(QFile::encodeName(futurePath).constData(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    QVERIFY(future >= 0);
    QVariantMap header;
    header.insert(QStringLiteral("format"), QStringLiteral("IDOS"));
    header.insert(QStringLiteral("version"), 999);
    QVERIFY(IDOSDataMetadata::writeValue(future, "IDOS", header, error));
    QVERIFY(H5Fclose(future) >= 0);
    QVERIFY(IDOSProject::load(futurePath, &registry, error) == nullptr);
    QVERIFY(error.contains(QStringLiteral("version")));
    simulation->setParsedSourceData(QVariantMap());
    const QString blockedPath = directory.filePath(QStringLiteral("blocked.idos"));
    QVERIFY(QDir().mkpath(blockedPath));
    QFile sentinel(blockedPath + QStringLiteral("/sentinel"));
    QVERIFY(sentinel.open(QIODevice::WriteOnly));
    QCOMPARE(sentinel.write("preserved"), qint64(9));
    sentinel.close();
    QVERIFY(!project.save(blockedPath, &registry, error));
    QVERIFY(sentinel.open(QIODevice::ReadOnly));
    QCOMPARE(sentinel.readAll(), QByteArray("preserved"));
    QCOMPARE(project.filePath(), path);
}
void CommandRegistryTest::onProjectWindowStorage()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("window.idos"));
    IDOSMainWindow window;
    IDOSProject source;
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("Saved well"));
    source.add(well);
    const QString id = well->objectId();
    window.setProject(&source);
    QVERIFY(window.findChild<QAction*>(QStringLiteral("openProjectAction"))->isEnabled());
    QVERIFY(window.findChild<QAction*>(QStringLiteral("saveProjectAction"))->isEnabled());
    QString error;
    QVERIFY2(source.save(path, window.dataRegistry(), error), qPrintable(error));
    QVERIFY(window.saveProject(path));
    QVERIFY(!window.isWindowModified());
    well->setName(QStringLiteral("Unsaved name"));
    QVERIFY(window.isWindowModified());
    QVERIFY(window.openProject(path, false));
    QVERIFY(window.project() != &source);
    QVERIFY(window.project()->object(id) != nullptr);
    QCOMPARE(window.project()->object(id)->name(), QStringLiteral("Saved well"));
    QVERIFY(window.dataTreeModel()->indexFromDataId(id).isValid());
    QVERIFY(!window.project()->isModified());
    QVERIFY(!window.isWindowModified());
}
void CommandRegistryTest::onStoragePerformance()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = qEnvironmentVariableIsSet("IDOS_STORAGE_BENCHMARK_PATH")
        ? qEnvironmentVariable("IDOS_STORAGE_BENCHMARK_PATH") : directory.filePath(QStringLiteral("performance.idos"));
    IDOSDataRegistry registry;
    IDOSApplication::initDataRegistry(&registry);
    IDOSProject project;
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("Benchmark well"));
    QVector<IDOSWellPathPoint> points;
    points.reserve(2000);
    for (int index = 0; index < 2000; ++index)
    {
        points.append(IDOSWellPathPoint(index, index * 0.25, index * 0.5, -index, index));
    }
    IDOSWellPath trajectory;
    trajectory.setPoints(points);
    well->setPath(trajectory);
    project.add(well);
    IDOSSimulationCaseObject* simulation = new IDOSSimulationCaseObject();
    simulation->addItemRef(IDOSCaseItemRef(QStringLiteral("case.well"), well->objectId()));
    QVariantList rows;
    for (int index = 0; index < 2000; ++index)
    {
        QVariantMap row;
        row.insert(QStringLiteral("step"), index);
        row.insert(QStringLiteral("rate"), index * 1.25);
        row.insert(QStringLiteral("well"), well->name());
        rows.append(row);
    }
    simulation->setWellControlRows(rows);
    project.add(simulation);
    QString error;
    QElapsedTimer timer;
    timer.start();
    QVERIFY2(project.save(path, &registry, error), qPrintable(error));
    const qint64 saveMs = timer.elapsed();
    timer.restart();
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(path, &registry, error));
    QVERIFY2(loaded != nullptr, qPrintable(error));
    const qint64 loadMs = timer.elapsed();
    QVERIFY(sameProjectValue(well->projectData(), loaded->object(well->objectId())->projectData()));
    QVERIFY(sameProjectValue(simulation->projectData(), loaded->object(simulation->objectId())->projectData()));
    qInfo().noquote() << QStringLiteral("Storage benchmark: save=%1 ms load=%2 ms bytes=%3")
        .arg(saveMs).arg(loadMs).arg(QFileInfo(path).size());
}

void CommandRegistryTest::onLegacyProjectStorage()
{
    const QString path = qEnvironmentVariable("IDOS_STORAGE_LEGACY_PATH");
    if (path.isEmpty())
    {
        return;
    }
    IDOSDataRegistry registry;
    IDOSApplication::initDataRegistry(&registry);
    QString error;
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(path, &registry, error));
    QVERIFY2(loaded != nullptr, qPrintable(error));
    QCOMPARE(loaded->objects().size(), 2);
    bool foundWell = false;
    for (IDOSDataObject* object : loaded->objects())
    {
        IDOSWell* well = qobject_cast<IDOSWell*>(object);
        if (well != nullptr)
        {
            QCOMPARE(well->path().pointCount(), 2000);
            foundWell = true;
        }
    }
    QVERIFY(foundWell);
}
void CommandRegistryTest::onExistingProjectPerformance()
{
    const QString input = qEnvironmentVariable("IDOS_STORAGE_INPUT_PATH");
    const QString output = qEnvironmentVariable("IDOS_STORAGE_OUTPUT_PATH");
    if (input.isEmpty() || output.isEmpty())
    {
        return;
    }
    QVERIFY(QFileInfo(input).absoluteFilePath() != QFileInfo(output).absoluteFilePath());
    IDOSDataRegistry registry;
    IDOSApplication::initDataRegistry(&registry);
    QString error;
    QElapsedTimer timer;
    timer.start();
    QScopedPointer<IDOSProject> original(IDOSProject::load(input, &registry, error));
    QVERIFY2(original != nullptr, qPrintable(error));
    const qint64 oldLoadMs = timer.elapsed();
    timer.restart();
    QVERIFY2(original->save(output, &registry, error), qPrintable(error));
    const qint64 saveMs = timer.elapsed();
    timer.restart();
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(output, &registry, error));
    QVERIFY2(loaded != nullptr, qPrintable(error));
    const qint64 loadMs = timer.elapsed();
    QCOMPARE(loaded->objects().size(), original->objects().size());
    QVERIFY(sameProjectValue(loaded->metadata().projectData(), original->metadata().projectData()));
    for (IDOSDataObject* object : original->objects())
    {
        IDOSDataObject* restored = loaded->object(object->objectId());
        QVERIFY(restored != nullptr);
        QCOMPARE(restored->typeId(), object->typeId());
        QCOMPARE(restored->name(), object->name());
        QVERIFY2(sameProjectValue(restored->projectData(), object->projectData()), qPrintable(object->typeId()));
    }
    qInfo().noquote() << QStringLiteral("Actual project: objects=%1 oldLoad=%2 ms save=%3 ms newLoad=%4 ms oldBytes=%5 newBytes=%6")
        .arg(original->objects().size()).arg(oldLoadMs).arg(saveMs).arg(loadMs)
        .arg(QFileInfo(input).size()).arg(QFileInfo(output).size());
    if (qEnvironmentVariableIsSet("IDOS_STORAGE_PROFILE_DISPLAY"))
    {
        IDOSMainWindow window;
        timer.restart();
        window.setProject(loaded.data());
        qInfo() << "Actual project display setup:" << timer.elapsed() << "ms";
        window.setProject(nullptr);
    }
}

void CommandRegistryTest::onProjectPresentationRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("presentation.idos"));
    IDOSProject project;
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("Visible well"));
    IDOSWellLogChannel channel(QStringLiteral("GR"));
    channel.setData(QVector<double>{0.0, 5.0, 10.0}, QVector<double>{1.0, 2.0, 3.0});
    IDOSWellLogSet logSet;
    logSet.addChannel(channel);
    well->setLogs(logSet);
    project.add(well);
    IDOSGrid* grid = new IDOSGrid();
    grid->setCornerPointData(1, 1, 1, QVector<double>(24, 0.0), QVector<double>{0, 0, 0, 0, 1, 1, 1, 1}, QVector<int>{1});
    project.add(grid);
    IDOSGridProperty* property = new IDOSGridProperty();
    property->setGridId(grid->objectId());
    property->setKeyword(QStringLiteral("PORO"));
    property->setDimensions(1, 1, 1);
    QVERIFY(property->setValues(QVector<double>{0.25}));
    project.add(property);
    const QString wellId = well->objectId();
    const QString gridId = grid->objectId();
    const QString propertyId = property->objectId();
    IDOSMainWindow window;
    window.setProject(&project);
    const QModelIndex wellRoot = window.dataTreeModel()->indexFromDataId(wellId);
    const QModelIndex logBranch = window.dataTreeModel()->index(0, 0, wellRoot);
    const QModelIndex logLeaf = window.dataTreeModel()->index(0, 0, logBranch);
    QVERIFY(logLeaf.isValid());
    QVERIFY(QMetaObject::invokeMethod(&window, "onDataTreeItemActivated", Qt::DirectConnection, Q_ARG(QModelIndex, logLeaf)));
    QList<IDOSWellLogTrackView*> logViews = window.findChildren<IDOSWellLogTrackView*>();
    QCOMPARE(logViews.size(), 1);
    QVariantMap expectedLog = logViews.first()->presentationState();
    expectedLog.insert(QStringLiteral("hasDepthRange"), true);
    expectedLog.insert(QStringLiteral("minimumDepth"), 2.0);
    expectedLog.insert(QStringLiteral("maximumDepth"), 8.0);
    logViews.first()->restorePresentationState(expectedLog);
    window.findChild<QAction*>(QStringLiteral("new2DWindowAction"))->trigger();
    window.findChild<QAction*>(QStringLiteral("new3DWindowAction"))->trigger();
    const QList<IDOSRenderView*> views = window.renderServer()->views();
    QCOMPARE(views.size(), 3);
    QVariantList expectedViews;
    QVariantList expectedWells;
    QVariantList expectedGrids;
    for (int index = 0; index < views.size(); ++index)
    {
        IDOSRenderView* view = views.at(index);
        view->setBackground(index == 0 ? QColor("#214365") : QColor("#654321"));
        view->setAxesVisible(index != 1);
        view->setLegendVisible(index != 2);
        view->setCursorTrackingEnabled(index == 1);
        QVariantMap state = view->presentationState();
        QVariantMap camera = state.value(QStringLiteral("camera")).toMap();
        camera.insert(QStringLiteral("position"), QVariantList{10.0 + index, 20.0, 30.0});
        camera.insert(QStringLiteral("focalPoint"), QVariantList{1.0, 2.0, 3.0});
        camera.insert(QStringLiteral("viewUp"), QVariantList{0.0, 1.0, 0.0});
        camera.insert(QStringLiteral("parallelScale"), 12.5 + index);
        state.insert(QStringLiteral("camera"), camera);
        QVERIFY(view->restorePresentationState(state));
        const QString wellRenderId = window.renderServer()->objectIds(view, wellId).first();
        IDOSWellRenderProvider* wellProvider = dynamic_cast<IDOSWellRenderProvider*>(view->scene()->object(wellRenderId)->provider());
        QVERIFY(wellProvider != nullptr);
        wellProvider->setVisible(index != 1);
        wellProvider->setOpacity(0.5 + index * 0.1);
        QVariantMap display = wellProvider->displaySettings();
        display.insert(QStringLiteral("trajectoryWidth"), 2.0 + index);
        QVERIFY(wellProvider->setDisplaySettings(display));
        const QString gridRenderId = window.renderServer()->objectIds(view, gridId).first();
        IDOSGridRenderProvider* gridProvider = dynamic_cast<IDOSGridRenderProvider*>(view->scene()->object(gridRenderId)->provider());
        QVERIFY(gridProvider != nullptr);
        gridProvider->setProperty(index == 1 ? nullptr : property);
        gridProvider->setVisible(index != 2);
        gridProvider->setOpacity(0.7);
        QVERIFY(gridProvider->setDisplayMode(IDOSDisplayMode::SurfaceWithEdges));
        QVERIFY(gridProvider->setEdgeColor(QColor("#123456")));
        QVERIFY(gridProvider->setEdgeWidth(2.5 + index));
        QVariantMap coloring = gridProvider->coloringSettings();
        coloring.insert(QStringLiteral("reversed"), index == 1);
        QVERIFY(gridProvider->setColoringSettings(coloring));
        expectedViews.append(view->presentationState());
        expectedWells.append(wellProvider->presentationState());
        expectedGrids.append(gridProvider->presentationState());
    }
    window.dataTreeView()->collapseAll();
    const QModelIndex wellIndex = window.dataTreeModel()->indexFromDataId(wellId);
    QVERIFY(wellIndex.isValid());
    window.dataTreeView()->setExpanded(wellIndex.parent(), true);
    window.dataTreeView()->setExpanded(wellIndex, true);
    window.dataTreeView()->selectionModel()->select(wellIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QCoreApplication::processEvents();
    const QVariantMap treeState = window.dataTreeView()->presentationState();
    const QString activeId = window.renderServer()->currentView()->property("viewId").toString();
    QVERIFY(window.saveProject(path));
    QCOMPARE(project.presentationState().value(QStringLiteral("logViews")).toList().size(), 1);
    QVERIFY(!project.isModified());
    QVERIFY(window.openProject(path, false));
    const QList<IDOSRenderView*> restored = window.renderServer()->views();
    QCOMPARE(restored.size(), 3);
    ads::CDockManager* dockManager = window.findChild<ads::CDockManager*>();
    QVERIFY(dockManager != nullptr);
    ads::CDockWidget* logDock = dockManager->findDockWidget(QStringLiteral("wellLogTrackDock_%1").arg(wellId));
    QVERIFY(logDock != nullptr);
    IDOSWellLogTrackView* restoredLog = qobject_cast<IDOSWellLogTrackView*>(logDock->widget());
    QVERIFY(restoredLog != nullptr);
    QVERIFY(sameProjectValue(restoredLog->presentationState(), expectedLog));
    QCOMPARE(window.renderServer()->currentView()->property("viewId").toString(), activeId);
    QVERIFY(!window.project()->isModified());
    QCoreApplication::processEvents();
    QVERIFY(!window.project()->isModified());
    QVERIFY(sameProjectValue(window.project()->presentationState().value(QStringLiteral("render")), window.renderServer()->presentationState()));
    QVERIFY(sameProjectValue(window.dataTreeView()->presentationState(), treeState));
    for (int index = 0; index < restored.size(); ++index)
    {
        IDOSRenderView* view = restored.at(index);
        QVERIFY(sameProjectValue(view->presentationState(), expectedViews.at(index)));
        const QString wellRenderId = window.renderServer()->objectIds(view, wellId).first();
        const QString gridRenderId = window.renderServer()->objectIds(view, gridId).first();
        QVERIFY(sameProjectValue(view->scene()->object(wellRenderId)->provider()->presentationState(), expectedWells.at(index)));
        QVERIFY(sameProjectValue(view->scene()->object(gridRenderId)->provider()->presentationState(), expectedGrids.at(index)));
    }
    const QVariantMap unlinked = restored.last()->presentationState();
    QVERIFY(restored.last()->linkCameraTo(restored.first()));
    QVERIFY(window.saveProject(path));
    QVERIFY(window.openProject(path, false));
    const QList<IDOSRenderView*> linkedViews = window.renderServer()->views();
    QVERIFY(linkedViews.last()->isCameraLinkedTo(linkedViews.first()));
    linkedViews.last()->unlinkCamera();
    QVERIFY(sameProjectValue(linkedViews.last()->presentationState(), unlinked));
    linkedViews.first()->scaleView(1.1);
    QVERIFY(window.project()->isModified());
    QVERIFY(window.saveProject(path));
    QVERIFY(!window.project()->isModified());
    IDOSProject empty;
    window.setProject(&empty);
    QCOMPARE(window.renderServer()->views().size(), 1);
    QVERIFY(dockManager->findDockWidget(QStringLiteral("wellLogTrackDock_%1").arg(wellId)) == nullptr);
    QCOMPARE(window.renderServer()->currentView()->property("viewId").toString(), QStringLiteral("main3d"));
    QVERIFY(!empty.isModified());
}

void CommandRegistryTest::onInvalidPresentationState()
{
    IDOSRenderView view;
    const QVariantMap original = view.presentationState();
    QVariantMap invalid = original;
    QVariantMap camera = invalid.value(QStringLiteral("camera")).toMap();
    camera.insert(QStringLiteral("parallelScale"), -1.0);
    invalid.insert(QStringLiteral("camera"), camera);
    QVERIFY(!view.restorePresentationState(invalid));
    QVERIFY(sameProjectValue(view.presentationState(), original));
    IDOSSeismicVolume volume;
    volume.setTraceInlineNumbers(QVector<int>{10, 20, 30});
    IDOSSeismicVolumeRenderProvider seismic(&volume);
    QVERIFY(seismic.setInlineNumber(10));
    const QVariantMap slice = seismic.presentationState();
    QVERIFY(seismic.setInlineNumber(30));
    QVERIFY(seismic.restorePresentationState(slice));
    QCOMPARE(seismic.inlineNumber(), 10);
    IDOSProject project;
    QVariantMap state;
    state.insert(QStringLiteral("version"), 1);
    QVariantMap render;
    render.insert(QStringLiteral("views"), QVariantList{QVariantMap{{QStringLiteral("id"), QStringLiteral("view_999999999")}}});
    state.insert(QStringLiteral("render"), render);
    project.setPresentationState(state);
    IDOSMainWindow window;
    window.setProject(&project);
    QCOMPARE(window.renderServer()->views().size(), 1);
}

void CommandRegistryTest::onWindowPreferences()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("ui.ini"));
    qputenv("IDOS_UI_SETTINGS_PATH", path.toUtf8());
    {
        IDOSMainWindow window;
        window.resize(1234, 788);
        window.move(50, 70);
        QVERIFY(window.close());
    }
    {
        QSettings settings(path, QSettings::IniFormat);
        QVERIFY(!settings.value(QStringLiteral("window/geometry")).toByteArray().isEmpty());
        QVERIFY(!settings.value(QStringLiteral("dockLayouts/main3d")).toByteArray().isEmpty());
        IDOSMainWindow window;
        QCOMPARE(window.size(), QSize(1234, 788));
    }
    qunsetenv("IDOS_UI_SETTINGS_PATH");
}
