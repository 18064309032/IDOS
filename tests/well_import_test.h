#ifndef WELL_IMPORT_TEST_H
#define WELL_IMPORT_TEST_H
#include <QObject>
class WellImportTest : public QObject
{
    Q_OBJECT
  private slots:
    void onSampleFiles();
    void onInvalidRows();
    void onReorderedColumns();
};
#endif
