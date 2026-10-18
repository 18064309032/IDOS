#include <stdexcept>

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QIcon>
#include <QJsonObject>
#include <QList>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

#include <SARibbonPanel.h>
#include <SARibbonPanelItem.h>
#include <SARibbonPanelLayout.h>

#include "case/idossimulationcaseinventoryreportwidget.h"
#include "case/idossimulationcasetablewidget.h"
#include "case/idossimulationcurvecomparisonwidget.h"
#include "case/idossimulationdatagenerationwidget.h"
#include "case/idossimulationdeckbrowserwidget.h"
#include "case/idossimulationinitializewidget.h"
#include "case/idossimulationparsesummarywidget.h"
#include "case/idossimulationpreflightwidget.h"
#include "case/idossimulationpropertycurvewidget.h"
#include "case/idossimulationresultfileswidget.h"
#include "case/idossimulationresultplaybackwidget.h"
#include "case/idossimulationrunarchivewidget.h"
#include "case/idossimulationrunstatuswidget.h"
#include "case/idossimulationwellcontrolswidget.h"
#include "case/idossimulationworkflowwidget.h"
#include "data/case/idossimulationcaseobject.h"
#include "idosmainwindow.h"
#include "idosproject.h"

#include "idossimulation.h"

IDOSSimulation::IDOSSimulation(IDOSMainWindow* window)
    :
    QObject(window)
    , m_window(window)
    , m_project(nullptr)
    , m_simulationPanel(nullptr)
    , m_resultsPanel(nullptr)
    , m_outputPanel(nullptr)
    , m_parseSummaryAction(nullptr)
    , m_generateDataAction(nullptr)
    , m_deckBrowserAction(nullptr)
    , m_caseTableAction(nullptr)
    , m_propertyCurvesAction(nullptr)
    , m_curveComparisonAction(nullptr)
    , m_wellControlsAction(nullptr)
    , m_runPreflightAction(nullptr)
    , m_initializeAction(nullptr)
    , m_runStatusAction(nullptr)
    , m_stopTaskAction(nullptr)
    , m_resultFilesAction(nullptr)
    , m_runArchiveAction(nullptr)
    , m_caseInventoryReportAction(nullptr)
    , m_workflowAction(nullptr)
    , m_caseTableWidget(nullptr)
    , m_parseSummaryWidget(nullptr)
    , m_generateDataWidget(nullptr)
    , m_deckBrowserWidget(nullptr)
    , m_propertyCurvesWidget(nullptr)
    , m_curveComparisonWidget(nullptr)
    , m_wellControlsWidget(nullptr)
    , m_runPreflightWidget(nullptr)
    , m_initializeWidget(nullptr)
    , m_runStatusWidget(nullptr)
    , m_resultFilesWidget(nullptr)
    , m_resultPlaybackWidget(nullptr)
    , m_runArchiveWidget(nullptr)
    , m_caseInventoryReportWidget(nullptr)
    , m_workflowWidget(nullptr)
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
    m_resultsPanel = m_window->homeRibbonPanel(QStringLiteral("results"));
    m_outputPanel = m_window->homeRibbonPanel(QStringLiteral("output"));
    if (m_simulationPanel == nullptr || m_resultsPanel == nullptr || m_outputPanel == nullptr)
    {
        throw std::runtime_error(tr("The required Home ribbon groups are unavailable.").toStdString());
    }
    {
        SARibbonPanel* inputPanel = m_simulationPanel;
        m_deckBrowserAction = new QAction(tr("Deck Browser"), this);
        m_deckBrowserAction->setObjectName(QStringLiteral("simulationDeckBrowserAction"));
        m_deckBrowserAction->setIcon(QIcon(QStringLiteral(":/images/simulation-deck.svg")));
        m_deckBrowserAction->setToolTip(tr("Browse parsed keywords in the selected DATA file."));
        inputPanel->addSmallAction(m_deckBrowserAction);
        connect(m_deckBrowserAction,
                &QAction::triggered,
                this,
                &IDOSSimulation::onDeckBrowserTriggered);

        QAction* parseSummaryAction = new QAction(tr("Parse Summary"), this);
        m_parseSummaryAction = parseSummaryAction;
        m_parseSummaryAction->setObjectName(QStringLiteral("simulationParseSummaryAction"));
        parseSummaryAction->setIcon(QIcon(QStringLiteral(":/images/analysis-chart.svg")));
        parseSummaryAction->setToolTip(tr("Show the DATA parse summary saved when the case was imported."));
        inputPanel->addSmallAction(parseSummaryAction);
        connect(m_parseSummaryAction,
                &QAction::triggered,
                this,
                &IDOSSimulation::onParseSummaryTriggered);

        m_generateDataAction = new QAction(tr("Generate DATA"), this);
        m_generateDataAction->setObjectName(QStringLiteral("simulationGenerateDataAction"));
        m_generateDataAction->setIcon(QIcon(QStringLiteral(":/images/app-export.svg")));
        m_generateDataAction->setToolTip(tr("Edit the source DATA text and export it with indexed input files."));
        m_outputPanel->addSmallAction(m_generateDataAction);
        connect(m_generateDataAction,
                &QAction::triggered,
                this,
                &IDOSSimulation::onGenerateDataTriggered);

        SARibbonPanel* viewPanel = m_resultsPanel;
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
        m_runPreflightAction = new QAction(tr("Run Preflight"), this);
        m_runPreflightAction->setObjectName(QStringLiteral("simulationRunPreflightAction"));
        m_runPreflightAction->setIcon(QIcon(QStringLiteral(":/images/simulation-preflight.svg")));
        m_runPreflightAction->setToolTip(tr("Check the selected case before starting a simulation run."));
        runPanel->addLargeAction(m_runPreflightAction);
        connect(m_runPreflightAction,
                &QAction::triggered,
                this,
                &IDOSSimulation::onRunPreflightTriggered);

        m_initializeAction = new QAction(tr("Initialize"), this);
        m_initializeAction->setObjectName(QStringLiteral("simulationInitializeAction"));
        m_initializeAction->setIcon(QIcon(QStringLiteral(":/images/simulation-initialize.svg")));
        m_initializeAction->setToolTip(tr("Prepare and validate an isolated run directory after Run Preflight."));
        runPanel->addLargeAction(m_initializeAction);
        connect(m_initializeAction,
                &QAction::triggered,
                this,
                &IDOSSimulation::onInitializeTriggered);

        m_runStatusAction = new QAction(tr("Run Status"), this);
        m_runStatusAction->setObjectName(QStringLiteral("simulationRunStatusAction"));
        m_runStatusAction->setIcon(QIcon(QStringLiteral(":/images/analysis-run.svg")));
        m_runStatusAction->setToolTip(tr("Configure and run Flow, view live output and import results."));
        runPanel->addLargeAction(m_runStatusAction);
        connect(m_runStatusAction,
                &QAction::triggered,
                this,
                &IDOSSimulation::onRunStatusTriggered);

        m_stopTaskAction = new QAction(tr("Stop Task"), this);
        m_stopTaskAction->setIcon(QIcon(QStringLiteral(":/images/simulation-stop.svg")));
        m_stopTaskAction->setObjectName(QStringLiteral("simulationStopTaskAction"));
        m_stopTaskAction->setToolTip(tr("Stop the Flow task in the run window."));
        m_stopTaskAction->setEnabled(false);
        runPanel->addLargeAction(m_stopTaskAction);
        connect(m_stopTaskAction, &QAction::triggered, this, &IDOSSimulation::onStopTaskTriggered);

        SARibbonPanel* resultsPanel = m_resultsPanel;
        m_resultFilesAction = new QAction(tr("Result Files"), this);
        m_resultFilesAction->setObjectName(QStringLiteral("simulationResultFilesAction"));
        m_resultFilesAction->setIcon(QIcon(QStringLiteral(":/images/gui-case-results.svg")));
        m_resultFilesAction->setToolTip(tr("Browse files produced in the initialized run directory."));
        resultsPanel->addLargeAction(m_resultFilesAction);
        connect(m_resultFilesAction,
                &QAction::triggered,
                this,
                &IDOSSimulation::onResultFilesTriggered);

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
        m_workflowAction = new QAction(tr("Workflow"), this);
        m_workflowAction->setObjectName(QStringLiteral("simulationWorkflowAction"));
        m_workflowAction->setIcon(QIcon(QStringLiteral(":/images/simulation-workflow.svg")));
        m_workflowAction->setToolTip(tr("Review the preparation and result stages for the selected case."));
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
    m_initialized = true;
    onProjectChanged(m_window->project());
}

void IDOSSimulation::shutdown()
{
    if (m_window != nullptr)
    {
        disconnect(m_window, &IDOSMainWindow::projectChanged, this, &IDOSSimulation::onProjectChanged);
    }
    if (!m_parseSummaryWidget.isNull())
    {
        delete m_parseSummaryWidget.data();
        m_parseSummaryWidget = nullptr;
    }
    if (!m_generateDataWidget.isNull())
    {
        delete m_generateDataWidget.data();
        m_generateDataWidget = nullptr;
    }
    if (!m_caseTableWidget.isNull())
    {
        delete m_caseTableWidget.data();
        m_caseTableWidget = nullptr;
    }
    if (!m_deckBrowserWidget.isNull())
    {
        delete m_deckBrowserWidget.data();
        m_deckBrowserWidget = nullptr;
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
    if (!m_runPreflightWidget.isNull())
    {
        delete m_runPreflightWidget.data();
        m_runPreflightWidget = nullptr;
    }
    if (!m_initializeWidget.isNull())
    {
        delete m_initializeWidget.data();
        m_initializeWidget = nullptr;
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
    if (!m_workflowWidget.isNull())
    {
        delete m_workflowWidget.data();
        m_workflowWidget = nullptr;
    }
    removeHomeActions();
    if (!m_resultPlaybackWidget.isNull())
    {
        delete m_resultPlaybackWidget.data();
    }
    m_simulationPanel = nullptr;
    m_resultsPanel = nullptr;
    m_outputPanel = nullptr;
    m_parseSummaryAction = nullptr;
    m_generateDataAction = nullptr;
    m_deckBrowserAction = nullptr;
    m_caseTableAction = nullptr;
    m_propertyCurvesAction = nullptr;
    m_curveComparisonAction = nullptr;
    m_wellControlsAction = nullptr;
    m_runPreflightAction = nullptr;
    m_initializeAction = nullptr;
    m_runStatusAction = nullptr;
    m_resultFilesAction = nullptr;
    m_runArchiveAction = nullptr;
    m_caseInventoryReportAction = nullptr;
    m_workflowAction = nullptr;
    m_caseTableWidget = nullptr;
    m_parseSummaryWidget = nullptr;
    m_generateDataWidget = nullptr;
    m_deckBrowserWidget = nullptr;
    m_propertyCurvesWidget = nullptr;
    m_curveComparisonWidget = nullptr;
    m_wellControlsWidget = nullptr;
    m_runPreflightWidget = nullptr;
    m_initializeWidget = nullptr;
    m_runStatusWidget = nullptr;
    m_resultFilesWidget = nullptr;
    m_resultPlaybackWidget = nullptr;
    m_runArchiveWidget = nullptr;
    m_caseInventoryReportWidget = nullptr;
    m_workflowWidget = nullptr;
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
                this, &IDOSSimulation::onProjectDataChanged);
    }
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
    onCurrentObjectChanged(m_project != nullptr ? m_project->currentObject() : nullptr);
}

void IDOSSimulation::onProjectDataChanged()
{
    updateActionAvailability();
}

void IDOSSimulation::updateActionAvailability()
{
    const bool hasSimulationCase = currentSimulationCase(m_project) != nullptr;
    if (m_parseSummaryAction != nullptr)
    {
        m_parseSummaryAction->setEnabled(hasSimulationCase);
    }
    if (m_generateDataAction != nullptr)
    {
        m_generateDataAction->setEnabled(hasSimulationCase);
    }
    if (m_deckBrowserAction != nullptr)
    {
        m_deckBrowserAction->setEnabled(hasSimulationCase);
    }
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
    if (m_runPreflightAction != nullptr)
    {
        m_runPreflightAction->setEnabled(hasSimulationCase);
    }
    if (m_initializeAction != nullptr)
    {
        m_initializeAction->setEnabled(hasSimulationCase);
    }
    if (m_runStatusAction != nullptr)
    {
        m_runStatusAction->setEnabled(hasSimulationCase);
    }
    if (m_stopTaskAction != nullptr)
    {
        m_stopTaskAction->setEnabled(m_runStatusWidget != nullptr && m_runStatusWidget->isRunning());
    }
    if (m_resultFilesAction != nullptr)
    {
        m_resultFilesAction->setEnabled(hasSimulationCase);
    }
    if (m_runArchiveAction != nullptr)
    {
        m_runArchiveAction->setEnabled(simulationCase != nullptr && !simulationCase->runInfo().isEmpty());
    }
    if (m_caseInventoryReportAction != nullptr)
    {
        m_caseInventoryReportAction->setEnabled(hasSimulationCase);
    }
    if (m_workflowAction != nullptr)
    {
        m_workflowAction->setEnabled(hasSimulationCase);
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

void IDOSSimulation::onParseSummaryTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }
    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    const QVariantMap summary = simulationCase->parsedSourceData().value(QStringLiteral("summary")).toMap();
    if (summary.isEmpty())
    {
        QMessageBox::information(
            parent,
            tr("Parse Summary"),
            tr("This case has no DATA parse summary. Import the case from a DATA file first."));
        return;
    }

    if (!m_parseSummaryWidget.isNull())
    {
        delete m_parseSummaryWidget.data();
        m_parseSummaryWidget = nullptr;
    }

    m_parseSummaryWidget = new IDOSSimulationParseSummaryWidget(simulationCase, parent);
    m_parseSummaryWidget->show();
}

void IDOSSimulation::onGenerateDataTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }

    onGenerateInputRequested(project, simulationCase->objectId());
}

void IDOSSimulation::onGenerateInputRequested(IDOSProject* project, const QString& caseId)
{
    IDOSSimulationCaseObject* simulationCase = project != nullptr
        ? qobject_cast<IDOSSimulationCaseObject*>(project->object(caseId)) : nullptr;
    if (simulationCase == nullptr)
    {
        return;
    }

    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_generateDataWidget.isNull())
    {
        delete m_generateDataWidget.data();
        m_generateDataWidget = nullptr;
    }

    m_generateDataWidget = new IDOSSimulationDataGenerationWidget(simulationCase, parent);
    m_generateDataWidget->show();
}

void IDOSSimulation::onDeckBrowserTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }
    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (simulationCase->sourceFile().isEmpty())
    {
        QMessageBox::information(parent, tr("Deck Browser"), tr("This case has no source DATA file."));
        return;
    }

    if (simulationCase->parsedSourceData().value(QStringLiteral("keywordIndex")).toList().isEmpty())
    {
        QMessageBox::information(parent, tr("Deck Browser"), tr("No keywords were found in the DATA file."));
        return;
    }

    if (!m_deckBrowserWidget.isNull())
    {
        delete m_deckBrowserWidget.data();
        m_deckBrowserWidget = nullptr;
    }

    m_deckBrowserWidget = new IDOSSimulationDeckBrowserWidget(simulationCase, parent);
    m_deckBrowserWidget->show();
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

void IDOSSimulation::onRunPreflightTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }

    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_runPreflightWidget.isNull())
    {
        delete m_runPreflightWidget.data();
        m_runPreflightWidget = nullptr;
    }

    m_runPreflightWidget = new IDOSSimulationPreflightWidget(simulationCase, parent);
    m_runPreflightWidget->show();
}

void IDOSSimulation::onInitializeTriggered()
{
    if (m_runStatusWidget != nullptr && m_runStatusWidget->isRunning())
    {
        m_runStatusWidget->show();
        return;
    }
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }

    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    IDOSSimulationPreflightWidget preflight(simulationCase, parent);
    if (preflight.hasBlockingIssues())
    {
        if (!m_runPreflightWidget.isNull())
        {
            delete m_runPreflightWidget.data();
            m_runPreflightWidget = nullptr;
        }
        onRunPreflightTriggered();
        QMessageBox::warning(parent,
                             tr("Initialize"),
                             tr("Initialization was stopped because Run Preflight found blocking issues."));
        return;
    }

    if (!m_initializeWidget.isNull())
    {
        delete m_initializeWidget.data();
        m_initializeWidget = nullptr;
    }

    m_initializeWidget = new IDOSSimulationInitializeWidget(simulationCase, parent);
    m_initializeWidget->show();
}

void IDOSSimulation::onRunStatusTriggered()
{
    if (m_runStatusWidget != nullptr && m_runStatusWidget->isRunning())
    {
        m_runStatusWidget->show();
        m_runStatusWidget->raise();
        return;
    }
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }

    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_runStatusWidget.isNull())
    {
        delete m_runStatusWidget.data();
        m_runStatusWidget = nullptr;
    }

    m_runStatusWidget = new IDOSSimulationRunStatusWidget(simulationCase, parent);
    connect(m_runStatusWidget,
            &IDOSSimulationRunStatusWidget::runningChanged,
            this,
            &IDOSSimulation::updateActionAvailability);
    m_runStatusWidget->show();
    updateActionAvailability();
}

void IDOSSimulation::onStopTaskTriggered()
{
    if (m_runStatusWidget != nullptr)
    {
        m_runStatusWidget->stop();
        m_runStatusWidget->show();
    }
    updateActionAvailability();
}

void IDOSSimulation::onResultFilesTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }

    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_resultFilesWidget.isNull())
    {
        delete m_resultFilesWidget.data();
        m_resultFilesWidget = nullptr;
    }

    m_resultFilesWidget = new IDOSSimulationResultFilesWidget(simulationCase, parent);
    m_resultFilesWidget->show();
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

void IDOSSimulation::onWorkflowTriggered()
{
    IDOSProject* project = m_window != nullptr ? m_window->project() : nullptr;
    IDOSSimulationCaseObject* simulationCase = currentSimulationCase(project);
    if (simulationCase == nullptr)
    {
        return;
    }

    QWidget* parent = m_window != nullptr ? m_window : nullptr;
    if (!m_workflowWidget.isNull())
    {
        delete m_workflowWidget.data();
        m_workflowWidget = nullptr;
    }

    m_workflowWidget = new IDOSSimulationWorkflowWidget(simulationCase, parent);
    m_workflowWidget->show();
}
