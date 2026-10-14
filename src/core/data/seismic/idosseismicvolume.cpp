#include "idosseismicvolume.h"

IDOSSeismicVolume::IDOSSeismicVolume(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_sourceFile()
    , m_textHeader()
    , m_byteOrder()
    , m_sampleFormat(0)
    , m_sampleFormatName()
    , m_sampleIntervalMicroseconds(0)
    , m_samplesPerTrace(0)
    , m_traceSampleOffsets()
    , m_traceSampleCounts()
    , m_traceInlineNumbers()
    , m_traceCrosslineNumbers()
    , m_sampleValues()
    , m_traceCount(0)
    , m_inlineMinimum(0)
    , m_inlineMaximum(0)
    , m_crosslineMinimum(0)
    , m_crosslineMaximum(0)
{
}

QString IDOSSeismicVolume::typeId() const
{
    return QStringLiteral("idos.seismic.volume");
}

QString IDOSSeismicVolume::sourceFile() const
{
    return m_sourceFile;
}

void IDOSSeismicVolume::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
}

QString IDOSSeismicVolume::textHeader() const
{
    return m_textHeader;
}

void IDOSSeismicVolume::setTextHeader(const QString& textHeader)
{
    m_textHeader = textHeader;
}

QString IDOSSeismicVolume::byteOrder() const
{
    return m_byteOrder;
}

void IDOSSeismicVolume::setByteOrder(const QString& byteOrder)
{
    m_byteOrder = byteOrder;
}

int IDOSSeismicVolume::sampleFormat() const
{
    return m_sampleFormat;
}

void IDOSSeismicVolume::setSampleFormat(int sampleFormat)
{
    m_sampleFormat = sampleFormat;
}

QString IDOSSeismicVolume::sampleFormatName() const
{
    return m_sampleFormatName;
}

void IDOSSeismicVolume::setSampleFormatName(const QString& sampleFormatName)
{
    m_sampleFormatName = sampleFormatName;
}

int IDOSSeismicVolume::sampleIntervalMicroseconds() const
{
    return m_sampleIntervalMicroseconds;
}

void IDOSSeismicVolume::setSampleIntervalMicroseconds(int sampleIntervalMicroseconds)
{
    m_sampleIntervalMicroseconds = sampleIntervalMicroseconds;
}

int IDOSSeismicVolume::samplesPerTrace() const
{
    return m_samplesPerTrace;
}

void IDOSSeismicVolume::setSamplesPerTrace(int samplesPerTrace)
{
    m_samplesPerTrace = samplesPerTrace;
}

QVector<qint64> IDOSSeismicVolume::traceSampleOffsets() const
{
    return m_traceSampleOffsets;
}

void IDOSSeismicVolume::setTraceSampleOffsets(const QVector<qint64>& offsets)
{
    m_traceSampleOffsets = offsets;
}

QVector<int> IDOSSeismicVolume::traceSampleCounts() const
{
    return m_traceSampleCounts;
}

void IDOSSeismicVolume::setTraceSampleCounts(const QVector<int>& counts)
{
    m_traceSampleCounts = counts;
}

QVector<int> IDOSSeismicVolume::traceInlineNumbers() const
{
    return m_traceInlineNumbers;
}

void IDOSSeismicVolume::setTraceInlineNumbers(const QVector<int>& numbers)
{
    m_traceInlineNumbers = numbers;
}

QVector<int> IDOSSeismicVolume::traceCrosslineNumbers() const
{
    return m_traceCrosslineNumbers;
}

void IDOSSeismicVolume::setTraceCrosslineNumbers(const QVector<int>& numbers)
{
    m_traceCrosslineNumbers = numbers;
}

QVector<float> IDOSSeismicVolume::sampleValues() const
{
    return m_sampleValues;
}

void IDOSSeismicVolume::setSampleValues(const QVector<float>& values)
{
    m_sampleValues = values;
}

qint64 IDOSSeismicVolume::traceCount() const
{
    return m_traceCount;
}

void IDOSSeismicVolume::setTraceCount(qint64 traceCount)
{
    m_traceCount = traceCount;
}

int IDOSSeismicVolume::inlineMinimum() const
{
    return m_inlineMinimum;
}

void IDOSSeismicVolume::setInlineMinimum(int inlineMinimum)
{
    m_inlineMinimum = inlineMinimum;
}

int IDOSSeismicVolume::inlineMaximum() const
{
    return m_inlineMaximum;
}

void IDOSSeismicVolume::setInlineMaximum(int inlineMaximum)
{
    m_inlineMaximum = inlineMaximum;
}

int IDOSSeismicVolume::crosslineMinimum() const
{
    return m_crosslineMinimum;
}

void IDOSSeismicVolume::setCrosslineMinimum(int crosslineMinimum)
{
    m_crosslineMinimum = crosslineMinimum;
}

int IDOSSeismicVolume::crosslineMaximum() const
{
    return m_crosslineMaximum;
}

void IDOSSeismicVolume::setCrosslineMaximum(int crosslineMaximum)
{
    m_crosslineMaximum = crosslineMaximum;
}
