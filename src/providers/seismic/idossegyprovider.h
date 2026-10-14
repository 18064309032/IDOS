#ifndef IDOS_SEGY_PROVIDER_H
#define IDOS_SEGY_PROVIDER_H

#include <QByteArray>

#include "idosdataprovider.h"

class PROVIDERS_EXPORT IDOSSegyProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    int bytesPerSample(int sampleFormat) const;
    QString sampleFormatName(int sampleFormat) const;
    float decodeSample(const QByteArray& bytes, int offset, int sampleFormat, bool littleEndian) const;
    quint16 readUInt16(const QByteArray& bytes, int offset, bool littleEndian) const;
    quint32 readUInt32(const QByteArray& bytes, int offset, bool littleEndian) const;
    quint64 readUInt64(const QByteArray& bytes, int offset, bool littleEndian) const;
    QString decodeTextHeader(const QByteArray& bytes) const;
    bool traceHeaderCoordinateOffsets(const QString& textHeader, int& inlineOffset,
                                     int& crosslineOffset) const;
};

#endif // IDOS_SEGY_PROVIDER_H
