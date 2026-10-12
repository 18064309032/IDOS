#ifndef CASE_WORKFLOW_TEST_H
#define CASE_WORKFLOW_TEST_H

#include <QObject>
#include <QString>

class IDOSRenderObject;
class IDOSRenderView;

class CaseWorkflowTest : public QObject
{
    Q_OBJECT

  public:
    CaseWorkflowTest();
  private slots:
    void onProvider();
    void onProviderDimensions();
    void onViewDecorations();
    void onRenderLifecycle();
    void onRenderArchitecture();
    void onExamplePlugin();
    void onCreateAndImport();
    void onDialog();
    void onFlowExecution();
    void onSimulationOutput();
    void onFlowOutputFraming();

  private:
    void onCloseContextMenu();
    IDOSRenderObject* findRenderObject(IDOSRenderView* view, const QString& dataId) const;
    QString m_name;
    bool m_cancel;
    bool m_expectDisabled;
};
#endif
