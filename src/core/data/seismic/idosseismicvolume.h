#ifndef IDOS_SEISMIC_VOLUME_H
#define IDOS_SEISMIC_VOLUME_H

#include <QString>
#include <QVector>

#include "idosdataobject.h"

class CORE_EXPORT IDOSSeismicVolume : public IDOSDataObject
{
    Q_OBJECT

  public:
    explicit IDOSSeismicVolume(QObject* parent = nullptr);

    QString typeId() const override;
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    QString textHeader() const;
    void setTextHeader(const QString& textHeader);
    QString byteOrder() const;
    void setByteOrder(const QString& byteOrder);
    int sampleFormat() const;
    void setSampleFormat(int sampleFormat);
    QString sampleFormatName() const;
    void setSampleFormatName(const QString& sampleFormatName);
    int sampleIntervalMicroseconds() const;
    void setSampleIntervalMicroseconds(int sampleIntervalMicroseconds);
    int samplesPerTrace() const;
    void setSamplesPerTrace(int samplesPerTrace);
    QVector<qint64> traceSampleOffsets() const;
    void setTraceSampleOffsets(const QVector<qint64>& offsets);
    QVector<int> traceSampleCounts() const;
    void setTraceSampleCounts(const QVector<int>& counts);
    QVector<int> traceInlineNumbers() const;
    void setTraceInlineNumbers(const QVector<int>& numbers);
    QVector<int> traceCrosslineNumbers() const;
    void setTraceCrosslineNumbers(const QVector<int>& numbers);
    QVector<float> sampleValues() const;
    void setSampleValues(const QVector<float>& values);
    qint64 traceCount() const;
    void setTraceCount(qint64 traceCount);
    int inlineMinimum() const;
    void setInlineMinimum(int inlineMinimum);
    int inlineMaximum() const;
    void setInlineMaximum(int inlineMaximum);
    int crosslineMinimum() const;
    void setCrosslineMinimum(int crosslineMinimum);
    int crosslineMaximum() const;
    void setCrosslineMaximum(int crosslineMaximum);

  private:
    QString m_sourceFile;
    QString m_textHeader;
    QString m_byteOrder;
    int m_sampleFormat;
    QString m_sampleFormatName;
    int m_sampleIntervalMicroseconds;
    int m_samplesPerTrace;
    QVector<qint64> m_traceSampleOffsets;
    QVector<int> m_traceSampleCounts;
    QVector<int> m_traceInlineNumbers;
    QVector<int> m_traceCrosslineNumbers;
    QVector<float> m_sampleValues;
    qint64 m_traceCount;
    int m_inlineMinimum;
    int m_inlineMaximum;
    int m_crosslineMinimum;
    int m_crosslineMaximum;
};

#endif // IDOS_SEISMIC_VOLUME_H
