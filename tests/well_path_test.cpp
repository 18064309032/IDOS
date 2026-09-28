#include "well_path_test.h"
#include "idosdataprovider.h"
#include "idosproviderregistry.h"
#include "idoswell.h"
#include "idoswellgeometryresolver.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
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
    QCOMPARE(well->path().spatialReference().verticalDatum(), IDOSWellSpatialReference::VerticalDatum::KellyBushing);
    QCOMPARE(well->path().spatialReference().depthDirection(), IDOSWellSpatialReference::DepthDirection::PositiveDown);
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
    QCOMPARE(well->path().spatialReference().verticalDatum(), IDOSWellSpatialReference::VerticalDatum::Unknown);
    qDeleteAll(objects);
}

void WellPathTest::onResolveGeometry()
{
    IDOSWellGeometryResolver resolver;
    IDOSWellHead head;
    IDOSWellPath path;
    QVector<IDOSWellPathPoint> points;
    points.append(IDOSWellPathPoint(1500.0, 100.0, 200.0, -1500.0, 1500.0));
    points.append(IDOSWellPathPoint(1600.0, 100.0, 200.0, -1600.0, 1600.0));
    path.setPoints(points);

    IDOSWellGeometryResolution resolution = resolver.resolve(head, path);
    QCOMPARE(resolution.startPointType(), IDOSWellGeometryResolution::StartPointType::TrajectoryStart);
    QCOMPARE(resolution.startPoint(), QVector3D(100.0F, 200.0F, -1500.0F));

    head.setSurfaceX(100.0);
    head.setSurfaceY(200.0);
    head.setSurfaceElevation(12.0);
    IDOSWellSpatialReference reference;
    reference.setVerticalDatum(IDOSWellSpatialReference::VerticalDatum::KellyBushing);
    reference.setDepthDirection(IDOSWellSpatialReference::DepthDirection::PositiveDown);
    head.setSpatialReference(reference);
    path.setSpatialReference(reference);

    resolution = resolver.resolve(head, path);
    QCOMPARE(resolution.startPointType(), IDOSWellGeometryResolution::StartPointType::TrajectoryStart);

    points.clear();
    points.append(IDOSWellPathPoint(0.0, 100.0, 200.0, 12.0, 0.0));
    points.append(IDOSWellPathPoint(100.0, 100.0, 200.0, -88.0, 100.0));
    path.setPoints(points);
    resolution = resolver.resolve(head, path);
    QCOMPARE(resolution.startPointType(), IDOSWellGeometryResolution::StartPointType::Wellhead);
    QCOMPARE(resolution.startPoint(), QVector3D(100.0F, 200.0F, 12.0F));

    IDOSWellSpatialReference incompatibleReference;
    incompatibleReference.setVerticalDatum(IDOSWellSpatialReference::VerticalDatum::MeanSeaLevel);
    incompatibleReference.setDepthDirection(IDOSWellSpatialReference::DepthDirection::PositiveDown);
    path.setSpatialReference(incompatibleReference);
    resolution = resolver.resolve(head, path);
    QCOMPARE(resolution.startPointType(), IDOSWellGeometryResolution::StartPointType::TrajectoryStart);

    path.clear();
    resolution = resolver.resolve(head, path);
    QCOMPARE(resolution.startPointType(), IDOSWellGeometryResolution::StartPointType::Wellhead);

    IDOSWellHead unknownHead;
    resolution = resolver.resolve(unknownHead, path);
    QCOMPARE(resolution.startPointType(), IDOSWellGeometryResolution::StartPointType::Unknown);
}

QTEST_MAIN(WellPathTest)
