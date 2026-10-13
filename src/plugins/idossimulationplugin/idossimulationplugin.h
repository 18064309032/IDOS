#ifndef IDOS_SIMULATION_PLUGIN_H
#define IDOS_SIMULATION_PLUGIN_H

#include <memory>

#include <QObject>
#include <QPointer>
#include <QString>

#include "idosplugin.h"

class QAction;
class IDOSColoringTreeGuiProvider;
class IDOSDataObject;
class IDOSInterface;
class IDOSProject;
class IDOSSimulationCaseInventoryReportWidget;
class IDOSSimulationCaseTableWidget;
class IDOSSimulationCaseObject;
class IDOSSimulationCurveComparisonWidget;
class IDOSSimulationDataGenerationWidget;
class IDOSSimulationDeckBrowserWidget;
class IDOSSimulationInitializeWidget;
class IDOSSimulationParseSummaryWidget;
class IDOSSimulationPreflightWidget;
class IDOSSimulationPropertyCurveWidget;
class IDOSSimulationResultFilesWidget;
class IDOSSimulationResultPlaybackWidget;
class IDOSSimulationRunStatusWidget;
class IDOSSimulationRunArchiveWidget;
class IDOSSimulationWorkflowWidget;
class IDOSSimulationWellControlsWidget;
class IDOSToolMetadata;
class SARibbonPanel;
class QTranslator;

class IDOSSimulationPlugin : public QObject, public IDOSPlugin
{
    Q_OBJECT

  public:
    explicit IDOSSimulationPlugin(IDOSInterface* interface);
    ~IDOSSimulationPlugin() override;

    void initGui() override;
    void unload() override;
    bool canUnload(QString& reason) const override;

  private:
    void uninstallTranslator();
    void removeHomeActions();
    void onProjectChanged(IDOSProject* project);
    void onCurrentObjectChanged(IDOSDataObject* object);
    void onProjectDataChanged(const QString& objectId);
    void updateActionAvailability();
    void onCaseTableTriggered();
    void onParseSummaryTriggered();
    void onGenerateDataTriggered();
    void onDeckBrowserTriggered();
    void onPropertyCurvesTriggered();
    void onCurveComparisonTriggered();
    void onWellControlsTriggered();
    void onRunPreflightTriggered();
    void onInitializeTriggered();
    void onRunStatusTriggered();
    void onStopTaskTriggered();
    void onResultFilesTriggered();
    void onRunArchiveTriggered();
    void onCaseInventoryReportTriggered();
    void onWorkflowTriggered();

  private:
    IDOSInterface* m_interface;
    QPointer<IDOSProject> m_project;
    QPointer<SARibbonPanel> m_simulationPanel;
    QPointer<SARibbonPanel> m_resultsPanel;
    QPointer<SARibbonPanel> m_outputPanel;
    QTranslator* m_translator;
    QAction* m_parseSummaryAction;
    QAction* m_generateDataAction;
    QAction* m_deckBrowserAction;
    QAction* m_caseTableAction;
    QAction* m_propertyCurvesAction;
    QAction* m_curveComparisonAction;
    QAction* m_wellControlsAction;
    QAction* m_runPreflightAction;
    QAction* m_initializeAction;
    QAction* m_runStatusAction;
    QAction* m_stopTaskAction;
    QAction* m_resultFilesAction;
    QAction* m_runArchiveAction;
    QAction* m_caseInventoryReportAction;
    QAction* m_workflowAction;
    IDOSToolMetadata* m_renderToolMetadata;
    std::shared_ptr<IDOSColoringTreeGuiProvider> m_coloringTreeGuiProvider;
    QPointer<IDOSSimulationCaseTableWidget> m_caseTableWidget;
    QPointer<IDOSSimulationParseSummaryWidget> m_parseSummaryWidget;
    QPointer<IDOSSimulationDataGenerationWidget> m_generateDataWidget;
    QPointer<IDOSSimulationDeckBrowserWidget> m_deckBrowserWidget;
    QPointer<IDOSSimulationPropertyCurveWidget> m_propertyCurvesWidget;
    QPointer<IDOSSimulationCurveComparisonWidget> m_curveComparisonWidget;
    QPointer<IDOSSimulationWellControlsWidget> m_wellControlsWidget;
    QPointer<IDOSSimulationPreflightWidget> m_runPreflightWidget;
    QPointer<IDOSSimulationInitializeWidget> m_initializeWidget;
    QPointer<IDOSSimulationRunStatusWidget> m_runStatusWidget;
    QPointer<IDOSSimulationResultFilesWidget> m_resultFilesWidget;
    QPointer<IDOSSimulationResultPlaybackWidget> m_resultPlaybackWidget;
    QPointer<IDOSSimulationRunArchiveWidget> m_runArchiveWidget;
    QPointer<IDOSSimulationCaseInventoryReportWidget> m_caseInventoryReportWidget;
    QPointer<IDOSSimulationWorkflowWidget> m_workflowWidget;
    bool m_initialized;
};

#endif // IDOS_SIMULATION_PLUGIN_H
