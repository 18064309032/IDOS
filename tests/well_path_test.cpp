#include "well_path_test.h"
#include "idosdataobjecthandling.h"
#include "idosdataprovider.h"
#include "idosproviderregistry.h"
#include "idosproject.h"
#include "idoswell.h"
#include "idoswellpathimportdialog.h"
#include <QDir>
#include <QFile>
#include <QPushButton>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QTimer>
#include <QSignalSpy>
#include <QtTest>

void WellPathTest::onParseSample()
{
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.well.path"));
    QVERIFY(reader != nullptr);
    QList<IDOSDataObject*> objects = reader->read(QFINDTESTDATA("data/A10.dev"));
    QCOMPARE(reader->lastError(), QString());
    QCOMPARE(objects.size(), 1);
    IDOSWell* well = qobject_cast<IDOSWell*>(objects.first());
    QCOMPARE(well->name(), QStringLiteral("A10"));
    QVERIFY(!well->hasWellHead());
    QCOMPARE(well->path().pointCount(), 3);
    const IDOSWellPathPoint point = well->path().points().first();
    QCOMPARE(point.md(), 1499.878992);
    QCOMPARE(point.x(), 456979.063700);
    QCOMPARE(point.y(), 6782712.412000);
    QCOMPARE(point.z(), -1499.878992);
    QCOMPARE(point.tvd(), 1499.878992);
    QCOMPARE(point.azimuth(), 99.853422);
    QVERIFY(well->path().sourceComments().join(QLatin1Char('\n')).contains(QStringLiteral("MD IS NOT EXACT")));
    qDeleteAll(objects);
    const QString directory = qEnvironmentVariable("IDOS_TEST_WELL_PATH_DIR");
    if (!directory.isEmpty())
    {
        const QStringList files = QDir(directory).entryList({QStringLiteral("*.dev")}, QDir::Files, QDir::Name);
        QCOMPARE(files.size(), 15);
        const QList<int> counts = {6011, 1805, 2297, 8268, 7612, 8268, 3839, 8268, 466, 546, 582, 756, 534, 597, 802};
        for (int i = 0; i < files.size(); ++i)
        {
            objects = reader->read(QDir(directory).filePath(files[i]));
            QCOMPARE(reader->lastError(), QString());
            QCOMPARE(objects.size(), 1);
            well = qobject_cast<IDOSWell*>(objects.first());
            QCOMPARE(well->path().pointCount(), counts[i]);
            qDeleteAll(objects);
        }
    }
}

void WellPathTest::onInvalidInput()
{
    QTemporaryDir directory;
    const QString filePath = directory.filePath(QStringLiteral("wrong-name.dev"));
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.well.path"));
    const QByteArray header("# WELL NAME: A10\nMD X Y Z TVD DX DY AZIM INCL\n");
    const QList<QByteArray> contents = {
        header + "10 1 2 -3 3 0 0 -90 10\n10 2 3 -4 4 1 1 -90 10\n",
        header + "10 1 2 -3 3 0 0 -90 10\n11 nan 3 -4 4 1 1 -90 10\n",
        header + "10 1 2 -3 3 0 0 -90\n",
        QByteArray("MD X Y Z TVD DX DY AZIM INCL\n10 1 2 -3 3 0 0 0 0\n11 1 2 -4 4 0 0 0 0\n"),
        header + "10 1 2 -3 3 0 0 0 0\n",
        QByteArray("# WELL NAME: A10\nMD X Y Z TVD DX DY AZIM AZIM\n")};
    for (const QByteArray& content : contents)
    {
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(content);
        file.close();
        const QList<IDOSDataObject*> objects = reader->read(filePath);
        QVERIFY(objects.isEmpty());
        QVERIFY(!reader->lastError().isEmpty());
    }
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("# WELL NAME: A10\nINCL AZIM DY DX TVD Z Y X MD\n10 -90 0 0 3 -3 2 1 10\n10 -90 1 1 4 -4 3 2 11\n");
    file.close();
    QList<IDOSDataObject*> objects = reader->read(filePath);
    QCOMPARE(objects.size(), 1);
    QVERIFY(reader->lastError().isEmpty());
    const IDOSWell* well = qobject_cast<IDOSWell*>(objects.first());
    QCOMPARE(well->name(), QStringLiteral("A10"));
    QCOMPARE(well->path().points().first().azimuth(), -90.0);
    QCOMPARE(well->path().points().last().md(), 11.0);
    qDeleteAll(objects);
}

void WellPathTest::onBatchImport()
{
    IDOSProject project;
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("a10"));
    IDOSWellHead head;
    head.setKb(125.0);
    well->setWellHead(head);
    project.addObject(well);
    const QString id = well->objectId();
    const QString sample = QFINDTESTDATA("data/A10.dev");
    QTemporaryDir directory;
    const QString duplicate = directory.filePath(QStringLiteral("different-name.dev"));
    QVERIFY(QFile::copy(sample, duplicate));
    const QStringList files = {sample, duplicate, directory.filePath(QStringLiteral("missing.dev"))};
    QSignalSpy changed(&project, &IDOSProject::objectChanged);
    QTimer timer;
    connect(&timer, &QTimer::timeout, this, &WellPathTest::onPreview);
    timer.start(20);
    m_cancel = true;
    QCOMPARE(IDOSDataObjectHandling::importWellPaths(&project, files, nullptr), 0);
    QVERIFY(!well->hasPath());
    QCOMPARE(changed.count(), 0);
    m_cancel = false;
    QCOMPARE(IDOSDataObjectHandling::importWellPaths(&project, files, nullptr), 1);
    QCOMPARE(project.objects().size(), 1);
    QCOMPARE(project.objectById(id), well);
    QCOMPARE(well->wellHead().kb(), 125.0);
    QCOMPARE(well->path().pointCount(), 3);
    QCOMPARE(changed.count(), 1);
    QCOMPARE(IDOSDataObjectHandling::importWellPaths(&project, files, nullptr), 0);
    QCOMPARE(changed.count(), 1);
    IDOSProject empty;
    QCOMPARE(IDOSDataObjectHandling::importWellPaths(&empty, files, nullptr), 0);
    QVERIFY(empty.objects().isEmpty());
    IDOSWell* ambiguous = new IDOSWell();
    ambiguous->setName(QStringLiteral("A10"));
    project.addObject(ambiguous);
    QCOMPARE(IDOSDataObjectHandling::importWellPaths(&project, files, nullptr), 0);
    QVERIFY(!ambiguous->hasPath());
    timer.stop();
}

void WellPathTest::onPreview()
{
    QMessageBox* message = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
    if (message != nullptr)
    {
        message->accept();
        return;
    }
    IDOSWellPathImportDialog* dialog = qobject_cast<IDOSWellPathImportDialog*>(QApplication::activeModalWidget());
    if (dialog == nullptr)
    {
        return;
    }
    const QString screenshotRoot = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshotRoot.isEmpty() && m_cancel)
    {
        dialog->grab().save(screenshotRoot + QStringLiteral("/well-path-import.png"));
    }
    QPushButton* confirm = dialog->findChild<QPushButton*>(QStringLiteral("confirmPathImport"));
    if (m_cancel || !confirm->isEnabled())
    {
        dialog->reject();
    }
    else
    {
        confirm->click();
    }
}
QTEST_MAIN(WellPathTest)
