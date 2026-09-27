#include "well_import_test.h"
#include "idosproviderregistry.h"
#include "idosdataprovider.h"
#include "idoswell.h"
#include "idoswellimportdialog.h"
#include <QFile>
#include <QPushButton>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QtTest>

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
            IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.well.header"));
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
        IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.well.header"));
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
        IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.well.header"));
    QList<IDOSDataObject*> objects = provider->read(file.fileName());
    QCOMPARE(objects.size(), 1);
    const IDOSWell* well = qobject_cast<IDOSWell*>(objects.first());
    QCOMPARE(well->wellHead().kb(), 125.0);
    QCOMPARE(well->wellHead().surfaceElevation(), 0.0);
    QCOMPARE(well->wellHead().symbol(), 7);
    QCOMPARE(well->wellHead().surfaceX(), 1.0);
    qDeleteAll(objects);
}
void WellImportTest::onPreviewDuplicatesAndCancel()
{
    IDOSWell first;
    first.setName(QStringLiteral("A10"));
    IDOSWell duplicate;
    duplicate.setName(QStringLiteral("a10"));
    IDOSWell next;
    next.setName(QStringLiteral("B1"));
    const QList<IDOSDataObject*> objects = {&first, &duplicate, &next};
    IDOSWellImportDialog dialog(QStringLiteral("sample"), objects, {QStringLiteral("b1")});
    QCOMPARE(dialog.selectedRows(), QList<int>{0});
    QTableWidget* table = dialog.findChild<QTableWidget*>(QStringLiteral("wellImportPreview"));
    QVERIFY(table != nullptr);
    QCOMPARE(table->rowCount(), 3);
    dialog.show();
    QTest::qWait(100);
    const QString screenshotRoot = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshotRoot.isEmpty())
    {
        QVERIFY(dialog.grab().save(screenshotRoot + QStringLiteral("/well-import.png")));
    }
    dialog.reject();
    QCOMPARE(dialog.result(), int(QDialog::Rejected));
    QVERIFY(first.parent() == nullptr);
    IDOSWellImportDialog allDuplicates(QStringLiteral("sample"), objects,
                                       {QStringLiteral("A10"), QStringLiteral("B1")});
    QVERIFY(allDuplicates.selectedRows().isEmpty());
    QVERIFY(!allDuplicates.findChild<QPushButton*>(QStringLiteral("confirmWellImport"))->isEnabled());
}
QTEST_MAIN(WellImportTest)
