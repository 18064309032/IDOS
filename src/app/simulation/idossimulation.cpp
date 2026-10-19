#include <stdexcept>

#include <QAction>
#include <QCoreApplication>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QSpinBox>
#include <QString>
#include <QTemporaryDir>
#include <QTimer>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QWidget>

#include <SARibbonPanel.h>
#include <SARibbonPanelItem.h>
#include <SARibbonPanelLayout.h>

#include "case/idossimulationcaseeclipseprovider.h"
#include "case/idossimulationcaseinventoryreportwidget.h"
#include "case/idossimulationcasetablewidget.h"
#include "case/idossimulationcurvecomparisonwidget.h"
#include "case/idossimulationdatagenerationwidget.h"
#include "case/idossimulationinitializewidget.h"
#include "case/idossimulationpreflightwidget.h"
#include "case/idossimulationpropertycurvewidget.h"
#include "case/idossimulationresultfileswidget.h"
#include "case/idossimulationresultplaybackwidget.h"
#include "case/idossimulationrunarchivewidget.h"
#include "case/idossimulationrunstatuswidget.h"
#include "case/idossimulationwellcontrolswidget.h"
#include "data/case/idossimulationcaseobject.h"
#include "idosmainwindow.h"
#include "idosproject.h"
#include "logger/idosloglevel.h"
#include "logger/idoslogger.h"

#include "idossimulation.h"

IDOSSimulation::IDOSSimulation(IDOSMainWindow* window)
    :
    QObject(window)
    , m_window(window)
    , m_project(nullptr)
    , m_simulationPanel(nullptr)
    , m_caseViewPanel(nullptr)
    , m_resultsPanel(nullptr)
    , m_outputPanel(nullptr)
    , m_caseTableAction(nullptr)
    , m_propertyCurvesAction(nullptr)
    , m_curveComparisonAction(nullptr)
    , m_wellControlsAction(nullptr)
    , m_runStatusAction(nullptr)
    , m_runSettingsAction(nullptr)
    , m_stopTaskAction(nullptr)
    , m_runArchiveAction(nullptr)
    , m_caseInventoryReportAction(nullptr)
    , m_workflowAction(nullptr)
    , m_caseTableWidget(nullptr)
    , m_generateDataWidget(nullptr)
    , m_propertyCurvesWidget(nullptr)
    , m_curveComparisonWidget(nullptr)
    , m_wellControlsWidget(nullptr)
    , m_runPreparer(nullptr)
    , m_runStatusWidget(nullptr)
    , m_resultFilesWidget(nullptr)
    , m_resultPlaybackWidget(nullptr)
    , m_runArchiveWidget(nullptr)
    , m_caseInventoryReportWidget(nullptr)
    , m_workspaceWidget(nullptr)
    , m_runScrollArea(nullptr)
    , m_workspaceCaseLabel(nullptr)
    , m_runSettingsRootEdit(nullptr)
    , m_initialized(false)
{
}

IDOSSimulation::~IDOSSimulation()
{
    shutdown();
}

void IDOSSimulation::initialize()
{
    if (m_window == nullptr || m_initialized)
    {
        return;
    }
    connect(m_window,
            &IDOSMainWindow::projectChanged,
            this,
&IDOSSimulation::onProjectChanged,
            Qt::UniqueConnection);
    m_simulationPanel = m_window->homeRibbonPanel(QStringLiteral("simulation"));
    m_caseViewPanel = m_window->homeRibbonPanel(QStringLiteral("caseView"));
    m_resultsPanel = m_window->homeRibbonPanel(QStringLiteral("results"));
    m_outputPanel = m_window->homeRibbonPanel(QStringLiteral("output"));
    if (m_simulationPanel == nullptr || m_caseViewPanel == nullptr || m_resultsPanel == nullptr ||
        m_outputPanel == nullptr)
    {
        throw std::runtime_error(tr("The required Home ribbon groups are unavailable.").toStdString());
    }
    {
        SARibbonPanel* viewPanel = m_caseViewPanel;
        m_caseTableAction = new QAction(tr("Table"), this);
        m_caseTableAction->setObjectName(QStringLiteral("simulationCaseTableAction"));
        m_caseTableAction->setIcon(QIcon(QStringLiteral(":/images/gui-case-views.svg")));
        m_caseTableAction->setToolTip(tr("Show a structured overview of the selected simulation case."));
        viewPanel->addSmallAction(m_caseTableAction);
        connect(m_caseTableAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onCaseTableTriggered);
        m_propertyCurvesAction = new QAction(tr("Property Curves"), this);
        m_propertyCurvesAction->setObjectName(QStringLiteral("simulationPropertyCurvesAction"));
        m_propertyCurvesAction->setIcon(QIcon(QStringLiteral(":/images/analysis-curve.svg")));
        m_propertyCurvesAction->setToolTip(tr("Plot relative permeability curves for the selected simulation case."));
        viewPanel->addSmallAction(m_propertyCurvesAction);
        connect(m_propertyCurvesAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onPropertyCurvesTriggered);
        m_wellControlsAction = new QAction(tr("Well Controls"), this);
        m_wellControlsAction->setObjectName(QStringLiteral("simulationWellControlsAction"));
        m_wellControlsAction->setIcon(QIcon(QStringLiteral(":/images/simulation-well-controls.svg")));
        m_wellControlsAction->setToolTip(tr("Show schedule well controls for the selected simulation case."));
        viewPanel->addSmallAction(m_wellControlsAction);
        connect(m_wellControlsAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onWellControlsTriggered);
        SARibbonPanel* runPanel = m_simulationPanel;
        m_runSettingsAction = new QAction(tr("Run Settings"), this);
        m_runSettingsAction->setObjectName(QStringLiteral("simulationRunSettingsAction"));
        m_runSettingsAction->setIcon(QIcon(QStringLiteral(":/images/simulation-settings.svg")));
        m_runSettingsAction->setToolTip(tr("Configure run directory rules, WSL, Flow and CPU settings."));
        runPanel->addLargeAction(m_runSettingsAction);
        connect(m_runSettingsAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onRunSettingsRequested);
        m_runStatusAction = new QAction(tr("Run"), this);
        m_runStatusAction->setObjectName(QStringLiteral("simulationRunAction"));
        m_runStatusAction->setIcon(QIcon(QStringLiteral(":/images/analysis-run.svg")));
        m_runStatusAction->setToolTip(tr("Check the selected case, prepare a run directory and start Flow."));
        runPanel->addLargeAction(m_runStatusAction);
        connect(m_runStatusAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onRunRequested);
        m_stopTaskAction = new QAction(tr("Stop Task"), this);
        m_stopTaskAction->setIcon(QIcon(QStringLiteral(":/images/simulation-stop.svg")));
        m_stopTaskAction->setObjectName(QStringLiteral("simulationStopTaskAction"));
        m_stopTaskAction->setToolTip(tr("Stop the Flow task in the run window."));
        m_stopTaskAction->setEnabled(false);
        runPanel->addLargeAction(m_stopTaskAction);
        connect(m_stopTaskAction, &QAction::triggered, this, &
IDOSSimulation::onStopTaskTriggered);
        SARibbonPanel* resultsPanel = m_resultsPanel;
        m_resultPlaybackWidget = new IDOSSimulationResultPlaybackWidget(m_resultsPanel);
        QAction* playbackAction = resultsPanel->addLargeWidget(m_resultPlaybackWidget);
        playbackAction->setParent(this);
        SARibbonPanel* analysisPanel = m_resultsPanel;
        m_curveComparisonAction = new QAction(tr("Curve Comparison"), this);
        m_curveComparisonAction->setObjectName(QStringLiteral("simulationCurveComparisonAction"));
        m_curveComparisonAction->setIcon(QIcon(QStringLiteral(":/images/simulation-curve-comparison.svg")));
        m_curveComparisonAction->setToolTip(tr("Compare the same curve across tables in the selected case."));
        analysisPanel->addSmallAction(m_curveComparisonAction);
        connect(m_curveComparisonAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onCurveComparisonTriggered);
        QAction* fitEvaluationAction = new QAction(tr("Fit Evaluation"), this);
        fitEvaluationAction->setIcon(QIcon(QStringLiteral(":/images/simulation-fit-evaluation.svg")));
        fitEvaluationAction->setEnabled(false);
        fitEvaluationAction->setToolTip(tr("Planned action; not implemented yet."));
        analysisPanel->addSmallAction(fitEvaluationAction);
        SARibbonPanel* workflowPanel = m_simulationPanel;
        m_workflowAction = new QAction(tr("Simulation Run"), this);
        m_workflowAction->setObjectName(QStringLiteral("simulationRunWorkspaceAction"));
        m_workflowAction->setIcon(QIcon(QStringLiteral(":/images/simulation-workflow.svg")));
        m_workflowAction->setCheckable(true);
        m_workflowAction->setToolTip(
            tr("Show or hide the simulation run workspace with run status, history and result files."));
        workflowPanel->addSmallAction(m_workflowAction);
        connect(m_workflowAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onWorkflowTriggered);
        m_runArchiveAction = new QAction(tr("Run Archive"), this);
        m_runArchiveAction->setObjectName(QStringLiteral("simulationRunArchiveAction"));
        m_runArchiveAction->setIcon(QIcon(QStringLiteral(":/images/simulation-archive.svg")));
        m_runArchiveAction->setToolTip(tr("Create a separate archive snapshot of the initialized run directory."));
        m_outputPanel->addSmallAction(m_runArchiveAction);
        connect(m_runArchiveAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onRunArchiveTriggered);
        SARibbonPanel* reportPanel = m_outputPanel;
        m_caseInventoryReportAction = new QAction(tr("Case Inventory Report"), this);
        m_caseInventoryReportAction->setObjectName(QStringLiteral("simulationCaseInventoryReportAction"));
        m_caseInventoryReportAction->setIcon(QIcon(QStringLiteral(":/images/simulation-report.svg")));
        m_caseInventoryReportAction->setToolTip(tr("Generate and export an inventory report for the selected case."));
        reportPanel->addSmallAction(m_caseInventoryReportAction);
        connect(m_caseInventoryReportAction,
                &QAction::triggered,
                this,
&IDOSSimulation::onCaseInventoryReportTriggered);
    }
    m_workspaceWidget = new QWidget(m_window);
    m_workspaceWidget->setObjectName(QStringLiteral("simulationWorkspace"));
    m_workspaceWidget->setStyleSheet(QStringLiteral(
        "QWidget#simulationWorkspace { background: #f3f6fa; }"));
    QVBoxLayout* workspaceLayout = new QVBoxLayout(m_workspaceWidget);
    workspaceLayout->setContentsMargins(10, 10, 10, 10);
    workspaceLayout->setSpacing(8);
    QHBoxLayout* workspaceHeader = new QHBoxLayout();
    QLabel* workspaceIcon = new QLabel(m_workspaceWidget);
    workspaceIcon->setPixmap(QIcon(QStringLiteral(":/images/simulation-workflow.svg")).pixmap(24, 24));
    workspaceHeader->addWidget(workspaceIcon);
    QVBoxLayout* workspaceTitleLayout = new QVBoxLayout();
    QLabel* workspaceTitle = new QLabel(tr("Simulation Run"), m_workspaceWidget);
    workspaceTitle->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 600; color: #26374a;"));
    m_workspaceCaseLabel = new QLabel(m_workspaceWidget);
    m_workspaceCaseLabel->setStyleSheet(QStringLiteral("color: #607286;"));
    workspaceTitleLayout->addWidget(workspaceTitle);
    workspaceTitleLayout->addWidget(m_workspaceCaseLabel);
    workspaceHeader->addLayout(workspaceTitleLayout);
    workspaceHeader->addStretch(1);
    workspaceLayout->addLayout(workspaceHeader);
    m_runScrollArea = new QScrollArea(m_workspaceWidget);
    m_runScrollArea->setObjectName(QStringLiteral("simulationRunWorkspacePage"));
    m_runScrollArea->setWidgetResizable(true);
    m_runScrollArea->setFrameShape(QFrame::NoFrame);
    workspaceLayout->addWidget(m_runScrollArea, 1);
    rebuildWorkspace(currentSimulationCase(m_window->project()));
    m_window->setSimulationWorkspaceWidget(m_workspaceWidget);
    connect(m_window,
            &IDOSMainWindow::simulationWorkspaceVisibilityChanged,
            m_workflowAction,
            &QAction::setChecked,
            Qt::UniqueConnection);
    m_workflowAction->setChecked(m_window->simulationWorkspaceVisible());
    m_initialized = true;
    onProjectChanged(m_window->project());
}

void IDOSSimulation::shutdown()
{
    if (m_window != nullptr)
    {
        disconnect(m_window, &IDOSMainWindow::projectChanged, this, &
IDOSSimulation::onProjectChanged);
    }
    if (!m_caseTableWidget.isNull())
    {
        delete m_caseTableWidget.data();
        m_caseTableWidget = nullptr;
    }
    if (!m_propertyCurvesWidget.isNull())
    {
        delete m_propertyCurvesWidget.data();
        m_propertyCurvesWidget = nullptr;
    }
    if (!m_curveComparisonWidget.isNull())
    {
        delete m_curveComparisonWidget.data();
        m_curveComparisonWidget = nullptr;
    }
    if (!m_wellControlsWidget.isNull())
    {
        delete m_wellControlsWidget.data();
        m_wellControlsWidget = nullptr;
    }
    if (!m_runPreparer.isNull())
    {
        delete m_runPreparer.data();
        m_runPreparer = nullptr;
    }
    if (!m_runStatusWidget.isNull())
    {
        delete m_runStatusWidget.data();
        m_runStatusWidget = nullptr;
    }
    if (!m_resultFilesWidget.isNull())
    {
        delete m_resultFilesWidget.data();
        m_resultFilesWidget = nullptr;
    }
    if (!m_resultPlaybackWidget.isNull())
    {
        m_resultPlaybackWidget->setCurrentObject(nullptr);
    }
    if (!m_runArchiveWidget.isNull())
    {
        delete m_runArchiveWidget.data();
        m_runArchiveWidget = nullptr;
    }
    if (!m_caseInventoryReportWidget.isNull())
    {
        delete m_caseInventoryReportWidget.data();
        m_caseInventoryReportWidget = nullptr;
    }
    if (!m_generateDataWidget.isNull())
    {
        delete m_generateDataWidget.data();
        m_generateDataWidget = nullptr;
    }
    delete m_workspaceWidget;
    m_workspaceWidget = nullptr;
    m_runScrollArea = nullptr;
    m_workspaceCaseLabel = nullptr;
    removeHomeActions();
    if (!m_resultPlaybackWidget.isNull())
    {
        delete m_resultPlaybackWidget.data();
    }
    m_simulationPanel = nullptr;
    m_caseViewPanel = nullptr;
    m_resultsPanel = nullptr;
    m_outputPanel = nullptr;
    m_caseTableAction = nullptr;
    m_propertyCurvesAction = nullptr;
    m_curveComparisonAction = nullptr;
    m_wellControlsAction = nullptr;
    m_runStatusAction = nullptr;
    m_runSettingsAction = nullptr;
    m_runArchiveAction = nullptr;
    m_caseInventoryReportAction = nullptr;
    m_workflowAction = nullptr;
    m_caseTableWidget = nullptr;
    m_generateDataWidget = nullptr;
    m_propertyCurvesWidget = nullptr;
    m_curveComparisonWidget = nullptr;
    m_wellControlsWidget = nullptr;
    m_runPreparer = nullptr;
    m_runStatusWidget = nullptr;
    m_resultFilesWidget = nullptr;
    m_resultPlaybackWidget = nullptr;
    m_runArchiveWidget = nullptr;
    m_caseInventoryReportWidget = nullptr;
    if (m_project != nullptr)
    {
        disconnect(m_project, nullptr, this, nullptr);
        m_project = nullptr;
    }
    m_initialized = false;
}

void IDOSSimulation::removeHomeActions()
{
    const QList<QAction*> actions = findChildren<QAction*>(QString(), Qt::FindDirectChildrenOnly);
    for (QAction* action : actions)
    {
        QList<QPointer<QWidget>> removedWidgets;
        const QList<QWidget*> widgets = action->associatedWidgets();
        for (QWidget* widget : widgets)
        {
            SARibbonPanel* panel = qobject_cast<SARibbonPanel*>(widget);
            if (panel != nullptr)
            {
                SARibbonPanelItem* item = panel->panelLayout()->panelItem(action);
                if (item != nullptr)
                {
                    removedWidgets.append(QPointer<QWidget>(item->widget()));
                }
                panel->removeAction(action);
            }
        }
        for (const QPointer<QWidget>& widget : removedWidgets)
        {
            if (!widget.isNull())
            {
                // Finish Ribbon's deferred cleanup before the shared panels are destroyed.
                QCoreApplication::sendPostedEvents(widget.data(), QEvent::DeferredDelete);
            }
        }
        delete action;
    }
}

void IDOSSimulation::onProjectChanged(IDOSProject* project)
{
    if (m_runStatusWidget != nullptr)
    {
        m_runStatusWidget->cancel();
    }
    if (!m_generateDataWidget.isNull())
    {
        delete m_generateDataWidget.data();
        m_generateDataWidget = nullptr;
    }
    if (m_project != nullptr)
    {
        disconnect(m_project, nullptr, this, nullptr);
    }
    m_project = project;
    if (m_project != nullptr)
    {
        connect(m_project,
                &IDOSProject::currentObjectChanged,
                this,
&IDOSSimulation::onCurrentObjectChanged);
        connect(m_project,
                &IDOSProject::currentCaseChanged,
                this,
&IDOSSimulation::onCurrentCaseChanged);
        connect(m_project,
                &IDOSProject::objectDataChanged,
                this,
&IDOSSimulation::onProjectDataChanged);
        connect(m_project, &IDOSProject::objectsDataChanged,
                this, &
IDOSSimulation::onProjectDataChanged);
    }
    rebuildWorkspace(currentSimulationCase(m_project));
    if (!m_resultPlaybackWidget.isNull())
    {
        m_resultPlaybackWidget->setCurrentObject(m_project != nullptr ? m_project->currentObject() : nullptr);
    }
    updateActionAvailability();
}

void IDOSSimulation::onCurrentObjectChanged(IDOSDataObject* object)
{
    if (!m_resultPlaybackWidget.isNull())
    {
        m_resultPlaybackWidget->setCurrentObject(object);
    }
    updateActionAvailability();
}

void IDOSSimulation::onCurrentCaseChanged(const QString& caseId)
{
    Q_UNUSED(caseId)
    if (!m_generateDataWidget.isNull())
    {
        delete m_generateDataWidget.data();
        m_generateDataWidget = nullptr;
    }
    rebuildWorkspace(currentSimulationCase(m_project));
    onCurrentObjectChanged(m_project != nullptr ? m_project->currentObject() : nullptr);
}

void IDOSSimulation::onProjectDataChanged()
{
    updateActionAvailability();
    if (m_workspaceWidget != nullptr)
    {
        QTimer::singleShot(0, this, &
IDOSSimulation::refreshWorkspace);
    }
}

void IDOSSimulation::refreshWorkspace()
{
    rebuildWorkspace(currentSimulationCase(m_project));
}

void IDOSSimulation::updateActionAvailability()
{
    const bool hasSimulationCase = currentSimulationCase(m_project) != nullptr;
    if (m_caseTableAction != nullptr)
    {
        m_caseTableAction->setEnabled(hasSimulationCase);
    }
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(m_project);
    if (m_propertyCurvesAction != nullptr)
    {
        m_propertyCurvesAction->setEnabled(simulationCase != nullptr && !simulationCase->curveTables().isEmpty());
    }
    if (m_curveComparisonAction != nullptr)
    {
        m_curveComparisonAction->setEnabled(simulationCase != nullptr && !simulationCase->curveTables().isEmpty());
    }
    if (m_wellControlsAction != nullptr)
    {
        m_wellControlsAction->setEnabled(simulationCase != nullptr && !simulationCase->wellControlRows().isEmpty());
    }
    if (m_runStatusAction != nullptr)
    {
        m_runStatusAction->setEnabled(hasSimulationCase &&
                                      (m_runStatusWidget.isNull() || !m_runStatusWidget->isRunning()));
    }
    if (m_runSettingsAction != nullptr)
    {
        m_runSettingsAction->setEnabled(m_runStatusWidget.isNull() || !m_runStatusWidget->isRunning());
    }
    if (m_stopTaskAction != nullptr)
    {
        m_stopTaskAction->setEnabled(m_runStatusWidget != nullptr && m_runStatusWidget->isRunning());
    }
    if (m_runArchiveAction != nullptr)
    {
        m_runArchiveAction->setEnabled(simulationCase != nullptr && !simulationCase->runInfo().isEmpty());
    }
    if (m_caseInventoryReportAction != nullptr)
    {
        m_caseInventoryReportAction->setEnabled(hasSimulationCase);
    }
}
IDOSSimulationCaseObject* IDOSSimulation::currentSimulationCase(IDOSProject* project) const
{
    if (project == nullptr)
    {
        return nullptr;
    }
    IDOSSimulationCaseObject* simulationCase =
        qobject_cast<IDOSSimulationCaseObject*>(project->object(project->currentCaseId()));
    return simulationCase != nullptr
               ? simulationCase
               : qobject_cast<IDOSSimulationCaseObject*>(project->currentObject());
}

void IDOSSimulation::onCaseTableTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }
    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (simulationCase->parsedSourceData().value(QStringLiteral("summary")).toMap().isEmpty())
    {
        QMessageBox::information(
            parent,
            tr("Table"),
            tr("This case has no table summary. Import the case from a DATA file first."));
        return;
    }
    if (!m_caseTableWidget.isNull())
    {
        delete m_caseTableWidget.data();
        m_caseTableWidget = nullptr;
    }
    m_caseTableWidget = new IDOSSimulationCaseTableWidget(simulationCase, parent);
    m_caseTableWidget->show();
}

void IDOSSimulation::onGenerateInputRequested(IDOSProject* project, const QString& caseId)
{
    IDOSSimulationCaseObject* simulationCase = project != nullptr
        ? qobject_cast<IDOSSimulationCaseObject*>(project->object(caseId)) : nullptr;
    if (simulationCase == nullptr)
    {
        return;
    }
    if (!m_generateDataWidget.isNull())
    {
        delete m_generateDataWidget.data();
        m_generateDataWidget = nullptr;
    }
    QDialog dialog(m_window);
    dialog.setWindowTitle(tr("Generate Simulation Input"));
    dialog.resize(900, 700);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    m_generateDataWidget = new IDOSSimulationDataGenerationWidget(simulationCase, &dialog);
    layout->addWidget(m_generateDataWidget.data());
    dialog.exec();
    m_generateDataWidget = nullptr;
}

void IDOSSimulation::onPropertyCurvesTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase != nullptr)
    {
        showPropertyCurves(project, simulationCase->objectId());
    }
}

void IDOSSimulation::onPropertyCurvesRequested(IDOSProject* project, const QString& caseId)
{
    showPropertyCurves(project, caseId);
}

void IDOSSimulation::showPropertyCurves(IDOSProject* project, const QString& caseId)
{
    IDOSSimulationCaseObject* simulationCase = project != nullptr
        ? qobject_cast<IDOSSimulationCaseObject*>(project->object(caseId)) : nullptr;
    if (simulationCase == nullptr || simulationCase->curveTables().isEmpty())
    {
        return;
    }
    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_propertyCurvesWidget.isNull())
    {
        delete m_propertyCurvesWidget.data();
        m_propertyCurvesWidget = nullptr;
    }
    m_propertyCurvesWidget = new IDOSSimulationPropertyCurveWidget(simulationCase, parent);
    m_propertyCurvesWidget->show();
}

void IDOSSimulation::onCurveComparisonTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr || simulationCase->curveTables().isEmpty())
    {
        return;
    }
    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_curveComparisonWidget.isNull())
    {
        delete m_curveComparisonWidget.data();
        m_curveComparisonWidget = nullptr;
    }
    m_curveComparisonWidget = new IDOSSimulationCurveComparisonWidget(simulationCase, parent);
    m_curveComparisonWidget->show();
}

void IDOSSimulation::onWellControlsTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr || simulationCase->wellControlRows().isEmpty())
    {
        return;
    }
    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_wellControlsWidget.isNull())
    {
        delete m_wellControlsWidget.data();
        m_wellControlsWidget = nullptr;
    }
    m_wellControlsWidget = new IDOSSimulationWellControlsWidget(simulationCase, parent);
    m_wellControlsWidget->show();
}

void IDOSSimulation::onRunRequested()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr || m_window == nullptr)
    {
        return;
    }
    if (m_runStatusWidget.isNull() ||
        (!m_runStatusWidget->isRunning() && !m_runStatusWidget->isForCase(simulationCase->objectId())))
    {
        if (!m_runStatusWidget.isNull())
        {
            delete m_runStatusWidget.data();
        }
        rebuildWorkspace(simulationCase);
    }
    if (m_runStatusWidget.isNull())
    {
        return;
    }
    m_window->showSimulationWorkspace();
    if (m_runStatusWidget->isRunning())
    {
        return;
    }
    IDOSSimulationPreflightWidget preflight(simulationCase);
    if (preflight.hasBlockingIssues())
    {
        m_runStatusWidget->reportRunRequestFailure(
            tr("Input checks found blocking issues. Review the Output panel for details."));
        return;
    }
    QTemporaryDir generatedDataDirectory;
    if (!generatedDataDirectory.isValid())
    {
        m_runStatusWidget->reportRunRequestFailure(tr("Unable to create a temporary DATA directory."));
        return;
    }
    const QFileInfo sourceDataInfo(simulationCase->sourceFile());
    QString generatedDataFileName = sourceDataInfo.completeBaseName();
    if (generatedDataFileName.isEmpty())
    {
        generatedDataFileName = simulationCase->name();
    }
    generatedDataFileName.append(QStringLiteral("_generated."));
    generatedDataFileName.append(sourceDataInfo.suffix().isEmpty()
                                     ? QStringLiteral("DATA")
                                     : sourceDataInfo.suffix());
    QString dataGenerationError;
    IDOSSimulationCaseEclipseProvider dataProvider;
    if (!dataProvider.writeMemoryData(simulationCase,
                                      project,
                                      generatedDataDirectory.path(),
                                      generatedDataFileName,
                                      &dataGenerationError))
    {
        IDOS_MESSAGE(tr("DATA generation failed: %1").arg(dataGenerationError), IDOSLogLevel::Error);
        m_runStatusWidget->reportRunRequestFailure(tr("DATA generation failed. Review the Output panel for details."));
        return;
    }
    const QString generatedDataFile = QDir(generatedDataDirectory.path()).absoluteFilePath(generatedDataFileName);
    IDOS_MESSAGE(tr("Generated temporary DATA file: %1").arg(generatedDataFile), IDOSLogLevel::Info);
    if (m_runPreparer.isNull())
    {
        m_runStatusWidget->reportRunRequestFailure(tr("The run preparation service is unavailable."));
        return;
    }
    m_runPreparer->setGeneratedDataFile(generatedDataFile);
    QString runRoot = QSettings().value(QStringLiteral("simulation/runDirectoryRoot")).toString().trimmed();
    if (runRoot.isEmpty())
    {
        runRoot = QDir(sourceDataInfo.absolutePath()).absoluteFilePath(QStringLiteral("idos_runs"));
    }
    QString caseDirectoryName;
    const QString caseName = simulationCase->name();
    for (int characterIndex = 0; characterIndex < caseName.size(); ++characterIndex)
    {
        const QChar character = caseName.at(characterIndex);
        if (character.isLetterOrNumber() || character == QLatin1Char('-') || character == QLatin1Char('_'))
        {
            caseDirectoryName.append(character);
        }
        else
        {
            caseDirectoryName.append(QLatin1Char('_'));
        }
    }
    if (caseDirectoryName.isEmpty())
    {
        caseDirectoryName = simulationCase->objectId();
    }
    const QString caseRunRoot = QDir(runRoot).absoluteFilePath(caseDirectoryName);
    int runNumber = 1;
    QString runDirectory;
    do
    {
        runDirectory = QDir(caseRunRoot).absoluteFilePath(
            QStringLiteral("run-%1-%2")
                .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")))
                .arg(runNumber, 3, 10, QLatin1Char('0')));
        ++runNumber;
    }
    while (QFileInfo(runDirectory).exists());
    m_runPreparer->setRunDirectory(runDirectory);
    if (!m_runPreparer->prepareRunDirectory())
    {
        m_runStatusWidget->reportRunRequestFailure(tr("Run preparation failed. Review the Output panel for details."));
        return;
    }
    QVariantMap runInfo = simulationCase->runInfo();
    const QVariantMap preparedRunInfo = m_runPreparer->preparedRunInfo();
    for (QVariantMap::const_iterator iterator = preparedRunInfo.constBegin();
         iterator != preparedRunInfo.constEnd();
         ++iterator)
    {
        runInfo.insert(iterator.key(), iterator.value());
    }
    simulationCase->setRunInfo(runInfo);
    m_runStatusWidget->startFlow();
    updateActionAvailability();
}

void IDOSSimulation::onStopTaskTriggered()
{
    if (m_runStatusWidget != nullptr)
    {
        m_runStatusWidget->stop();
        if (m_runScrollArea != nullptr && m_window != nullptr)
        {
            m_window->showSimulationWorkspace();
            m_runScrollArea->ensureWidgetVisible(m_runStatusWidget.data(), 0, 8);
        }
    }
    updateActionAvailability();
}

void IDOSSimulation::onRunArchiveTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr || simulationCase->runInfo().isEmpty())
    {
        return;
    }
    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_runArchiveWidget.isNull())
    {
        delete m_runArchiveWidget.data();
        m_runArchiveWidget = nullptr;
    }
    m_runArchiveWidget = new IDOSSimulationRunArchiveWidget(simulationCase, parent);
    m_runArchiveWidget->show();
}

void IDOSSimulation::onCaseInventoryReportTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }
    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_caseInventoryReportWidget.isNull())
    {
        delete m_caseInventoryReportWidget.data();
        m_caseInventoryReportWidget = nullptr;
    }
    m_caseInventoryReportWidget = new IDOSSimulationCaseInventoryReportWidget(simulationCase, parent);
    m_caseInventoryReportWidget->show();
}

void IDOSSimulation::rebuildWorkspace(IDOSSimulationCaseObject* simulationCase)
{
    if (m_runScrollArea == nullptr)
    {
        return;
    }
    const int scrollValue = m_runScrollArea->verticalScrollBar()->value();
    if (!m_runStatusWidget.isNull() && !m_runStatusWidget->isRunning() &&
        !m_runStatusWidget->isForCase(simulationCase != nullptr ? simulationCase->objectId() : QString()))
    {
        delete m_runStatusWidget.data();
        m_runStatusWidget = nullptr;
    }
    if (!m_runStatusWidget.isNull())
    {
        m_runStatusWidget->setParent(m_window);
    }
    if (!m_runPreparer.isNull())
    {
        delete m_runPreparer.data();
        m_runPreparer = nullptr;
    }
    QWidget* previousPage = m_runScrollArea->takeWidget();
    delete previousPage;
    if (simulationCase != nullptr)
    {
        m_runPreparer = new IDOSSimulationInitializeWidget(simulationCase, m_workspaceWidget);
    }

    QWidget* page = new QWidget(m_runScrollArea);
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(10);

    QFrame* runCard = new QFrame(page);
    runCard->setObjectName(QStringLiteral("simulationRunCard"));
    runCard->setStyleSheet(QStringLiteral(
        "QFrame#simulationRunCard, QFrame#simulationResultsCard { background: #ffffff; "
        "border: 1px solid #d8e0e9; border-radius: 5px; }"
        "QLabel#simulationRunSectionTitle { color: #26374a; font-size: 14px; font-weight: 600; }"));
    QVBoxLayout* runCardLayout = new QVBoxLayout(runCard);
    runCardLayout->setContentsMargins(14, 12, 14, 12);
    runCardLayout->setSpacing(8);
    QHBoxLayout* runHeadingLayout = new QHBoxLayout();
    QLabel* runIcon = new QLabel(runCard);
    runIcon->setPixmap(QIcon(QStringLiteral(":/images/analysis-run.svg")).pixmap(20, 20));
    runHeadingLayout->addWidget(runIcon);
    QLabel* runTitle = new QLabel(tr("Run History and Status"), runCard);
    runTitle->setObjectName(QStringLiteral("simulationRunSectionTitle"));
    runHeadingLayout->addWidget(runTitle);
    runHeadingLayout->addStretch(1);
    runCardLayout->addLayout(runHeadingLayout);
    if (m_runStatusWidget.isNull())
    {
        m_runStatusWidget = new IDOSSimulationRunStatusWidget(simulationCase, m_window);
        connect(m_runStatusWidget,
                &IDOSSimulationRunStatusWidget::runningChanged,
                this,
                &IDOSSimulation::updateActionAvailability);
        connect(m_runStatusWidget,
                &IDOSSimulationRunStatusWidget::runFinished,
                this,
                &IDOSSimulation::onRunFinished);
        connect(m_runStatusWidget,
                &IDOSSimulationRunStatusWidget::historyEntryActivated,
                this,
                &IDOSSimulation::onRunHistorySelected);
    }
    m_runStatusWidget->setParent(runCard);
    runCardLayout->addWidget(m_runStatusWidget.data());
    pageLayout->addWidget(runCard);

    QFrame* resultsCard = new QFrame(page);
    resultsCard->setObjectName(QStringLiteral("simulationResultsCard"));
    QVBoxLayout* resultsCardLayout = new QVBoxLayout(resultsCard);
    resultsCardLayout->setContentsMargins(14, 12, 14, 12);
    resultsCardLayout->setSpacing(8);
    QHBoxLayout* resultsHeadingLayout = new QHBoxLayout();
    QLabel* resultsIcon = new QLabel(resultsCard);
    resultsIcon->setPixmap(QIcon(QStringLiteral(":/images/gui-case-results.svg")).pixmap(20, 20));
    resultsHeadingLayout->addWidget(resultsIcon);
    QLabel* resultsTitle = new QLabel(tr("Run Output Files"), resultsCard);
    resultsTitle->setObjectName(QStringLiteral("simulationRunSectionTitle"));
    resultsHeadingLayout->addWidget(resultsTitle);
    resultsHeadingLayout->addStretch(1);
    resultsCardLayout->addLayout(resultsHeadingLayout);
    m_resultFilesWidget = new IDOSSimulationResultFilesWidget(simulationCase, resultsCard);
    resultsCardLayout->addWidget(m_resultFilesWidget.data());
    pageLayout->addWidget(resultsCard);
    pageLayout->addStretch(1);

    m_runScrollArea->setWidget(page);
    m_workspaceCaseLabel->setText(simulationCase != nullptr
                                      ? simulationCase->name()
                                      : tr("No simulation case selected"));
    m_runScrollArea->verticalScrollBar()->setValue(scrollValue);
}
void IDOSSimulation::onWorkflowTriggered()
{
    if (m_window == nullptr || m_workflowAction == nullptr)
    {
        return;
    }
    m_window->setSimulationWorkspaceVisible(m_workflowAction->isChecked());
}

void IDOSSimulation::onRunSettingsRequested()
{
    if (m_window == nullptr)
    {
        return;
    }
    if (!m_runStatusWidget.isNull() && m_runStatusWidget->isRunning())
    {
        QMessageBox::information(m_window,
                                 tr("Run Settings"),
                                 tr("Run settings cannot be changed while Flow is running."));
        return;
    }
    QSettings settings;
    QDialog dialog(m_window);
    dialog.setWindowTitle(tr("Run Settings"));
    dialog.setModal(true);
    dialog.setMinimumWidth(460);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* description = new QLabel(
        tr("Set a shared parent folder for run directories and configure Flow execution. "
           "Leave the folder empty to create runs beside the case input files."),
        &dialog);
    description->setWordWrap(true);
    layout->addWidget(description);
    QFormLayout* form = new QFormLayout();
    QHBoxLayout* directoryLayout = new QHBoxLayout();
    m_runSettingsRootEdit = new QLineEdit(
        settings.value(QStringLiteral("simulation/runDirectoryRoot")).toString(), &dialog);
    m_runSettingsRootEdit->setObjectName(QStringLiteral("simulationRunDirectoryRootEdit"));
    m_runSettingsRootEdit->setPlaceholderText(tr("Default: a folder beside the case input files"));
    directoryLayout->addWidget(m_runSettingsRootEdit, 1);
    QPushButton* browseDirectoryButton = new QPushButton(tr("Browse…"), &dialog);
    directoryLayout->addWidget(browseDirectoryButton);
    connect(browseDirectoryButton,
            &QPushButton::clicked,
            this,
&IDOSSimulation::onBrowseRunDirectoryRootTriggered);
    QLineEdit* distributionEdit = new QLineEdit(
        settings.value(QStringLiteral("simulation/distribution"), QStringLiteral("Ubuntu-24.04")).toString(), &dialog);
    distributionEdit->setObjectName(QStringLiteral("simulationLinuxDistributionEdit"));
    QLineEdit* executableEdit = new QLineEdit(
        settings.value(QStringLiteral("simulation/executable"), QStringLiteral("/usr/bin/flow")).toString(), &dialog);
    executableEdit->setObjectName(QStringLiteral("simulationFlowExecutableEdit"));
    QSpinBox* threadsSpin = new QSpinBox(&dialog);
    threadsSpin->setObjectName(QStringLiteral("simulationCpuThreadsSpin"));
    threadsSpin->setRange(1, 256);
    threadsSpin->setValue(settings.value(QStringLiteral("simulation/threads"), 2).toInt());
    form->addRow(tr("Run directory parent"), directoryLayout);
    form->addRow(tr("Linux distribution"), distributionEdit);
    form->addRow(tr("Flow executable"), executableEdit);
    form->addRow(tr("CPU threads"), threadsSpin);
    layout->addLayout(form);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Apply"));
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted)
    {
        m_runSettingsRootEdit = nullptr;
        return;
    }
    const QString runDirectoryRoot = m_runSettingsRootEdit->text().trimmed();
    m_runSettingsRootEdit = nullptr;
    const QString distribution = distributionEdit->text().trimmed();
    const QString executable = executableEdit->text().trimmed();
    if ((!runDirectoryRoot.isEmpty() && !QDir::isAbsolutePath(runDirectoryRoot)) ||
        distribution.isEmpty() || !executable.startsWith(QLatin1Char('/')))
    {
        QMessageBox::warning(m_window,
                             tr("Run Settings"),
                             tr("Enter an absolute run directory parent when set, a Linux distribution, "
                                "and an absolute Flow executable path."));
        return;
    }
    settings.setValue(QStringLiteral("simulation/runDirectoryRoot"), runDirectoryRoot);
    settings.setValue(QStringLiteral("simulation/distribution"), distribution);
    settings.setValue(QStringLiteral("simulation/executable"), executable);
    settings.setValue(QStringLiteral("simulation/threads"), threadsSpin->value());
    if (!m_runStatusWidget.isNull())
    {
        m_runStatusWidget->refreshSettingsSummary();
    }
}

void IDOSSimulation::onBrowseRunDirectoryRootTriggered()
{
    if (m_window == nullptr || m_runSettingsRootEdit == nullptr)
    {
        return;
    }
    const QString selectedDirectory = QFileDialog::getExistingDirectory(
        m_window,
        tr("Select run directory parent"),
        m_runSettingsRootEdit->text().trimmed().isEmpty()
            ? QDir::homePath()
            : m_runSettingsRootEdit->text().trimmed());
    if (selectedDirectory.isEmpty())
    {
        return;
    }
    m_runSettingsRootEdit->setText(QDir::cleanPath(selectedDirectory));
}

void IDOSSimulation::onRunFinished(bool succeeded)
{
    if (!succeeded || m_window == nullptr)
    {
        return;
    }
    refreshWorkspace();
    m_window->showSimulationWorkspace();
}

void IDOSSimulation::onRunHistorySelected(const QString& outputDirectory)
{
    if (m_window == nullptr || m_resultFilesWidget.isNull())
    {
        return;
    }
    m_resultFilesWidget->showRunDirectory(outputDirectory);
    m_window->showSimulationWorkspace();
}
