#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QLocale>
#include <QObject>
#include <QString>
#include <QTranslator>

#include <SARibbonCategory.h>
#include <SARibbonPanel.h>

#include "plugin/idosinterface.h"

#include "idossimulationplugin.h"

static const QString sName = QObject::tr("Numerical Simulation");
static const QString sDescription = QObject::tr("Reservoir simulation plugin");
static const QString sCategory = QObject::tr("Reservoir Engineering");
static const QString sPluginVersion = QStringLiteral("0.1.0");
static const IDOSPlugin::PluginType sPluginType = IDOSPlugin::UI;
static const QString sPluginIcon = QStringLiteral(":/images/gui-case.svg");

IDOSSimulationPlugin::IDOSSimulationPlugin(IDOSInterface* interface)
    :
    QObject(nullptr)
    , IDOSPlugin(sName, sDescription, sCategory, sPluginVersion, sPluginType)
    , m_interface(interface)
    , m_category(nullptr)
    , m_translator(nullptr)
    , m_initialized(false)
{
}

IDOSSimulationPlugin::~IDOSSimulationPlugin()
{
    uninstallTranslator();
}

void IDOSSimulationPlugin::uninstallTranslator()
{
    if (m_translator == nullptr)
    {
        return;
    }

    qApp->removeTranslator(m_translator);
    delete m_translator;
    m_translator = nullptr;
}

void IDOSSimulationPlugin::initGui()
{
    if (m_interface == nullptr || m_initialized)
    {
        return;
    }

    m_translator = new QTranslator(this);
    const QString translationPath = qApp->applicationDirPath()
        + QStringLiteral("/i18n/idossimulationplugin_") + QLocale().name()
        + QStringLiteral(".qm");
    if (!m_translator->load(translationPath) || !qApp->installTranslator(m_translator))
    {
        delete m_translator;
        m_translator = nullptr;
    }

    m_category = m_interface->addRibbonCategory(
        QStringLiteral("idosSimulationCategory"),
        tr("Numerical Simulation"));
    if (m_category != nullptr)
    {
        SARibbonPanel* casePanel = m_category->addPanel(tr("Case"));
        QAction* newCaseAction = new QAction(tr("New Case"), m_category);
        newCaseAction->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
        newCaseAction->setEnabled(false);
        newCaseAction->setToolTip(tr("Planned action; not implemented yet."));
        casePanel->addLargeAction(newCaseAction);

        QAction* importDataAction = new QAction(tr("Import DATA"), m_category);
        importDataAction->setIcon(QIcon(QStringLiteral(":/images/app-import.svg")));
        importDataAction->setEnabled(false);
        importDataAction->setToolTip(tr("Planned action; not implemented yet."));
        casePanel->addLargeAction(importDataAction);

        SARibbonPanel* inputPanel = m_category->addPanel(tr("Input"));
        QAction* deckBrowserAction = new QAction(tr("Deck Browser"), m_category);
        deckBrowserAction->setIcon(QIcon(QStringLiteral(":/images/simulation-deck.svg")));
        deckBrowserAction->setEnabled(false);
        deckBrowserAction->setToolTip(tr("Planned action; not implemented yet."));
        inputPanel->addLargeAction(deckBrowserAction);

        QAction* parseSummaryAction = new QAction(tr("Parse Summary"), m_category);
        parseSummaryAction->setIcon(QIcon(QStringLiteral(":/images/analysis-chart.svg")));
        parseSummaryAction->setEnabled(false);
        parseSummaryAction->setToolTip(tr("Planned action; not implemented yet."));
        inputPanel->addLargeAction(parseSummaryAction);

        QAction* generateDataAction = new QAction(tr("Generate DATA"), m_category);
        generateDataAction->setIcon(QIcon(QStringLiteral(":/images/app-export.svg")));
        generateDataAction->setEnabled(false);
        generateDataAction->setToolTip(tr("Planned action; not implemented yet."));
        inputPanel->addLargeAction(generateDataAction);

        SARibbonPanel* viewPanel = m_category->addPanel(tr("View"));
        QAction* tableAction = new QAction(tr("Table"), m_category);
        tableAction->setIcon(QIcon(QStringLiteral(":/images/gui-case-views.svg")));
        tableAction->setEnabled(false);
        tableAction->setToolTip(tr("Planned action; not implemented yet."));
        viewPanel->addLargeAction(tableAction);

        QAction* propertyCurvesAction = new QAction(tr("Property Curves"), m_category);
        propertyCurvesAction->setIcon(QIcon(QStringLiteral(":/images/analysis-curve.svg")));
        propertyCurvesAction->setEnabled(false);
        propertyCurvesAction->setToolTip(tr("Planned action; not implemented yet."));
        viewPanel->addLargeAction(propertyCurvesAction);

        QAction* wellControlsAction = new QAction(tr("Well Controls"), m_category);
        wellControlsAction->setIcon(QIcon(QStringLiteral(":/images/simulation-well-controls.svg")));
        wellControlsAction->setEnabled(false);
        wellControlsAction->setToolTip(tr("Planned action; not implemented yet."));
        viewPanel->addLargeAction(wellControlsAction);

        SARibbonPanel* runPanel = m_category->addPanel(tr("Run"));
        QAction* runPreflightAction = new QAction(tr("Run Preflight"), m_category);
        runPreflightAction->setIcon(QIcon(QStringLiteral(":/images/simulation-preflight.svg")));
        runPreflightAction->setEnabled(false);
        runPreflightAction->setToolTip(tr("Planned action; not implemented yet."));
        runPanel->addLargeAction(runPreflightAction);

        QAction* initializeAction = new QAction(tr("Initialize"), m_category);
        initializeAction->setIcon(QIcon(QStringLiteral(":/images/simulation-initialize.svg")));
        initializeAction->setEnabled(false);
        initializeAction->setToolTip(tr("Planned action; not implemented yet."));
        runPanel->addLargeAction(initializeAction);

        QAction* runStatusAction = new QAction(tr("Run Status"), m_category);
        runStatusAction->setIcon(QIcon(QStringLiteral(":/images/analysis-run.svg")));
        runStatusAction->setEnabled(false);
        runStatusAction->setToolTip(tr("Planned action; not implemented yet."));
        runPanel->addLargeAction(runStatusAction);

        QAction* stopTaskAction = new QAction(tr("Stop Task"), m_category);
        stopTaskAction->setIcon(QIcon(QStringLiteral(":/images/simulation-stop.svg")));
        stopTaskAction->setEnabled(false);
        stopTaskAction->setToolTip(tr("Planned action; not implemented yet."));
        runPanel->addLargeAction(stopTaskAction);

        SARibbonPanel* resultsPanel = m_category->addPanel(tr("Results"));
        QAction* resultFilesAction = new QAction(tr("Result Files"), m_category);
        resultFilesAction->setIcon(QIcon(QStringLiteral(":/images/gui-case-results.svg")));
        resultFilesAction->setEnabled(false);
        resultFilesAction->setToolTip(tr("Planned action; not implemented yet."));
        resultsPanel->addLargeAction(resultFilesAction);

        QAction* importResultsAction = new QAction(tr("Import Results"), m_category);
        importResultsAction->setIcon(QIcon(QStringLiteral(":/images/simulation-results-import.svg")));
        importResultsAction->setEnabled(false);
        importResultsAction->setToolTip(tr("Planned action; not implemented yet."));
        resultsPanel->addLargeAction(importResultsAction);

        SARibbonPanel* analysisPanel = m_category->addPanel(tr("Analysis"));
        QAction* curveComparisonAction = new QAction(tr("Curve Comparison"), m_category);
        curveComparisonAction->setIcon(QIcon(QStringLiteral(":/images/simulation-curve-comparison.svg")));
        curveComparisonAction->setEnabled(false);
        curveComparisonAction->setToolTip(tr("Planned action; not implemented yet."));
        analysisPanel->addLargeAction(curveComparisonAction);

        QAction* fitEvaluationAction = new QAction(tr("Fit Evaluation"), m_category);
        fitEvaluationAction->setIcon(QIcon(QStringLiteral(":/images/simulation-fit-evaluation.svg")));
        fitEvaluationAction->setEnabled(false);
        fitEvaluationAction->setToolTip(tr("Planned action; not implemented yet."));
        analysisPanel->addLargeAction(fitEvaluationAction);

        SARibbonPanel* workflowPanel = m_category->addPanel(tr("Workflow"));
        QAction* workflowAction = new QAction(tr("Workflow"), m_category);
        workflowAction->setIcon(QIcon(QStringLiteral(":/images/simulation-workflow.svg")));
        workflowAction->setEnabled(false);
        workflowAction->setToolTip(tr("Planned action; not implemented yet."));
        workflowPanel->addLargeAction(workflowAction);

        QAction* runArchiveAction = new QAction(tr("Run Archive"), m_category);
        runArchiveAction->setIcon(QIcon(QStringLiteral(":/images/simulation-archive.svg")));
        runArchiveAction->setEnabled(false);
        runArchiveAction->setToolTip(tr("Planned action; not implemented yet."));
        workflowPanel->addLargeAction(runArchiveAction);

        SARibbonPanel* reportPanel = m_category->addPanel(tr("Report"));
        QAction* caseInventoryReportAction = new QAction(tr("Case Inventory Report"), m_category);
        caseInventoryReportAction->setIcon(QIcon(QStringLiteral(":/images/simulation-report.svg")));
        caseInventoryReportAction->setEnabled(false);
        caseInventoryReportAction->setToolTip(tr("Planned action; not implemented yet."));
        reportPanel->addLargeAction(caseInventoryReportAction);
    }
    m_initialized = m_category != nullptr;
}

void IDOSSimulationPlugin::unload()
{
    if (m_interface != nullptr && m_category != nullptr)
    {
        m_interface->removeRibbonCategory(m_category);
    }

    m_category = nullptr;
    m_initialized = false;
    uninstallTranslator();
}

extern "C" IDOS_PLUGIN_EXPORT const QString* name()
{
    return &sName;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* description()
{
    return &sDescription;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* category()
{
    return &sCategory;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* version()
{
    return &sPluginVersion;
}

extern "C" IDOS_PLUGIN_EXPORT int type()
{
    return sPluginType;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* icon()
{
    return &sPluginIcon;
}

extern "C" IDOS_PLUGIN_EXPORT void unload(IDOSPlugin* plugin)
{
    if (plugin == nullptr)
    {
        return;
    }

    delete plugin;
}

extern "C" IDOS_PLUGIN_EXPORT IDOSPlugin* classFactory(IDOSInterface* interface)
{
    return new IDOSSimulationPlugin(interface);
}
