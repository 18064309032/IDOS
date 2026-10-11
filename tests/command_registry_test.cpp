#include <memory>

#include <QJsonArray>
#include <QJsonObject>
#include <QtTest>

#include "command/idoscommandmanager.h"
#include "command/idoscommandmetadata.h"
#include "command/idoscommandregistry.h"
#include "command/idosimportcommands.h"
#include "idosapplication.h"
#include "idoscaseitemref.h"
#include "idoscreatecasecommand.h"
#include "idoscreatewellcommand.h"
#include "idosdataobject.h"
#include "idosdataregistry.h"
#include "idosdeletecasecommand.h"
#include "idosdeletegridcommand.h"
#include "idosdeletepropertycommand.h"
#include "idosdeletewellcommand.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idosrenameobjectcommand.h"
#include "idossimulationcaseobject.h"
#include "idoswell.h"
#include "idoswellmetadata.h"

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
    QCOMPARE(tools.size(), 11);

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

CommandRegistryTest::CommandRegistryTest()
{
    IDOSApplication::initProviders();
}
