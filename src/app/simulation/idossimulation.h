#ifndef IDOS_SIMULATION_H
#define IDOS_SIMULATION_H

#include <QObject>
#include <QPointer>
#include <QString>

#include "idos_app.h"

class QAction;
class IDOSDataObject;
class IDOSMainWindow;
class IDOSProject;
class IDOSSimulationCaseInventoryReportWidget;
class IDOSSimulationCaseTableWidget;
class IDOSSimulationCaseObject;
class IDOSSimulationCurveComparisonWidget;
class IDOSSimulationDataGenerationWidget;
class IDOSSimulationInitializeWidget;
class IDOSSimulationPropertyCurveWidget;
class IDOSSimulationResultFilesWidget;
class IDOSSimulationResultPlaybackWidget;
class IDOSSimulationRunStatusWidget;
class IDOSSimulationRunArchiveWidget;
class IDOSSimulationWellControlsWidget;
class SARibbonPanel;
class QLabel;
class QLineEdit;
class QScrollArea;
class QWidget;

class APP_EXPORT IDOSSimulation : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSSimulation(IDOSMainWindow* window);
    ~IDOSSimulation() override;

    void initialize();

  public slots:
    void onGenerateInputRequested(IDOSProject* project, const QString& caseId);
    void onPropertyCurvesRequested(IDOSProject* project, const QString& caseId);

  private:
    void shutdown();
    void removeHomeActions();
    void onProjectChanged(IDOSProject* project);
    void onCurrentObjectChanged(IDOSDataObject* object);
    void onCurrentCaseChanged(const QString& caseId);
    void onProjectDataChanged();
    void refreshWorkspace();
    void updateActionAvailability();
    void rebuildWorkspace(IDOSSimulationCaseObject* simulationCase);
    IDOSSimulationCaseObject* currentSimulationCase(IDOSProject* project) const;
    void showPropertyCurves(IDOSProject* project, const QString& caseId);
    void onCaseTableTriggered();
    void onPropertyCurvesTriggered();
    void onCurveComparisonTriggered();
    void onWellControlsTriggered();
    void onRunRequested();
    void onStopTaskTriggered();
    void onRunArchiveTriggered();
    void onCaseInventoryReportTriggered();
    void onWorkflowTriggered();
    void onRunFinished(bool succeeded);
    void onRunHistorySelected(const QString& outputDirectory);
    void onRunSettingsRequested();
    void onBrowseRunDirectoryRootTriggered();

  private:
    IDOSMainWindow* m_window;
    QPointer<IDOSProject> m_project;
    QPointer<SARibbonPanel> m_simulationPanel;
    QPointer<SARibbonPanel> m_caseViewPanel;
    QPointer<SARibbonPanel> m_resultsPanel;
    QPointer<SARibbonPanel> m_outputPanel;

    QAction* m_caseTableAction;
    QAction* m_propertyCurvesAction;
    QAction* m_curveComparisonAction;
    QAction* m_wellControlsAction;
    QAction* m_runStatusAction;
    QAction* m_runSettingsAction;
    QAction* m_stopTaskAction;
    QAction* m_runArchiveAction;
    QAction* m_caseInventoryReportAction;
    QAction* m_workflowAction;
    QPointer<IDOSSimulationCaseTableWidget> m_caseTableWidget;
    QPointer<IDOSSimulationDataGenerationWidget> m_generateDataWidget;
    QPointer<IDOSSimulationPropertyCurveWidget> m_propertyCurvesWidget;
    QPointer<IDOSSimulationCurveComparisonWidget> m_curveComparisonWidget;
    QPointer<IDOSSimulationWellControlsWidget> m_wellControlsWidget;
    QPointer<IDOSSimulationInitializeWidget> m_runPreparer;
    QPointer<IDOSSimulationRunStatusWidget> m_runStatusWidget;
    QPointer<IDOSSimulationResultFilesWidget> m_resultFilesWidget;
    QPointer<IDOSSimulationResultPlaybackWidget> m_resultPlaybackWidget;
    QPointer<IDOSSimulationRunArchiveWidget> m_runArchiveWidget;
    QPointer<IDOSSimulationCaseInventoryReportWidget> m_caseInventoryReportWidget;
    QWidget* m_workspaceWidget;
    QScrollArea* m_runScrollArea;
    QLabel* m_workspaceCaseLabel;
    QLineEdit* m_runSettingsRootEdit;
    bool m_initialized;
};

#endif // IDOS_SIMULATION_H
