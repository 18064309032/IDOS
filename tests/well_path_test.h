#ifndef WELL_PATH_TEST_H
#define WELL_PATH_TEST_H

#include <QObject>

class WellPathTest : public QObject
{
    Q_OBJECT

  public:
    WellPathTest();

  private slots:
    void onParseSample();
    void onInvalidInput();
    void onResolveGeometry();
};

#endif // WELL_PATH_TEST_H
