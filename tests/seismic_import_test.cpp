#include <memory>

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "data/seismic/idosseismicvolume.h"
#include "idosapplication.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataprovidermetadata.h"
#include "providers/idosdataproviderregistry.h"
#include "providers/seismic/idosseismicvolumerenderprovider.h"

#include "seismic_import_test.h"

SeismicImportTest::SeismicImportTest()
{
    IDOSApplication::initProviders();
}

void SeismicImportTest::onSegyCustomHeaderAndSamples()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString filePath = directory.filePath(QStringLiteral("custom.sgy"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    const QByteArray bytes = createSegyFile();
    QVERIFY(!bytes.isEmpty());
    QCOMPARE(file.write(bytes), static_cast<qint64>(bytes.size()));
    file.close();

    const QList<IDOSDataProviderMetadata*> metadata =
        IDOSDataProviderRegistry::instance().providers(filePath);
    QCOMPARE(metadata.size(), 1);
    QCOMPARE(metadata.first()->id(), QStringLiteral("idos.seismic.segy"));
    std::unique_ptr<IDOSDataProvider> provider = metadata.first()->create();
    QList<IDOSDataObject*> objects = provider->read(filePath);
    QVERIFY2(objects.size() == 1, qPrintable(provider->lastError()));
    IDOSSeismicVolume* volume = qobject_cast<IDOSSeismicVolume*>(objects.first());
    QVERIFY(volume != nullptr);
    QCOMPARE(volume->traceCount(), static_cast<qint64>(2));
    QCOMPARE(volume->samplesPerTrace(), 2);
    QCOMPARE(volume->inlineMinimum(), 107);
    QCOMPARE(volume->inlineMaximum(), 108);
    QCOMPARE(volume->crosslineMinimum(), 400);
    QCOMPARE(volume->crosslineMaximum(), 401);
    const QVector<qint64> expectedOffsets = QVector<qint64>() << 0 << 2;
    const QVector<int> expectedCounts = QVector<int>() << 2 << 2;
    const QVector<qint64> offsets = volume->traceSampleOffsets();
    const QVector<int> counts = volume->traceSampleCounts();
    QCOMPARE(offsets.size(), expectedOffsets.size());
    QCOMPARE(counts.size(), expectedCounts.size());
    for (int index = 0; index < offsets.size(); ++index)
    {
        QCOMPARE(offsets.at(index), expectedOffsets.at(index));
    }
    for (int index = 0; index < counts.size(); ++index)
    {
        QCOMPARE(counts.at(index), expectedCounts.at(index));
    }
    QCOMPARE(volume->sampleValues().size(), 4);
    QVERIFY(qAbs(volume->sampleValues().at(0) - 1.0F) < 0.0001F);
    QVERIFY(qAbs(volume->sampleValues().at(1) + 2.0F) < 0.0001F);
    QVERIFY(qAbs(volume->sampleValues().at(2) - 3.0F) < 0.0001F);
    QVERIFY(qAbs(volume->sampleValues().at(3) - 0.5F) < 0.0001F);

    IDOSSeismicVolumeRenderProvider renderProvider(volume);
    QCOMPARE(renderProvider.inlineNumber(), 108);
    QCOMPARE(renderProvider.actors().size(), 1);
    QVERIFY(renderProvider.setInlineNumber(107));
    QCOMPARE(renderProvider.inlineNumber(), 107);
    QCOMPARE(renderProvider.actors().size(), 1);
    qDeleteAll(objects);
}

QByteArray SeismicImportTest::createSegyFile() const
{
    QByteArray textHeader(3200, '\x40');
    textHeader.replace(0, 3, QByteArray::fromHex("c3f0f1"));
    const QByteArray customHeader = QByteArray::fromHex("c3f1f640c9959389958540f1f740e79389958540f2f5");
    textHeader.replace(15 * 80, customHeader.size(), customHeader);

    QByteArray bytes = textHeader;
    QByteArray binaryHeader(400, '\0');
    writeUInt16(&binaryHeader, 16, 5000);
    writeUInt16(&binaryHeader, 20, 2);
    writeUInt16(&binaryHeader, 24, 1);
    writeUInt16(&binaryHeader, 304, 0);
    bytes.append(binaryHeader);

    QByteArray firstTrace(240, '\0');
    writeUInt16(&firstTrace, 114, 2);
    writeUInt16(&firstTrace, 116, 5000);
    writeUInt32(&firstTrace, 16, 107);
    writeUInt32(&firstTrace, 24, 400);
    writeUInt32(&firstTrace, 188, 1001);
    writeUInt32(&firstTrace, 192, 2001);
    bytes.append(firstTrace);
    QByteArray firstSamples;
    writeUInt32(&firstSamples, 0, 0x41100000U);
    writeUInt32(&firstSamples, 4, 0xc1200000U);
    bytes.append(firstSamples);

    QByteArray secondTrace(240, '\0');
    writeUInt16(&secondTrace, 114, 2);
    writeUInt16(&secondTrace, 116, 5000);
    writeUInt32(&secondTrace, 16, 108);
    writeUInt32(&secondTrace, 24, 401);
    writeUInt32(&secondTrace, 188, 1002);
    writeUInt32(&secondTrace, 192, 2002);
    bytes.append(secondTrace);
    QByteArray secondSamples;
    writeUInt32(&secondSamples, 0, 0x41300000U);
    writeUInt32(&secondSamples, 4, 0x40800000U);
    bytes.append(secondSamples);
    return bytes;
}

void SeismicImportTest::writeUInt16(QByteArray* bytes, int offset, quint16 value) const
{
    if (bytes == nullptr || offset < 0)
    {
        return;
    }
    if (bytes->size() < offset + 2)
    {
        bytes->resize(offset + 2);
    }
    bytes->data()[offset] = static_cast<char>((value >> 8) & 0xffU);
    bytes->data()[offset + 1] = static_cast<char>(value & 0xffU);
}

void SeismicImportTest::writeUInt32(QByteArray* bytes, int offset, quint32 value) const
{
    if (bytes == nullptr || offset < 0)
    {
        return;
    }
    if (bytes->size() < offset + 4)
    {
        bytes->resize(offset + 4);
    }
    bytes->data()[offset] = static_cast<char>((value >> 24) & 0xffU);
    bytes->data()[offset + 1] = static_cast<char>((value >> 16) & 0xffU);
    bytes->data()[offset + 2] = static_cast<char>((value >> 8) & 0xffU);
    bytes->data()[offset + 3] = static_cast<char>(value & 0xffU);
}

QTEST_MAIN(SeismicImportTest)
