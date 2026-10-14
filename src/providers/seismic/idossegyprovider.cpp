#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QTextCodec>
#include <QtGlobal>

#include "idosseismicvolume.h"

#include "idossegyprovider.h"

bool IDOSSegyProvider::canRead(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly) || file.size() < 3600)
    {
        return false;
    }
    if (!file.seek(3200))
    {
        return false;
    }
    const QByteArray header = file.read(400);
    if (header.size() != 400)
    {
        return false;
    }
    const quint16 formatBigEndian = readUInt16(header, 24, false);
    const quint16 formatLittleEndian = readUInt16(header, 24, true);
    const quint16 samplesBigEndian = readUInt16(header, 20, false);
    const quint16 samplesLittleEndian = readUInt16(header, 20, true);
    const bool bigEndianValid = bytesPerSample(formatBigEndian) > 0 && samplesBigEndian > 0;
    const bool littleEndianValid = bytesPerSample(formatLittleEndian) > 0 && samplesLittleEndian > 0;
    return bigEndianValid || littleEndianValid;
}

QList<IDOSDataObject*> IDOSSegyProvider::read(const QString& filePath)
{
    setLastError(QString());
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        setLastError(QObject::tr("Cannot open SEG-Y file: %1").arg(file.errorString()));
        return {};
    }
    if (file.size() < 3600 || !file.seek(0))
    {
        setLastError(QObject::tr("The SEG-Y file is shorter than its mandatory headers."));
        return {};
    }
    const QByteArray textHeader = file.read(3200);
    const QByteArray binaryHeader = file.read(400);
    if (textHeader.size() != 3200 || binaryHeader.size() != 400)
    {
        setLastError(QObject::tr("The SEG-Y file has incomplete textual or binary headers."));
        return {};
    }

    const quint16 bigEndianFormat = readUInt16(binaryHeader, 24, false);
    const quint16 littleEndianFormat = readUInt16(binaryHeader, 24, true);
    const quint16 bigEndianSamples = readUInt16(binaryHeader, 20, false);
    const quint16 littleEndianSamples = readUInt16(binaryHeader, 20, true);
    const bool bigEndianValid = bytesPerSample(bigEndianFormat) > 0 && bigEndianSamples > 0;
    const bool littleEndianValid = bytesPerSample(littleEndianFormat) > 0 && littleEndianSamples > 0;
    if (!bigEndianValid && !littleEndianValid)
    {
        setLastError(QObject::tr("The SEG-Y binary header has an unsupported sample format or sample count."));
        return {};
    }
    bool littleEndian = false;
    if (!bigEndianValid && littleEndianValid)
    {
        littleEndian = true;
    }
    const quint16 sampleFormat = readUInt16(binaryHeader, 24, littleEndian);
    const int binarySamplesPerTrace = readUInt16(binaryHeader, 20, littleEndian);
    const int sampleInterval = readUInt16(binaryHeader, 16, littleEndian);
    const qint16 extendedTextHeaderCount = static_cast<qint16>(readUInt16(binaryHeader, 304, littleEndian));
    if (sampleInterval <= 0 || binarySamplesPerTrace <= 0 || extendedTextHeaderCount < -1)
    {
        setLastError(QObject::tr("The SEG-Y sample interval, sample count, or extended-header count is invalid."));
        return {};
    }

    if (extendedTextHeaderCount > 0)
    {
        const qint64 extendedHeaderBytes = static_cast<qint64>(extendedTextHeaderCount) * 3200;
        if (file.pos() + extendedHeaderBytes > file.size() || !file.seek(file.pos() + extendedHeaderBytes))
        {
            setLastError(QObject::tr("The SEG-Y extended textual headers are incomplete."));
            return {};
        }
    }
    else if (extendedTextHeaderCount == -1)
    {
        bool foundEndText = false;
        while (file.pos() + 3200 <= file.size())
        {
            const QByteArray extendedHeader = file.read(3200);
            if (extendedHeader.size() != 3200)
            {
                break;
            }
            if (decodeTextHeader(extendedHeader).contains(QStringLiteral("((SEG: EndText))"),
                                                           Qt::CaseInsensitive))
            {
                foundEndText = true;
                break;
            }
        }
        if (!foundEndText)
        {
            setLastError(QObject::tr("The SEG-Y extended textual headers have no end marker."));
            return {};
        }
    }

    const int sampleBytes = bytesPerSample(sampleFormat);
    const QString decodedTextHeader = decodeTextHeader(textHeader);
    int inlineOffset = 188;
    int crosslineOffset = 192;
    traceHeaderCoordinateOffsets(decodedTextHeader, inlineOffset, crosslineOffset);
    QVector<qint64> traceSampleOffsets;
    QVector<int> traceSampleCounts;
    QVector<int> traceInlineNumbers;
    QVector<int> traceCrosslineNumbers;
    QVector<float> sampleValues;
    const qint64 estimatedTraceCount =
        (file.size() - file.pos()) / (240 + static_cast<qint64>(binarySamplesPerTrace) * sampleBytes);
    const qint64 estimatedSamples = estimatedTraceCount * binarySamplesPerTrace;
    if (estimatedSamples > std::numeric_limits<int>::max())
    {
        setLastError(QObject::tr("The SEG-Y file contains more samples than the importer can index."));
        return {};
    }
    traceSampleOffsets.reserve(static_cast<int>(estimatedTraceCount));
    traceSampleCounts.reserve(static_cast<int>(estimatedTraceCount));
    traceInlineNumbers.reserve(static_cast<int>(estimatedTraceCount));
    traceCrosslineNumbers.reserve(static_cast<int>(estimatedTraceCount));
    sampleValues.reserve(static_cast<int>(estimatedSamples));
    qint64 traceCount = 0;
    qint64 totalSampleCount = 0;
    int inlineMinimum = std::numeric_limits<int>::max();
    int inlineMaximum = std::numeric_limits<int>::min();
    int crosslineMinimum = std::numeric_limits<int>::max();
    int crosslineMaximum = std::numeric_limits<int>::min();
    while (file.pos() < file.size())
    {
        const qint64 traceHeaderPosition = file.pos();
        const QByteArray traceHeader = file.read(240);
        if (traceHeader.size() != 240)
        {
            setLastError(QObject::tr("The SEG-Y file ends inside a trace header."));
            return {};
        }
        const quint16 traceSamplesValue = readUInt16(traceHeader, 114, littleEndian);
        const int traceSamples = traceSamplesValue > 0 ? traceSamplesValue : binarySamplesPerTrace;
        const qint64 traceDataBytes = static_cast<qint64>(traceSamples) * sampleBytes;
        if (traceSamples <= 0 || file.pos() + traceDataBytes > file.size() ||
            totalSampleCount + traceSamples > std::numeric_limits<int>::max())
        {
            setLastError(QObject::tr("The SEG-Y file has an invalid or incomplete trace %1.").arg(traceCount + 1));
            return {};
        }
        const int inlineNumber = static_cast<qint32>(readUInt32(traceHeader, inlineOffset, littleEndian));
        const int crosslineNumber = static_cast<qint32>(readUInt32(traceHeader, crosslineOffset, littleEndian));
        const qint64 sampleDataPosition = file.pos();
        const QByteArray traceSamplesData = file.read(static_cast<qint64>(traceDataBytes));
        if (traceSamplesData.size() != traceDataBytes)
        {
            setLastError(QObject::tr("The SEG-Y file ends inside trace %1 samples.").arg(traceCount + 1));
            return {};
        }
        traceSampleOffsets.append(totalSampleCount);
        traceSampleCounts.append(traceSamples);
        traceInlineNumbers.append(inlineNumber);
        traceCrosslineNumbers.append(crosslineNumber);
        for (int sampleIndex = 0; sampleIndex < traceSamples; ++sampleIndex)
        {
            sampleValues.append(decodeSample(traceSamplesData, sampleIndex * sampleBytes,
                                             sampleFormat, littleEndian));
        }
        inlineMinimum = qMin(inlineMinimum, inlineNumber);
        inlineMaximum = qMax(inlineMaximum, inlineNumber);
        crosslineMinimum = qMin(crosslineMinimum, crosslineNumber);
        crosslineMaximum = qMax(crosslineMaximum, crosslineNumber);
        if (file.pos() != sampleDataPosition + traceDataBytes ||
            !file.seek(traceHeaderPosition + 240 + traceDataBytes))
        {
            setLastError(QObject::tr("Cannot seek over SEG-Y trace samples."));
            return {};
        }
        totalSampleCount += traceSamples;
        ++traceCount;
    }
    if (traceCount == 0)
    {
        setLastError(QObject::tr("The SEG-Y file contains no traces."));
        return {};
    }

    IDOSSeismicVolume* volume = new IDOSSeismicVolume();
    volume->setName(QFileInfo(filePath).completeBaseName());
    volume->setSourceFile(filePath);
    volume->setTextHeader(decodedTextHeader);
    volume->setByteOrder(littleEndian ? QStringLiteral("Little-endian") : QStringLiteral("Big-endian"));
    volume->setSampleFormat(sampleFormat);
    volume->setSampleFormatName(sampleFormatName(sampleFormat));
    volume->setSampleIntervalMicroseconds(sampleInterval);
    volume->setSamplesPerTrace(traceSampleCounts.isEmpty() ? binarySamplesPerTrace
                                                            : *std::max_element(traceSampleCounts.cbegin(),
                                                                                traceSampleCounts.cend()));
    volume->setTraceSampleOffsets(traceSampleOffsets);
    volume->setTraceSampleCounts(traceSampleCounts);
    volume->setTraceInlineNumbers(traceInlineNumbers);
    volume->setTraceCrosslineNumbers(traceCrosslineNumbers);
    volume->setSampleValues(sampleValues);
    volume->setTraceCount(traceCount);
    volume->setInlineMinimum(inlineMinimum);
    volume->setInlineMaximum(inlineMaximum);
    volume->setCrosslineMinimum(crosslineMinimum);
    volume->setCrosslineMaximum(crosslineMaximum);
    return {volume};
}

int IDOSSegyProvider::bytesPerSample(int sampleFormat) const
{
    if (sampleFormat == 1 || sampleFormat == 2 || sampleFormat == 4 || sampleFormat == 5 ||
        sampleFormat == 10)
    {
        return 4;
    }
    if (sampleFormat == 3 || sampleFormat == 11)
    {
        return 2;
    }
    if (sampleFormat == 6 || sampleFormat == 9 || sampleFormat == 12)
    {
        return 8;
    }
    if (sampleFormat == 7 || sampleFormat == 15)
    {
        return 3;
    }
    if (sampleFormat == 8 || sampleFormat == 16)
    {
        return 1;
    }
    return 0;
}

float IDOSSegyProvider::decodeSample(const QByteArray& bytes, int offset, int sampleFormat,
                                    bool littleEndian) const
{
    if (sampleFormat == 1)
    {
        const quint32 rawValue = readUInt32(bytes, offset, littleEndian);
        const int sign = (rawValue & 0x80000000U) != 0 ? -1 : 1;
        const int exponent = static_cast<int>((rawValue >> 24) & 0x7fU) - 64;
        const quint32 fraction = rawValue & 0x00ffffffU;
        return static_cast<float>(sign * std::ldexp(static_cast<double>(fraction) / 16777216.0,
                                                   exponent * 4));
    }
    if (sampleFormat == 2 || sampleFormat == 4 || sampleFormat == 10)
    {
        const quint32 value = readUInt32(bytes, offset, littleEndian);
        if (sampleFormat == 10)
        {
            return static_cast<float>(value);
        }
        return static_cast<float>(static_cast<qint32>(value));
    }
    if (sampleFormat == 3 || sampleFormat == 11)
    {
        const quint16 value = readUInt16(bytes, offset, littleEndian);
        if (sampleFormat == 11)
        {
            return static_cast<float>(value);
        }
        return static_cast<float>(static_cast<qint16>(value));
    }
    if (sampleFormat == 5)
    {
        const quint32 value = readUInt32(bytes, offset, littleEndian);
        float result = 0.0F;
        std::memcpy(&result, &value, sizeof(result));
        return result;
    }
    if (sampleFormat == 6)
    {
        const quint64 value = readUInt64(bytes, offset, littleEndian);
        double result = 0.0;
        std::memcpy(&result, &value, sizeof(result));
        return static_cast<float>(result);
    }
    if (sampleFormat == 7 || sampleFormat == 15)
    {
        quint32 value = 0;
        if (littleEndian)
        {
            value = static_cast<quint32>(static_cast<quint8>(bytes.at(offset + 2))) << 16 |
                    static_cast<quint32>(static_cast<quint8>(bytes.at(offset + 1))) << 8 |
                    static_cast<quint32>(static_cast<quint8>(bytes.at(offset)));
        }
        else
        {
            value = static_cast<quint32>(static_cast<quint8>(bytes.at(offset))) << 16 |
                    static_cast<quint32>(static_cast<quint8>(bytes.at(offset + 1))) << 8 |
                    static_cast<quint32>(static_cast<quint8>(bytes.at(offset + 2)));
        }
        quint32 signedValue = value;
        if ((value & 0x00800000U) != 0)
        {
            signedValue |= 0xff000000U;
        }
        return static_cast<float>(static_cast<qint32>(signedValue));
    }
    if (sampleFormat == 8)
    {
        return static_cast<float>(static_cast<qint8>(bytes.at(offset)));
    }
    if (sampleFormat == 9 || sampleFormat == 12)
    {
        const quint64 value = readUInt64(bytes, offset, littleEndian);
        if (sampleFormat == 12)
        {
            return static_cast<float>(value);
        }
        return static_cast<float>(static_cast<qint64>(value));
    }
    if (sampleFormat == 16)
    {
        return static_cast<float>(static_cast<quint8>(bytes.at(offset)));
    }
    return 0.0F;
}

QString IDOSSegyProvider::sampleFormatName(int sampleFormat) const
{
    if (sampleFormat == 1)
    {
        return QStringLiteral("IBM 32-bit floating point");
    }
    if (sampleFormat == 2)
    {
        return QStringLiteral("32-bit integer");
    }
    if (sampleFormat == 3)
    {
        return QStringLiteral("16-bit integer");
    }
    if (sampleFormat == 5)
    {
        return QStringLiteral("IEEE 32-bit floating point");
    }
    if (sampleFormat == 6)
    {
        return QStringLiteral("IEEE 64-bit floating point");
    }
    if (sampleFormat == 8 || sampleFormat == 16)
    {
        return sampleFormat == 8 ? QStringLiteral("8-bit integer")
                                 : QStringLiteral("8-bit unsigned integer");
    }
    return QObject::tr("SEG-Y sample format %1").arg(sampleFormat);
}

quint16 IDOSSegyProvider::readUInt16(const QByteArray& bytes, int offset, bool littleEndian) const
{
    if (offset < 0 || offset + 1 >= bytes.size())
    {
        return 0;
    }
    const quint16 first = static_cast<quint8>(bytes.at(offset));
    const quint16 second = static_cast<quint8>(bytes.at(offset + 1));
    return littleEndian ? static_cast<quint16>(first | (second << 8))
                        : static_cast<quint16>((first << 8) | second);
}

quint32 IDOSSegyProvider::readUInt32(const QByteArray& bytes, int offset, bool littleEndian) const
{
    if (offset < 0 || offset + 3 >= bytes.size())
    {
        return 0;
    }
    const quint32 first = static_cast<quint8>(bytes.at(offset));
    const quint32 second = static_cast<quint8>(bytes.at(offset + 1));
    const quint32 third = static_cast<quint8>(bytes.at(offset + 2));
    const quint32 fourth = static_cast<quint8>(bytes.at(offset + 3));
    if (littleEndian)
    {
        return first | (second << 8) | (third << 16) | (fourth << 24);
    }
    return (first << 24) | (second << 16) | (third << 8) | fourth;
}

quint64 IDOSSegyProvider::readUInt64(const QByteArray& bytes, int offset, bool littleEndian) const
{
    if (offset < 0 || offset + 7 >= bytes.size())
    {
        return 0;
    }
    quint64 value = 0;
    for (int index = 0; index < 8; ++index)
    {
        if (littleEndian)
        {
            value |= static_cast<quint64>(static_cast<quint8>(bytes.at(offset + index))) << (index * 8);
        }
        else
        {
            value = (value << 8) | static_cast<quint8>(bytes.at(offset + index));
        }
    }
    return value;
}

QString IDOSSegyProvider::decodeTextHeader(const QByteArray& bytes) const
{
    if (bytes.isEmpty() || static_cast<quint8>(bytes.at(0)) < 0x80)
    {
        return QString::fromLatin1(bytes);
    }
    QTextCodec* codec = QTextCodec::codecForName("IBM 037");
    if (codec != nullptr)
    {
        return codec->toUnicode(bytes);
    }
    return QString::fromLatin1(bytes);
}

bool IDOSSegyProvider::traceHeaderCoordinateOffsets(const QString& textHeader, int& inlineOffset,
                                                     int& crosslineOffset) const
{
    const QRegularExpression expression(
        QStringLiteral("Inline\\s+(\\d+)\\s+Xline\\s+(\\d+)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = expression.match(textHeader);
    if (!match.hasMatch())
    {
        return false;
    }
    bool inlineValid = false;
    bool crosslineValid = false;
    const int inlineByte = match.captured(1).toInt(&inlineValid);
    const int crosslineByte = match.captured(2).toInt(&crosslineValid);
    if (!inlineValid || !crosslineValid || inlineByte < 1 || crosslineByte < 1 ||
        inlineByte > 237 || crosslineByte > 237)
    {
        return false;
    }
    inlineOffset = inlineByte - 1;
    crosslineOffset = crosslineByte - 1;
    return true;
}
