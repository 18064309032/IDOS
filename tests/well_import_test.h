#ifndef WELL_IMPORT_TEST_H
#define WELL_IMPORT_TEST_H

#include <QObject>

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
    void onInvalidRows();
    void onReorderedColumns();
    void onLasImport();
    void onInvalidLasFiles();
};

#endif
