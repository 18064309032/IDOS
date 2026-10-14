#ifndef CASE_WORKFLOW_TEST_H
#define CASE_WORKFLOW_TEST_H

#include <QObject>
#include <QString>
#include <QVector>

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
    void onPlatformSimulation();
    void onGridSettings();
    void onGridFilters();
    void onGridFilterCriteria();
    void onWellInputMenus();
    void onWellDisplaySettings();
    void onWellSymbolsAndStatusColors();
    void onRenderLifecycle();
    void onRenderArchitecture();
    void onExamplePlugin();
    void onCreateAndImport();
    void onFluidRockReferences();
    void onDialog();
    void onFlowExecution();
    void onSimulationOutput();
    void onFlowOutputFraming();

  private:
    bool sameSelection(const QVector<quint8>& first, const QVector<quint8>& second) const;
    void onCloseContextMenu();
    IDOSRenderObject* findRenderObject(IDOSRenderView* view, const QString& dataId) const;
    QString m_name;
    bool m_cancel;
    bool m_expectDisabled;
};
#endif
