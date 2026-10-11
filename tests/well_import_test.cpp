#include <memory>

#include <QFile>
#include <QTemporaryDir>
#include <QtGlobal>
#include <QtTest>

#include "idosapplication.h"
#include "idoscommandmanager.h"
#include "idoscommandregistry.h"
#include "idosdataprovider.h"
#include "idosdataproviderregistry.h"
#include "idosimportcommands.h"
#include "idosproject.h"
#include "idossimulationcaseobject.h"
#include "idoswell.h"
#include "idoswelllogchannel.h"
#include "idoswelllogset.h"

#include "well_import_test.h"

void WellImportTest::onCommandMergeUndoRedo()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registry.add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportWellDataCommandMetadata()));
    IDOSCommandManager* manager = project.commandManager();
    manager->setRegistry(&registry);
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral(" a10 "));
    project.add(well);
    const QString id = well->objectId();
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    IDOSCaseItemRef ref;
    ref.setObjectId(id);
    ref.setRole(QStringLiteral("case.well"));
    caseObject->setItemRefs(QList<IDOSCaseItemRef>{ref});
    project.add(caseObject);
    QTemporaryDir directory;
    const QString filePath = directory.filePath(QStringLiteral("header.txt"));
    QVERIFY(QFile::copy(QFINDTESTDATA("data/Wellheader.txt"), filePath));
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), filePath);
    QVERIFY(manager->execute(QStringLiteral("wellData.import"), arguments).value(QStringLiteral("success")).toBool());
    QCOMPARE(project.object(id), static_cast<IDOSDataObject*>(well));
    QVERIFY(well->hasWellHead());
    QCOMPARE(manager->result().value(QStringLiteral("createdCount")).toInt(), 14);
    QCOMPARE(manager->result().value(QStringLiteral("mergedCount")).toInt(), 1);
    QCOMPARE(caseObject->itemRefs().first().objectId(), id);
    manager->undo();
    QVERIFY(!well->hasWellHead());
    QCOMPARE(project.objects().size(), 2);
    QVERIFY(QFile::remove(filePath));
    manager->redo();
    QVERIFY(well->hasWellHead());
    QCOMPARE(project.objects().size(), 16);
    QVERIFY(!manager->canRedo());
    manager->undo();
    manager->clear();
    QCOMPARE(project.object(id), static_cast<IDOSDataObject*>(well));
}

void WellImportTest::onTargetedCommandAndFailure()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registry.add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportWellDataCommandMetadata()));
    IDOSCommandManager* manager = project.commandManager();
    manager->setRegistry(&registry);
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("Target"));
    project.add(well);
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), QFINDTESTDATA("data/A10.dev"));
    arguments.insert(QStringLiteral("targetWellId"), well->objectId());
    QVERIFY(manager->execute(QStringLiteral("wellData.import"), arguments).value(QStringLiteral("success")).toBool());
    QVERIFY(well->hasPath());
    QCOMPARE(well->name(), QStringLiteral("Target"));
    QCOMPARE(project.objects().size(), 1);
    manager->undo();
    QVERIFY(!well->hasPath());
    QVERIFY(well->path().isEmpty());
    arguments.insert(QStringLiteral("filePath"), QFINDTESTDATA("data/Wellheader.txt"));
    QVERIFY(!manager->execute(QStringLiteral("wellData.import"), arguments).value(QStringLiteral("success")).toBool());
    QVERIFY(manager->canRedo());
    QVERIFY(!well->hasWellHead());
    QCOMPARE(project.objects().size(), 1);
    manager->redo();
    QVERIFY(well->hasPath());
}

void WellImportTest::onCommandRestoresLogChannels()
{
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("input.las"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("~Version Information\nVERS. 2.0 : LAS version\n"
               "~Well Information\nWELL. A : Well name\nNULL. -999.25 : Null\n"
               "~Curve Information\nDEPT.M : Depth\nGR.API : Gamma\nRHOB.G/C3 : Density\n"
               "~ASCII\n100 20 2.4\n101 21 2.5\n");
    file.close();
    IDOSProject project;
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A"));
    IDOSWellLogChannel channel(QStringLiteral("GR"));
    channel.setData(QVector<double>{100}, QVector<double>{7});
    IDOSWellLogSet logs;
    logs.addChannel(channel);
    well->setLogs(logs);
    project.add(well);
    IDOSCommandManager* manager = project.commandManager();
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), path);
    QVERIFY(manager->execute(new IDOSImportObjectsCommand(
        &project, QStringLiteral("wellData.import"), arguments)));
    QCOMPARE(well->logs().channels().size(), 2);
    manager->undo();
    QCOMPARE(well->logs().channels().size(), 1);
    QCOMPARE(well->logs().channels().first().values().first(), 7.0);
    QVERIFY(QFile::remove(path));
    manager->redo();
    QCOMPARE(well->logs().channels().size(), 2);
    QCOMPARE(well->logs().channels().first().values().first(), 20.0);
}

void WellImportTest::onSampleFiles()
{
    const QString sample = QFINDTESTDATA("data/Wellheader.txt");
    QVERIFY(!sample.isEmpty());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString noExtension = directory.filePath(QStringLiteral("Wellheader"));
    QVERIFY(QFile::copy(sample, noExtension));
    const QStringList paths = {sample, noExtension};
    for (const QString& path : paths)
    {
        std::unique_ptr<IDOSDataProvider> provider =
            IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.well.header"));
        QVERIFY(provider != nullptr);
        QList<IDOSDataObject*> objects = provider->read(path);
        QCOMPARE(provider->lastError(), QString());
        QCOMPARE(objects.size(), 15);
        const IDOSWell* well = qobject_cast<IDOSWell*>(objects.first());
        QVERIFY(well != nullptr);
        QCOMPARE(well->name(), QStringLiteral("A10"));
        QCOMPARE(well->wellHead().surfaceX(), 456979.063700);
        QCOMPARE(well->wellHead().surfaceY(), 6782712.412000);
        QCOMPARE(well->wellHead().bottomDepth(), 2415.802791);
        QCOMPARE(well->wellHead().symbol(), 4);
        qDeleteAll(objects);
    }
}

void WellImportTest::onInvalidRows()
{
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("input"));
    const QByteArray header("WellName X-Coord Y-Coord Top_Depth Bottom Depth KB Symbol\n");
    const QList<QByteArray> badRows = {"B 1 2 3 4 5\n", "B nan 2 3 4 5 6\n", "B 1 2 5 4 5 6\n", "B 1 2 3 4 5 1.5\n"};
    std::unique_ptr<IDOSDataProvider> provider =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.well.header"));
    for (const QByteArray& row : badRows)
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(header + "A 1 2 3 4 5 6\n" + row);
        file.close();
        QList<IDOSDataObject*> objects = provider->read(path);
        QVERIFY(objects.isEmpty());
        QVERIFY(provider->lastError().contains(QStringLiteral("3")));
    }
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(header + "A 1 2 3 4 5 6\n");
    file.close();
    QList<IDOSDataObject*> objects = provider->read(path);
    QCOMPARE(objects.size(), 1);
    QVERIFY(provider->lastError().isEmpty());
    qDeleteAll(objects);
}

void WellImportTest::onReorderedColumns()
{
    QTemporaryDir directory;
    QFile file(directory.filePath(QStringLiteral("input")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("Symbol KB Bottom_Depth Top_Depth Y-Coord X-Coord WellName\n7 125 400 300 2 1 A\n");
    file.close();
    std::unique_ptr<IDOSDataProvider> provider =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.well.header"));
    QList<IDOSDataObject*> objects = provider->read(file.fileName());
    QCOMPARE(objects.size(), 1);
    const IDOSWell* well = qobject_cast<IDOSWell*>(objects.first());
    QCOMPARE(well->wellHead().kb(), 125.0);
    QCOMPARE(well->wellHead().surfaceElevation(), 0.0);
    QCOMPARE(well->wellHead().symbol(), 7);
    QCOMPARE(well->wellHead().surfaceX(), 1.0);
    qDeleteAll(objects);
}

void WellImportTest::onLasImport()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QFile file(directory.filePath(QStringLiteral("Well-A.las")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("~Version Information\n"
               "VERS. 2.0 : LAS version\n"
               "~Well Information\n"
               "WELL. Well-A : Well name\n"
               "XCOORD.M : 456789.5\n"
               "YCOORD.M : 6789123.5\n"
               "ELEV.M : 18.5\n"
               "KB.M : 24.0\n"
               "STRT.M 1000.0 : Start depth\n"
               "STOP.M 1001.0 : Stop depth\n"
               "DREF.M 12.0 : Reference depth\n"
               "NULL. -999.25 : Null sample\n"
               "~Curve Information\n"
               "DEPT.M : Measured depth index\n"
               "GR.API : Gamma ray\n"
               "RHOB.G/C3 : Bulk density\n"
               "~ASCII Log Data\n"
               "1000.0 80.5 2.35\n"
               "1000.5 -999.25 2.36\n"
               "1001.0 82.0 2.37\n");
    file.close();

    std::unique_ptr<IDOSDataProvider> provider =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.well.las"));
    QVERIFY(provider != nullptr);
    QList<IDOSDataObject*> objects = provider->read(file.fileName());
    QCOMPARE(provider->lastError(), QString());
    QCOMPARE(objects.size(), 1);

    const IDOSWell* well = qobject_cast<IDOSWell*>(objects.first());
    QVERIFY(well != nullptr);
    QCOMPARE(well->name(), QStringLiteral("Well-A"));
    QVERIFY(well->hasWellHead());
    QCOMPARE(well->wellHead().surfaceX(), 456789.5);
    QCOMPARE(well->wellHead().surfaceY(), 6789123.5);
    QCOMPARE(well->wellHead().surfaceElevation(), 18.5);
    QCOMPARE(well->wellHead().kb(), 24.0);
    QCOMPARE(well->wellHead().topDepth(), 1000.0);
    QCOMPARE(well->wellHead().bottomDepth(), 1001.0);
    QCOMPARE(well->referenceDepth(), 12.0);
    QVERIFY(well->hasLogs());
    QCOMPARE(well->logs().channelCount(), 2);

    const IDOSWellLogChannel* gammaRay = well->logs().channel(QStringLiteral("GR"));
    QVERIFY(gammaRay != nullptr);
    QCOMPARE(gammaRay->unit(), QStringLiteral("API"));
    QCOMPARE(gammaRay->sampleCount(), 3);
    QCOMPARE(gammaRay->depths().at(2), 1001.0);
    QCOMPARE(gammaRay->values().at(0), 80.5);
    QVERIFY(qIsNaN(gammaRay->values().at(1)));

    const IDOSWellLogChannel* density = well->logs().channel(QStringLiteral("RHOB"));
    QVERIFY(density != nullptr);
    QCOMPARE(density->unit(), QStringLiteral("G/C3"));
    QCOMPARE(density->values().at(2), 2.37);
    qDeleteAll(objects);
}

void WellImportTest::onInvalidLasFiles()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QFile file(directory.filePath(QStringLiteral("invalid.las")));
    std::unique_ptr<IDOSDataProvider> provider =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.well.las"));
    QVERIFY(provider != nullptr);

    const QList<QByteArray> inputs = {
        "~Version\n~Well\nWELL. : A\n~Curve\nDEPT.M\nGR.API\n",
        "~Version\n~Well\nWELL. : A\n~Curve\nDEPT.M\nGR.API\n~A\n1000 80 12\n",
        "~Version\n~Well\nWELL. : A\n~Curve\nDEPT.M\nGR.API\n~A\n1000 80\n1000 81\n"};
    for (const QByteArray& input : inputs)
    {
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write(input);
        file.close();
        QList<IDOSDataObject*> objects = provider->read(file.fileName());
        QVERIFY(objects.isEmpty());
        QVERIFY(!provider->lastError().isEmpty());
    }
}

QTEST_MAIN(WellImportTest)

WellImportTest::WellImportTest()
{
    IDOSApplication::initProviders();
}
