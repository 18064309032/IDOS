#ifndef SEISMIC_IMPORT_TEST_H
#define SEISMIC_IMPORT_TEST_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QtGlobal>

class SeismicImportTest : public QObject
{
    Q_OBJECT

  public:
    SeismicImportTest();

  private slots:
    void onSegyCustomHeaderAndSamples();

  private:
    QByteArray createSegyFile() const;
    void writeUInt16(QByteArray* bytes, int offset, quint16 value) const;
    void writeUInt32(QByteArray* bytes, int offset, quint32 value) const;
};

#endif // SEISMIC_IMPORT_TEST_H
