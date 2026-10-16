#include <memory>

#include <QFile>
#include <QTemporaryDir>
#include <QtGlobal>
#include <QtTest>

#include <xlsxdocument.h>

#include "command/idosimportcommands.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/fracture/idosfractureschedule.h"
#include "data/well/idoswell.h"
#include "data/well/idoswelllogchannel.h"
#include "data/well/idoswelllogset.h"
#include "data/well/idoswellmarker.h"
#include "data/well/idoswellmarkerset.h"
#include "data/well/idoswellspatialreference.h"
#include "fracture/idosfracturescheduleprovider.h"
#include "idosapplication.h"
#include "idoscommandmanager.h"
#include "idoscommandregistry.h"
#include "idosproject.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataprovidermetadata.h"
#include "providers/idosdataproviderregistry.h"
#include "well/idoswellcompletionprovider.h"

#include "well_import_test.h"

bool WellImportTest::writeInput(const QString& path, const QByteArray& content)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(content) == content.size();
}

void WellImportTest::onExtendedInputProviders()
{
    QTemporaryDir directory;
    IDOSDataProviderRegistry& registry = IDOSDataProviderRegistry::instance();
    const QString topsPath = directory.filePath(QStringLiteral("tops.txt"));
    QVERIFY(writeInput(topsPath, "UNITS FT\nWELL HORIZON MD\n\"Well A\" \"Top Sand\" 1000\nWellB Top 20\n"));
    QCOMPARE(registry.metadataForFile(topsPath).first()->id(), QStringLiteral("idos.well.tops"));
    std::unique_ptr<IDOSDataProvider> tops = registry.createProvider(QStringLiteral("idos.well.tops"));
    QList<IDOSDataObject*> objects = tops->read(topsPath);
    QCOMPARE(objects.size(), 2);
    IDOSWell* first = qobject_cast<IDOSWell*>(objects.first());
    QVERIFY(first != nullptr);
    QCOMPARE(first->markers().markers().first().name(), QStringLiteral("Top Sand"));
    QCOMPARE(first->markers().markers().first().depth(), 304.8);
    qDeleteAll(objects);
    QVERIFY(writeInput(topsPath, "A Top 10\nB Top nan\n"));
    QVERIFY(tops->read(topsPath).isEmpty());
    QVERIFY(!tops->lastError().isEmpty());
    const QString evPath = directory.filePath(QStringLiteral("input.ev"));
    QVERIFY(writeInput(evPath, "UNITS FIELD\nWELLNAME \"Well A\"\n1/1/1998 perforation 1000 1100 0 0 0 0\n"));
    std::unique_ptr<IDOSDataProvider> completions = registry.createProvider(QStringLiteral("idos.well.completion"));
    objects = completions->read(evPath);
    QCOMPARE(objects.size(), 1);
    first = qobject_cast<IDOSWell*>(objects.first());
    QCOMPARE(first->completions().first().k(), 0);
    QCOMPARE(first->completions().first().top(), 304.8);
    QCOMPARE(first->completions().first().eventDate(), QStringLiteral("1/1/1998"));
    qDeleteAll(objects);
    QVERIFY(writeInput(evPath, "UNITS METRIC\nWELLNAME A\n1/1/1998 perforation 20 10\n"));
    QVERIFY(completions->read(evPath).isEmpty());
    const QString checkPath = directory.filePath(QStringLiteral("input.txt"));
    QVERIFY(writeInput(checkPath, "UNITS M MS\nTVD TWT Well\n0 0 A\n100 80 A\n0 0 B\n-100 -90 B\n"));
    QCOMPARE(registry.metadataForFile(checkPath).first()->id(), QStringLiteral("idos.well.checkshot"));
    std::unique_ptr<IDOSDataProvider> checkshots = registry.createProvider(QStringLiteral("idos.well.checkshot"));
    objects = checkshots->read(checkPath);
    QCOMPARE(objects.size(), 2);
    first = qobject_cast<IDOSWell*>(objects.first());
    QCOMPARE(first->checkshot().timeType(), QStringLiteral("TWT"));
    QCOMPARE(first->checkshot().timeUnit(), QStringLiteral("MS"));
    QCOMPARE(qobject_cast<IDOSWell*>(objects.last())->checkshot().depths().first(), -100.0);
    qDeleteAll(objects);
    QVERIFY(writeInput(checkPath, "TVD TWT Well\n0 0 A\n100 10 A\n100 20 A\n"));
    QVERIFY(checkshots->read(checkPath).isEmpty());
    QVERIFY(!checkshots->lastError().isEmpty());
}

void WellImportTest::onSharedWellTextFormats()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    IDOSDataProviderRegistry& registry = IDOSDataProviderRegistry::instance();

    const QString trajectoryPath = directory.filePath(QStringLiteral("T201.txt"));
    QVERIFY(writeInput(trajectoryPath, "MD\tINCL\tAZIM\n0\t0\t0\n100\t10\t90\n"));
    const QList<IDOSDataProviderMetadata*> trajectoryMetadata = registry.metadataForFile(trajectoryPath);
    QVERIFY(!trajectoryMetadata.isEmpty());
    QCOMPARE(trajectoryMetadata.first()->id(), QStringLiteral("idos.well.path"));
    std::unique_ptr<IDOSDataProvider> trajectoryProvider =
        registry.createProvider(QStringLiteral("idos.well.path"));
    QList<IDOSDataObject*> objects = trajectoryProvider->read(trajectoryPath);
    QCOMPARE(objects.size(), 1);
    IDOSWell* trajectoryWell = qobject_cast<IDOSWell*>(objects.first());
    QVERIFY(trajectoryWell != nullptr);
    QCOMPARE(trajectoryWell->path().pointCount(), 2);
    QVERIFY(trajectoryWell->path().points().last().x() > 0.0);
    QVERIFY(trajectoryWell->path().points().last().z() < 0.0);
    qDeleteAll(objects);

    const QString topsPath = directory.filePath(QStringLiteral("WELL TOP.txt"));
    QVERIFY(writeInput(topsPath, "WELL\tSURFACE\tMD\nT201\tTopA\t100\nT201\tTopB\t200\n"));
    const QList<IDOSDataProviderMetadata*> topsMetadata = registry.metadataForFile(topsPath);
    QVERIFY(!topsMetadata.isEmpty());
    QCOMPARE(topsMetadata.first()->id(), QStringLiteral("idos.well.tops"));
    std::unique_ptr<IDOSDataProvider> topsProvider = registry.createProvider(QStringLiteral("idos.well.tops"));
    objects = topsProvider->read(topsPath);
    QCOMPARE(objects.size(), 1);
    IDOSWell* topsWell = qobject_cast<IDOSWell*>(objects.first());
    QVERIFY(topsWell != nullptr);
    QCOMPARE(topsWell->markers().markerCount(), 2);
    qDeleteAll(objects);

    const QString headerPath = directory.filePath(QStringLiteral("well header.txt"));
    QVERIFY(writeInput(headerPath,
                       "well name\tX\tY\tMD\tKB\nT201\t10\t20\t300\t40\n"));
    const QList<IDOSDataProviderMetadata*> headerMetadata = registry.metadataForFile(headerPath);
    QVERIFY(!headerMetadata.isEmpty());
    QCOMPARE(headerMetadata.first()->id(), QStringLiteral("idos.well.header"));
    std::unique_ptr<IDOSDataProvider> headerProvider = registry.createProvider(QStringLiteral("idos.well.header"));
    objects = headerProvider->read(headerPath);
    QCOMPARE(objects.size(), 1);
    IDOSWell* headerWell = qobject_cast<IDOSWell*>(objects.first());
    QVERIFY(headerWell != nullptr);
    QCOMPARE(headerWell->wellHead().surfaceX(), 10.0);
    QCOMPARE(headerWell->wellHead().bottomDepth(), 300.0);
    qDeleteAll(objects);
}

void WellImportTest::onExtendedInputUndoRedo()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registry.add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportWellDataCommandMetadata()));
    IDOSCommandManager* manager = project.commandManager();
    manager->setRegistry(&registry);
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A"));
    well->setCompletions({IDOSWellCompletion(5, true, 200.0, 100.0)});
    project.add(well);
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("input.perf"));
    QVERIFY(writeInput(path, "WELL TOP BOTTOM\nA 10 20\nB 30 40\n"));
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), path);
    QVERIFY(manager->execute(QStringLiteral("wellData.import"), arguments).value(QStringLiteral("success")).toBool());
    QCOMPARE(well->completions().size(), 2);
    QCOMPARE(project.objects().size(), 2);
    manager->undo();
    QCOMPARE(well->completions().size(), 1);
    QCOMPARE(well->completions().first().k(), 5);
    QCOMPARE(project.objects().size(), 1);
    QVERIFY(QFile::remove(path));
    manager->redo();
    QCOMPARE(well->completions().size(), 2);
    const QString checkPath = directory.filePath(QStringLiteral("input.cs"));
    QVERIFY(writeInput(checkPath, "TVD TWT Well\n0 0 A\n100 80 A\n"));
    arguments.insert(QStringLiteral("filePath"), checkPath);
    QVERIFY(manager->execute(QStringLiteral("wellData.import"), arguments).value(QStringLiteral("success")).toBool());
    QVERIFY(well->hasCheckshot());
    manager->undo();
    QVERIFY(!well->hasCheckshot());
    manager->redo();
    QCOMPARE(well->checkshot().times().last(), 80.0);
    const QString topsPath = directory.filePath(QStringLiteral("input.tops"));
    QVERIFY(writeInput(topsPath, "A Top 10\n"));
    arguments.insert(QStringLiteral("filePath"), topsPath);
    QVERIFY(manager->execute(QStringLiteral("wellData.import"), arguments).value(QStringLiteral("success")).toBool());
    QCOMPARE(well->markers().markerCount(), 1);
    manager->undo();
    QVERIFY(!well->hasMarkers());
    manager->redo();
    QCOMPARE(well->markers().markers().first().depth(), 10.0);
}

void WellImportTest::onLegacyExtendedSamples()
{
    IDOSDataProviderRegistry& registry = IDOSDataProviderRegistry::instance();
    const QStringList paths = {QFINDTESTDATA("data/WellTops.prn"), QFINDTESTDATA("data/PetrelTops.tops"),
                               QFINDTESTDATA("data/Perforations.ev"), QFINDTESTDATA("data/Checkshots.cs")};
    for (const QString& path : paths)
    {
        const QList<IDOSDataProviderMetadata*> candidates = registry.metadataForFile(path);
        QVERIFY(!candidates.isEmpty());
        std::unique_ptr<IDOSDataProvider> provider = candidates.first()->createProvider();
        QList<IDOSDataObject*> objects = provider->read(path);
        QVERIFY2(!objects.isEmpty(), qPrintable(provider->lastError()));
        if (candidates.first()->id() == QStringLiteral("idos.well.tops"))
        {
            QCOMPARE(objects.size(), 14);
            int markerCount = 0;
            for (IDOSDataObject* object : objects)
            {
                markerCount += qobject_cast<IDOSWell*>(object)->markers().markerCount();
            }
            QCOMPARE(markerCount, 98);
        }
        else if (candidates.first()->id() == QStringLiteral("idos.well.completion"))
        {
            QCOMPARE(objects.size(), 5);
        }
        else
        {
            QCOMPARE(objects.size(), 15);
        }
        for (IDOSDataObject* object : objects)
        {
            QVERIFY(qobject_cast<IDOSWell*>(object) != nullptr);
        }
        qDeleteAll(objects);
    }
}

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
            IDOSDataProviderRegistry::instance().createProvider(QStringLiteral("idos.well.header"));
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

void WellImportTest::onPetrelWellHeader()
{
    const QString sample = QFINDTESTDATA("data/PetrelWellheader.txt");
    QVERIFY(!sample.isEmpty());
    IDOSDataProviderRegistry& registry = IDOSDataProviderRegistry::instance();
    QCOMPARE(registry.metadataForFile(sample).first()->id(), QStringLiteral("idos.well.header"));
    std::unique_ptr<IDOSDataProvider> provider = registry.createProvider(QStringLiteral("idos.well.header"));
    QVERIFY(provider != nullptr);
    QList<IDOSDataObject*> objects = provider->read(sample);
    QCOMPARE(provider->lastError(), QString());
    QCOMPARE(objects.size(), 2);

    const IDOSWell* verticalWell = qobject_cast<IDOSWell*>(objects.at(0));
    QVERIFY(verticalWell != nullptr);
    QCOMPARE(verticalWell->name(), QStringLiteral("Zi224Vertical"));
    QCOMPARE(verticalWell->wellHead().surfaceX(), 18488365.77);
    QCOMPARE(verticalWell->wellHead().surfaceY(), 3312053.56);
    QCOMPARE(verticalWell->wellHead().surfaceElevation(), 381.1);
    QCOMPARE(verticalWell->wellHead().kb(), 381.1);
    QCOMPARE(verticalWell->wellHead().bottomDepth(), 4516.0);
    QCOMPARE(verticalWell->wellHead().spatialReference().verticalDatum(),
             IDOSWellSpatialReference::VerticalDatum::KellyBushing);
    QCOMPARE(verticalWell->wellHead().spatialReference().depthDirection(),
             IDOSWellSpatialReference::DepthDirection::PositiveDown);

    const IDOSWell* horizontalWell = qobject_cast<IDOSWell*>(objects.at(1));
    QVERIFY(horizontalWell != nullptr);
    QCOMPARE(horizontalWell->name(), QStringLiteral("Zi224H"));
    QCOMPARE(horizontalWell->wellHead().bottomDepth(), 6030.0);
    qDeleteAll(objects);
}

void WellImportTest::onSpreadsheetImports()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString schedulePath = directory.filePath(QStringLiteral("schedule.xlsx"));
    QXlsx::Document schedule;
    QVERIFY(schedule.write(1, 1, QStringLiteral("Pump rate (m3/min)")));
    QVERIFY(schedule.write(1, 2, QStringLiteral("Fluid name")));
    QVERIFY(schedule.write(1, 3, QStringLiteral("Fluid volume (m3)")));
    QVERIFY(schedule.write(2, 1, 3.5));
    QVERIFY(schedule.write(2, 2, QStringLiteral("Water")));
    QVERIFY(schedule.write(2, 3, 12.0));
    QVERIFY(schedule.saveAs(schedulePath));
    IDOSFractureScheduleProvider scheduleProvider;
    QVERIFY(scheduleProvider.canRead(schedulePath));
    QList<IDOSDataObject*> objects = scheduleProvider.read(schedulePath);
    QVERIFY2(objects.size() == 1, qPrintable(scheduleProvider.lastError()));
    const IDOSFractureSchedule* data = qobject_cast<IDOSFractureSchedule*>(objects.first());
    QVERIFY(data != nullptr);
    QCOMPARE(data->samples().size(), 1);
    QCOMPARE(data->samples().first().toMap().value(QStringLiteral("Fluid volume (m3)")).toDouble(), 12.0);
    qDeleteAll(objects);

    const QString completionPath = directory.filePath(QStringLiteral("Perforation_TestWell.xlsx"));
    QXlsx::Document completion;
    QVERIFY(completion.write(1, 1, QStringLiteral("Perf top MD (m)")));
    QVERIFY(completion.write(1, 2, QStringLiteral("Perf bottom MD (m)")));
    QVERIFY(completion.write(1, 3, QStringLiteral("Stage number")));
    QVERIFY(completion.write(2, 1, 100.0));
    QVERIFY(completion.write(2, 2, 110.0));
    QVERIFY(completion.write(2, 3, 2));
    QVERIFY(completion.saveAs(completionPath));
    IDOSWellCompletionProvider completionProvider;
    QVERIFY(completionProvider.canRead(completionPath));
    objects = completionProvider.read(completionPath);
    QVERIFY2(objects.size() == 1, qPrintable(completionProvider.lastError()));
    const IDOSWell* well = qobject_cast<IDOSWell*>(objects.first());
    QVERIFY(well != nullptr);
    QCOMPARE(well->name(), QStringLiteral("TestWell"));
    QCOMPARE(well->completions().size(), 1);
    QCOMPARE(well->completions().first().top(), 100.0);
    QCOMPARE(well->completions().first().bottom(), 110.0);
    QCOMPARE(well->completions().first().attributes().value(QStringLiteral("Stage number")).toInt(), 2);
    qDeleteAll(objects);
    QVERIFY(completion.write(2, 2, 90.0));
    QVERIFY(completion.saveAs(completionPath));
    objects = completionProvider.read(completionPath);
    QVERIFY(objects.isEmpty());
    QVERIFY(!completionProvider.lastError().isEmpty());
}

void WellImportTest::onInvalidRows()
{
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("input"));
    const QByteArray header("WellName X-Coord Y-Coord Top_Depth Bottom Depth KB Symbol\n");
    const QList<QByteArray> badRows = {"B 1 2 3 4 5\n", "B nan 2 3 4 5 6\n", "B 1 2 5 4 5 6\n", "B 1 2 3 4 5 1.5\n"};
    std::unique_ptr<IDOSDataProvider> provider =
        IDOSDataProviderRegistry::instance().createProvider(QStringLiteral("idos.well.header"));
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
        IDOSDataProviderRegistry::instance().createProvider(QStringLiteral("idos.well.header"));
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
        IDOSDataProviderRegistry::instance().createProvider(QStringLiteral("idos.well.las"));
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
        IDOSDataProviderRegistry::instance().createProvider(QStringLiteral("idos.well.las"));
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
