#ifndef WELL_IMPORT_TEST_H
#define WELL_IMPORT_TEST_H

#include <QByteArray>
#include <QObject>
#include <QString>

class WellImportTest : public QObject
{
    Q_OBJECT

  public:
    WellImportTest();

  private slots:
    void onCommandMergeUndoRedo();
    void onTargetedCommandAndFailure();
    void onCommandRestoresLogChannels();
    void onSampleFiles();
    void onPetrelWellHeader();
    void onInvalidRows();
    void onReorderedColumns();
    void onLasImport();
    void onInvalidLasFiles();
    void onExtendedInputProviders();
    void onExtendedInputUndoRedo();
    void onLegacyExtendedSamples();
    void onSharedWellTextFormats();

  private:
    bool writeInput(const QString& path, const QByteArray& content);
};

#endif
