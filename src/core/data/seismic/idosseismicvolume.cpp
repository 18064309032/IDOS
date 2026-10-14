#include <QObject>

#include "idosproject.h"

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

bool IDOSSeismicVolume::canClone() const
{
    return true;
}

IDOSDataObject* IDOSSeismicVolume::clone() const
{
    IDOSSeismicVolume* copy = new IDOSSeismicVolume();
    copy->setName(name());
    copy->setVisible(isVisible());
    copy->m_sourceFile = m_sourceFile;
    copy->m_textHeader = m_textHeader;
    copy->m_byteOrder = m_byteOrder;
    copy->m_sampleFormat = m_sampleFormat;
    copy->m_sampleFormatName = m_sampleFormatName;
    copy->m_sampleIntervalMicroseconds = m_sampleIntervalMicroseconds;
    copy->m_samplesPerTrace = m_samplesPerTrace;
    copy->m_traceSampleOffsets = m_traceSampleOffsets;
    copy->m_traceSampleCounts = m_traceSampleCounts;
    copy->m_traceInlineNumbers = m_traceInlineNumbers;
    copy->m_traceCrosslineNumbers = m_traceCrosslineNumbers;
    copy->m_sampleValues = m_sampleValues;
    copy->m_traceCount = m_traceCount;
    copy->m_inlineMinimum = m_inlineMinimum;
    copy->m_inlineMaximum = m_inlineMaximum;
    copy->m_crosslineMinimum = m_crosslineMinimum;
    copy->m_crosslineMaximum = m_crosslineMaximum;
    return copy;
}

QVariantMap IDOSSeismicVolume::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("textHeader"), QVariant::fromValue(m_textHeader));
    data.insert(QStringLiteral("byteOrder"), QVariant::fromValue(m_byteOrder));
    data.insert(QStringLiteral("sampleFormat"), QVariant::fromValue(m_sampleFormat));
    data.insert(QStringLiteral("sampleFormatName"), QVariant::fromValue(m_sampleFormatName));
    data.insert(QStringLiteral("sampleIntervalMicroseconds"), QVariant::fromValue(m_sampleIntervalMicroseconds));
    data.insert(QStringLiteral("samplesPerTrace"), QVariant::fromValue(m_samplesPerTrace));
    data.insert(QStringLiteral("traceSampleOffsets"), QVariant::fromValue(m_traceSampleOffsets));
    data.insert(QStringLiteral("traceSampleCounts"), QVariant::fromValue(m_traceSampleCounts));
    data.insert(QStringLiteral("traceInlineNumbers"), QVariant::fromValue(m_traceInlineNumbers));
    data.insert(QStringLiteral("traceCrosslineNumbers"), QVariant::fromValue(m_traceCrosslineNumbers));
    data.insert(QStringLiteral("sampleValues"), QVariant::fromValue(m_sampleValues));
    data.insert(QStringLiteral("traceCount"), QVariant::fromValue(m_traceCount));
    data.insert(QStringLiteral("inlineMinimum"), QVariant::fromValue(m_inlineMinimum));
    data.insert(QStringLiteral("inlineMaximum"), QVariant::fromValue(m_inlineMaximum));
    data.insert(QStringLiteral("crosslineMinimum"), QVariant::fromValue(m_crosslineMinimum));
    data.insert(QStringLiteral("crosslineMaximum"), QVariant::fromValue(m_crosslineMaximum));
    return data;
}

bool IDOSSeismicVolume::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    if (!data.contains(QStringLiteral("textHeader")) || data.value(QStringLiteral("textHeader")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("textHeader"));
        return false;
    }
    if (!data.contains(QStringLiteral("byteOrder")) || data.value(QStringLiteral("byteOrder")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("byteOrder"));
        return false;
    }
    if (!data.contains(QStringLiteral("sampleFormat")) || data.value(QStringLiteral("sampleFormat")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sampleFormat"));
        return false;
    }
    if (!data.contains(QStringLiteral("sampleFormatName")) || data.value(QStringLiteral("sampleFormatName")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sampleFormatName"));
        return false;
    }
    if (!data.contains(QStringLiteral("sampleIntervalMicroseconds")) || data.value(QStringLiteral("sampleIntervalMicroseconds")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sampleIntervalMicroseconds"));
        return false;
    }
    if (!data.contains(QStringLiteral("samplesPerTrace")) || data.value(QStringLiteral("samplesPerTrace")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("samplesPerTrace"));
        return false;
    }
    if (!data.contains(QStringLiteral("traceSampleOffsets")) || data.value(QStringLiteral("traceSampleOffsets")).userType() != qMetaTypeId<QVector<qint64>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("traceSampleOffsets"));
        return false;
    }
    if (!data.contains(QStringLiteral("traceSampleCounts")) || data.value(QStringLiteral("traceSampleCounts")).userType() != qMetaTypeId<QVector<int>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("traceSampleCounts"));
        return false;
    }
    if (!data.contains(QStringLiteral("traceInlineNumbers")) || data.value(QStringLiteral("traceInlineNumbers")).userType() != qMetaTypeId<QVector<int>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("traceInlineNumbers"));
        return false;
    }
    if (!data.contains(QStringLiteral("traceCrosslineNumbers")) || data.value(QStringLiteral("traceCrosslineNumbers")).userType() != qMetaTypeId<QVector<int>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("traceCrosslineNumbers"));
        return false;
    }
    if (!data.contains(QStringLiteral("sampleValues")) || data.value(QStringLiteral("sampleValues")).userType() != qMetaTypeId<QVector<float>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sampleValues"));
        return false;
    }
    if (!data.contains(QStringLiteral("traceCount")) || data.value(QStringLiteral("traceCount")).userType() != qMetaTypeId<qint64>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("traceCount"));
        return false;
    }
    if (!data.contains(QStringLiteral("inlineMinimum")) || data.value(QStringLiteral("inlineMinimum")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("inlineMinimum"));
        return false;
    }
    if (!data.contains(QStringLiteral("inlineMaximum")) || data.value(QStringLiteral("inlineMaximum")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("inlineMaximum"));
        return false;
    }
    if (!data.contains(QStringLiteral("crosslineMinimum")) || data.value(QStringLiteral("crosslineMinimum")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("crosslineMinimum"));
        return false;
    }
    if (!data.contains(QStringLiteral("crosslineMaximum")) || data.value(QStringLiteral("crosslineMaximum")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("crosslineMaximum"));
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_textHeader = data.value(QStringLiteral("textHeader")).value<QString>();
    m_byteOrder = data.value(QStringLiteral("byteOrder")).value<QString>();
    m_sampleFormat = data.value(QStringLiteral("sampleFormat")).value<int>();
    m_sampleFormatName = data.value(QStringLiteral("sampleFormatName")).value<QString>();
    m_sampleIntervalMicroseconds = data.value(QStringLiteral("sampleIntervalMicroseconds")).value<int>();
    m_samplesPerTrace = data.value(QStringLiteral("samplesPerTrace")).value<int>();
    m_traceSampleOffsets = data.value(QStringLiteral("traceSampleOffsets")).value<QVector<qint64>>();
    m_traceSampleCounts = data.value(QStringLiteral("traceSampleCounts")).value<QVector<int>>();
    m_traceInlineNumbers = data.value(QStringLiteral("traceInlineNumbers")).value<QVector<int>>();
    m_traceCrosslineNumbers = data.value(QStringLiteral("traceCrosslineNumbers")).value<QVector<int>>();
    m_sampleValues = data.value(QStringLiteral("sampleValues")).value<QVector<float>>();
    m_traceCount = data.value(QStringLiteral("traceCount")).value<qint64>();
    m_inlineMinimum = data.value(QStringLiteral("inlineMinimum")).value<int>();
    m_inlineMaximum = data.value(QStringLiteral("inlineMaximum")).value<int>();
    m_crosslineMinimum = data.value(QStringLiteral("crosslineMinimum")).value<int>();
    m_crosslineMaximum = data.value(QStringLiteral("crosslineMaximum")).value<int>();
    return true;
}
bool IDOSSeismicVolume::validateProjectReferences(const IDOSProject* project, QString& error) const
{
    Q_UNUSED(project);
    if (m_traceCount < 0 || m_traceCount != m_traceSampleOffsets.size() ||
        m_traceCount != m_traceSampleCounts.size() || m_traceCount != m_traceInlineNumbers.size() ||
        m_traceCount != m_traceCrosslineNumbers.size())
    {
        error = tr("Stored seismic trace arrays have inconsistent lengths.");
        return false;
    }
    for (int index = 0; index < m_traceSampleOffsets.size(); ++index)
    {
        if (m_traceSampleOffsets.at(index) < 0 || m_traceSampleCounts.at(index) < 0 ||
            m_traceSampleOffsets.at(index) > m_sampleValues.size() - static_cast<qint64>(m_traceSampleCounts.at(index)))
        {
            error = tr("Stored seismic samples do not contain a complete trace.");
            return false;
        }
    }
    return true;
}
