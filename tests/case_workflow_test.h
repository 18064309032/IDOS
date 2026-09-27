#ifndef CASE_WORKFLOW_TEST_H
#define CASE_WORKFLOW_TEST_H
#include <QObject>
#include <QString>
class CaseWorkflowTest : public QObject
{
    Q_OBJECT
  private slots:
    void onProvider();
    void onCreateAndImport();
    void onDialog();

  private:
    QString m_name;
    bool m_cancel;
    bool m_expectDisabled;
};
#endif
