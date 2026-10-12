#include <memory>

#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSize>
#include <QStatusBar>
#include <QStringList>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>

#include <DockAreaWidget.h>
#include <DockManager.h>
#include <DockWidget.h>
#include <DockWidgetTab.h>
#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonPanel.h>
#include <SARibbonQuickAccessBar.h>
#include <SARibbonSystemButtonBar.h>

#include "command/idoscommandmanager.h"
#include "command/idoscommandmetadata.h"
#include "command/idoscommandregistry.h"
#include "command/idosimportcommands.h"
#include "idosappinterface.h"
#include "idosapplication.h"
#include "idosassistantservice.h"
#include "idosassistantwidget.h"
#include "idoscaseobject.h"
#include "idoscreatecasecommand.h"
#include "idoscreatewellcommand.h"
#include "idosdataregistry.h"
#include "idosdebuginfowidget.h"
#include "idosdeletecasecommand.h"
#include "idosdeletegridcommand.h"
#include "idosdeletepropertycommand.h"
#include "idosdeletewellcommand.h"
#include "idosgridrendermetadata.h"
#include "idosgridtreeguiprovider.h"
#include "idosgridtreemetadata.h"
#include "idosimportrestartresultscommand.h"
#include "idosnewprojectdialog.h"
#include "idospluginmanagerwidget.h"
#include "idospluginregistry.h"
#include "idosproject.h"
#include "idosprojectmetadata.h"
#include "idospythonconsolewidget.h"
#include "idosrenameobjectcommand.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderregistry.h"
#include "idosrenderscene.h"
#include "idosrenderserver.h"
#include "idosrendertypes.h"
#include "idosrenderview.h"
#include "idosruntimeinfowidget.h"
#include "idossimulationcasetreeguiprovider.h"
#include "idossimulationcasetreemetadata.h"
#include "idostoolbar.h"
#include "idostoolregistry.h"
#include "idostree.h"
#include "idostreeitem.h"
#include "idostreemodel.h"
#include "idostreeregistry.h"
#include "idostreeserver.h"
#include "idostreeview.h"
#include "idoswell.h"
#include "idoswelllogtrackview.h"
#include "idoswellrendermetadata.h"
#include "idoswelltreeguiprovider.h"
#include "idoswelltreemetadata.h"
#include "log/idoslogger.h"

#include "idosmainwindow.h"

IDOSMainWindow::IDOSMainWindow(QWidget* parent)
    :
    SARibbonMainWindow(parent)
    , m_actionNewProject(nullptr)
    , m_actionOpenProject(nullptr)
    , m_actionSaveProject(nullptr)
    , m_actionSaveProjectAs(nullptr)
    , m_actionProjectSettings(nullptr)
    , m_actionUndo(nullptr)
    , m_actionRedo(nullptr)
    , m_actionPluginManager(nullptr)
    , m_actionViewPresets(nullptr)
    , m_actionResetView(nullptr)
    , m_actionFitAll(nullptr)
    , m_actionBackgroundColor(nullptr)
    , m_actionOrientationMarker(nullptr)
    , m_actionLegend(nullptr)
    , m_actionCaptureScreenshot(nullptr)
    , m_actionExportImage(nullptr)
    , m_actionAssistant(nullptr)
    , m_actionPythonConsole(nullptr)
    , m_actionNew3DWindow(nullptr)
    , m_actionNew2DWindow(nullptr)
    , m_actionWindowLayout(nullptr)
    , m_project(nullptr)
    , m_dataRegistry(nullptr)
    , m_commandManager(nullptr)
    , m_dataTree(new IDOSTree(this))
    , m_caseTree(new IDOSTree(this))
    , m_dataTreeModel(new IDOSTreeModel(m_dataTree, this))
    , m_caseTreeModel(new IDOSTreeModel(m_caseTree, this))
    , m_dataTreeView(nullptr)
    , m_caseTreeView(nullptr)
    , m_treeRegistry(new IDOSTreeRegistry(this))
    , m_treeServer(new IDOSTreeServer(m_treeRegistry, this))
    , m_renderRegistry(new IDOSRenderRegistry(this))
    , m_toolRegistry(new IDOSToolRegistry(this))
    , m_renderServer(new IDOSRenderServer(m_renderRegistry, this))
    , m_runtimeInfoWidget(nullptr)
    , m_pluginRegistry(nullptr)
    , m_pluginManagerWidget(nullptr)
    , m_appInterface(new IDOSAppInterface(this))
    , m_debugInfoWidget(nullptr)
    , m_assistantWidget(nullptr)
    , m_pythonConsoleWidget(nullptr)
    , m_dockManager(nullptr)
    , m_renderDock(nullptr)
    , m_outputDock(nullptr)
    , m_debugDock(nullptr)
    , m_pythonConsoleDock(nullptr)
    , m_assistantDock(nullptr)
    , m_viewCaseIds()
    , m_nextViewIndex(1)
    , m_currentWindowLayout(QStringLiteral("single"))
    , m_syncingTreeSelection(false)
{
    IDOSApplication* application = qobject_cast<IDOSApplication*>(QApplication::instance());
    m_dataRegistry = application != nullptr ? application->dataRegistry() : new IDOSDataRegistry(this);
    initProviders();
    initMainWindow();
    initDockManager();
    initRenderView();
    initRibbonAction();

    initDockWidgets();
    IDOS_MESSAGE(tr("Application started."), IDOSLogLevel::Info);
    initPluginManager();
    QTimer::singleShot(0, m_assistantWidget, &IDOSAssistantWidget::startAssistant);
}

void IDOSMainWindow::initProviders()
{
    IDOSApplication::initProviders();
    m_dataTree->setName(QStringLiteral("data"));
    m_caseTree->setName(QStringLiteral("case"));
    m_treeRegistry->add(new IDOSWellTreeMetadata());
    m_treeRegistry->add(new IDOSGridTreeMetadata());
    m_treeRegistry->add(new IDOSSimulationCaseTreeMetadata(m_treeRegistry));
    m_treeRegistry->addGuiProvider(std::make_shared<IDOSWellTreeGuiProvider>());
    m_treeRegistry->addGuiProvider(std::make_shared<IDOSGridTreeGuiProvider>());
    m_treeRegistry->addGuiProvider(std::make_shared<IDOSSimulationCaseTreeGuiProvider>());
    IDOSTreeItem* wellGroup = nullptr;
    IDOSTreeItem* group0 = new IDOSTreeItem();
    group0->setKey(QStringLiteral("data.wellGroup"));
    group0->setName(tr("Well Group"));
    group0->setIcon(QIcon(QStringLiteral(":/images/gui-well-group.svg")));
    group0->setGroup(true);
    wellGroup = group0;
    IDOSTreeItem* group1 = new IDOSTreeItem();
    group1->setKey(QStringLiteral("data.globalLogs"));
    group1->setName(tr("Global Well Logs"));
    group1->setIcon(QIcon(QStringLiteral(":/images/gui-global-logs.svg")));
    group1->setGroup(true);
    wellGroup->appendChild(group1);
    IDOSTreeItem* group2 = new IDOSTreeItem();
    group2->setKey(QStringLiteral("data.globalCompletions"));
    group2->setName(tr("Global Completions"));
    group2->setIcon(QIcon(QStringLiteral(":/images/gui-global-completions.svg")));
    group2->setGroup(true);
    wellGroup->appendChild(group2);
    IDOSTreeItem* group3 = new IDOSTreeItem();
    group3->setKey(QStringLiteral("data.globalObservations"));
    group3->setName(tr("Global Observations"));
    group3->setIcon(QIcon(QStringLiteral(":/images/gui-global-observations.svg")));
    group3->setGroup(true);
    wellGroup->appendChild(group3);
    IDOSTreeItem* group4 = new IDOSTreeItem();
    group4->setKey(QStringLiteral("data.wellProperties"));
    group4->setName(tr("Well Properties"));
    group4->setIcon(QIcon(QStringLiteral(":/images/gui-well-properties.svg")));
    group4->setGroup(true);
    wellGroup->appendChild(group4);
    IDOSTreeItem* group5 = new IDOSTreeItem();
    group5->setKey(QStringLiteral("data.wells"));
    group5->setName(tr("Wells"));
    group5->setIcon(QIcon(QStringLiteral(":/images/gui-wells.svg")));
    group5->setGroup(true);
    wellGroup->appendChild(group5);
    m_treeRegistry->addGroup(QStringLiteral("data"), wellGroup);
    IDOSTreeItem* group6 = new IDOSTreeItem();
    group6->setKey(QStringLiteral("data.seismic"));
    group6->setName(tr("Seismic"));
    group6->setIcon(QIcon(QStringLiteral(":/images/gui-seismic.svg")));
    group6->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group6);
    IDOSTreeItem* group7 = new IDOSTreeItem();
    group7->setKey(QStringLiteral("data.surfaceProperties"));
    group7->setName(tr("Surface Properties"));
    group7->setIcon(QIcon(QStringLiteral(":/images/gui-surface-properties.svg")));
    group7->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group7);
    IDOSTreeItem* group8 = new IDOSTreeItem();
    group8->setKey(QStringLiteral("data.faults"));
    group8->setName(tr("Faults"));
    group8->setIcon(QIcon(QStringLiteral(":/images/gui-faults.svg")));
    group8->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group8);
    IDOSTreeItem* group9 = new IDOSTreeItem();
    group9->setKey(QStringLiteral("data.boundaries"));
    group9->setName(tr("Polygons / Boundaries"));
    group9->setIcon(QIcon(QStringLiteral(":/images/gui-boundaries.svg")));
    group9->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group9);
    IDOSTreeItem* group10 = new IDOSTreeItem();
    group10->setKey(QStringLiteral("data.naturalFractures"));
    group10->setName(tr("Natural Fractures"));
    group10->setIcon(QIcon(QStringLiteral(":/images/gui-natural-fractures.svg")));
    group10->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group10);
    IDOSTreeItem* group11 = new IDOSTreeItem();
    group11->setKey(QStringLiteral("data.hydraulicFractures"));
    group11->setName(tr("Hydraulic Fracturing"));
    group11->setIcon(QIcon(QStringLiteral(":/images/gui-hydraulic-fractures.svg")));
    group11->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group11);
    IDOSTreeItem* group12 = new IDOSTreeItem();
    group12->setKey(QStringLiteral("data.fluids"));
    group12->setName(tr("Fluids"));
    group12->setIcon(QIcon(QStringLiteral(":/images/gui-fluids.svg")));
    group12->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group12);
    IDOSTreeItem* group13 = new IDOSTreeItem();
    group13->setKey(QStringLiteral("data.rockPhysics"));
    group13->setName(tr("Rock Physics Functions"));
    group13->setIcon(QIcon(QStringLiteral(":/images/gui-rock-physics.svg")));
    group13->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group13);
    IDOSTreeItem* group14 = new IDOSTreeItem();
    group14->setKey(QStringLiteral("data.initialConditions"));
    group14->setName(tr("Initial Conditions"));
    group14->setIcon(QIcon(QStringLiteral(":/images/gui-initial-conditions.svg")));
    group14->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group14);
    IDOSTreeItem* group15 = new IDOSTreeItem();
    group15->setKey(QStringLiteral("data.microseismic"));
    group15->setName(tr("Microseismic"));
    group15->setIcon(QIcon(QStringLiteral(":/images/gui-microseismic.svg")));
    group15->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group15);
    IDOSTreeItem* group16 = new IDOSTreeItem();
    group16->setKey(QStringLiteral("data.production"));
    group16->setName(tr("Production Summary"));
    group16->setIcon(QIcon(QStringLiteral(":/images/gui-production.svg")));
    group16->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group16);
    IDOSTreeItem* group17 = new IDOSTreeItem();
    group17->setKey(QStringLiteral("data.streamlines"));
    group17->setName(tr("Flow Diagnostics"));
    group17->setIcon(QIcon(QStringLiteral(":/images/gui-flow-diagnostics.svg")));
    group17->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group17);
    IDOSTreeItem* group18 = new IDOSTreeItem();
    group18->setKey(QStringLiteral("data.management"));
    group18->setName(tr("Field Management Strategies"));
    group18->setIcon(QIcon(QStringLiteral(":/images/gui-field-management.svg")));
    group18->setGroup(true);
    m_treeRegistry->addGroup(QStringLiteral("data"), group18);
    m_treeServer->addTree(m_dataTree);
    m_treeServer->addTree(m_caseTree);

    m_renderRegistry->add(new IDOSGridRenderMetadata());
    m_renderRegistry->add(new IDOSWellRenderMetadata());
    connect(m_renderServer, &IDOSRenderServer::objectsChanged, this, &IDOSMainWindow::onRenderObjectsChanged);
    connect(m_renderServer, &IDOSRenderServer::dataAboutToBeRemoved, this, &IDOSMainWindow::onRenderDataRemoving);
}

void IDOSMainWindow::initMainWindow()
{
    setWindowTitle(tr("IDOS"));
    setWindowIcon(QIcon(":/images/app-logo.svg"));
    statusBar()->addPermanentWidget(new QLabel(tr("Ready"), statusBar()));
}

void IDOSMainWindow::initDockManager()
{
    ads::CDockManager::setConfigFlag(ads::CDockManager::ActiveTabHasCloseButton, false);
    m_dockManager = new ads::CDockManager(this);
    m_dockManager->setStyleSheet(QString());
    setCentralWidget(m_dockManager);
    connect(m_dockManager,
            &ads::CDockManager::focusedDockWidgetChanged,
            this,
            &IDOSMainWindow::onFocusedDockWidgetChanged);
}

void IDOSMainWindow::initRenderView()
{
    QWidget* renderViewWidget = createView(QStringLiteral("main3d"), false);
    if (renderViewWidget == nullptr)
    {
        return;
    }
    m_renderDock = new ads::CDockWidget(m_dockManager, tr("3D View"));
    m_renderDock->setObjectName(QStringLiteral("main3DDock"));
    m_renderDock->setIcon(QIcon(QStringLiteral(":/images/render-view.svg")));
    m_renderDock->setWidget(renderViewWidget);
    m_renderDock->setFeatures(ads::CDockWidget::NoDockWidgetFeatures);
    m_dockManager->setCentralWidget(m_renderDock);
    m_viewDocks.insert(QStringLiteral("main3d"), m_renderDock);
}

void IDOSMainWindow::initDockWidgets()
{
    m_dataTreeView = new IDOSTreeView(this);
    m_dataTreeView->setModel(m_dataTreeModel);
    connect(m_dataTreeView, &QTreeView::doubleClicked, this, &IDOSMainWindow::onDataTreeItemActivated);
    connect(m_dataTreeView, &QTreeView::clicked, this, &IDOSMainWindow::onDataTreeItemClicked);
    connect(m_dataTreeView->selectionModel(), &QItemSelectionModel::currentChanged,this, &IDOSMainWindow::onDataTreeCurrentChanged);

    m_dataTreeView->setRegistry(m_treeRegistry);
    ads::CDockWidget* dataDock = new ads::CDockWidget(m_dockManager, tr("Data"));
    dataDock->setObjectName(QStringLiteral("dataTreeDock"));
    dataDock->setIcon(QIcon(QStringLiteral(":/images/gui-data-tree.svg")));
    dataDock->setWidget(m_dataTreeView);
    dataDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    ads::CDockAreaWidget* dataArea = m_dockManager->addDockWidget(ads::LeftDockWidgetArea, dataDock);

    m_runtimeInfoWidget = new IDOSRuntimeInfoWidget(this);
    m_outputDock = new ads::CDockWidget(m_dockManager, tr("Output"));
    m_outputDock->setObjectName(QStringLiteral("outputDock"));
    m_outputDock->setIcon(QIcon(QStringLiteral(":/images/gui-output.svg")));
    m_outputDock->tabWidget()->setIconSize(QSize(16, 16));
    m_outputDock->setWidget(m_runtimeInfoWidget, ads::CDockWidget::ForceNoScrollArea);
    m_outputDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    ads::CDockAreaWidget* outputArea = m_dockManager->addDockWidget(ads::BottomDockWidgetArea, m_outputDock, m_renderDock->dockAreaWidget());

    m_debugInfoWidget = new IDOSDebugInfoWidget(this);
    m_debugDock = new ads::CDockWidget(m_dockManager, tr("Debug"));
    m_debugDock->setObjectName(QStringLiteral("debugDock"));
    m_debugDock->setIcon(QIcon(QStringLiteral(":/images/gui-debug.svg")));
    m_debugDock->tabWidget()->setIconSize(QSize(16, 16));
    m_debugDock->setWidget(m_debugInfoWidget, ads::CDockWidget::ForceNoScrollArea);
    m_debugDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidgetTabToArea(m_debugDock, outputArea);

    m_pythonConsoleWidget = new IDOSPythonConsoleWidget(this);
    m_pythonConsoleDock = new ads::CDockWidget(m_dockManager, tr("Python Console"));
    m_pythonConsoleDock->setObjectName(QStringLiteral("pythonConsoleDock"));
    m_pythonConsoleDock->setIcon(QIcon(QStringLiteral(":/images/python-console.svg")));
    m_pythonConsoleDock->tabWidget()->setIconSize(QSize(16, 16));
    m_pythonConsoleDock->setWidget(m_pythonConsoleWidget, ads::CDockWidget::ForceNoScrollArea);
    m_pythonConsoleDock->setFeatures(ads::CDockWidget::DockWidgetMovable
                                     | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidgetTabToArea(m_pythonConsoleDock, outputArea);

    m_assistantWidget = new IDOSAssistantWidget(this);
    IDOSCommandRegistry* commandRegistry = m_assistantWidget->service()->commandRegistry();
    if (commandRegistry != nullptr)
    {
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateWellCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteGridCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeletePropertyCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteWellCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSRenameObjectCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportRestartResultsCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportGridCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportPropertyCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportWellDataCommandMetadata()));
    }
    m_assistantDock = new ads::CDockWidget(m_dockManager, tr("Assistant"));
    m_assistantDock->setObjectName(QStringLiteral("assistantDock"));
    m_assistantDock->setIcon(QIcon(QStringLiteral(":/images/assistant-chat.svg")));
    m_assistantDock->tabWidget()->setIconSize(QSize(16, 16));
    m_assistantDock->setWidget(m_assistantWidget, ads::CDockWidget::ForceNoScrollArea);
    m_assistantDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidgetTabToArea(m_assistantDock, outputArea);

    // ---- 工况树 ----
    m_caseTreeView = new IDOSTreeView(this);
    m_caseTreeView->setBlankMenuEnabled(true);
    m_caseTreeView->setModel(m_caseTreeModel);
    connect(m_caseTreeView, &QTreeView::clicked, this, &IDOSMainWindow::onCaseTreeItemClicked);
    connect(m_caseTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &IDOSMainWindow::onCaseTreeCurrentChanged);
    m_caseTreeView->setRegistry(m_treeRegistry);
    m_dataTreeView->setTargetView(activeView());
    m_caseTreeView->setTargetView(activeView());
    ads::CDockWidget* caseDock = new ads::CDockWidget(m_dockManager, tr("Case"));
    caseDock->setObjectName(QStringLiteral("caseTreeDock"));
    caseDock->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    caseDock->setWidget(m_caseTreeView);
    caseDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidget(ads::BottomDockWidgetArea, caseDock, dataArea);

    updateTreeSelection(activeViewId());
    m_dataTreeView->refreshCheckStates();
    m_caseTreeView->refreshCheckStates();
    outputArea->setCurrentDockWidget(m_outputDock);
}

void IDOSMainWindow::initPluginManager()
{
    m_pluginRegistry = IDOSPluginRegistry::instance();
    m_pluginRegistry->setIDosInterface(m_appInterface);
    m_pluginRegistry->restoreSessionPlugins(m_pluginRegistry->libraryDir().path());

    m_pluginManagerWidget = new IDOSPluginManagerWidget(m_pluginRegistry, this);
    m_pluginManagerWidget->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint
                                          | Qt::WindowCloseButtonHint
                                          | Qt::WindowSystemMenuHint);
    m_pluginManagerWidget->setWindowTitle(tr("Plugin Manager"));
    m_pluginManagerWidget->resize(1000, 640);
}

void IDOSMainWindow::initRibbonAction()
{
    m_actionNewProject = new QAction(QIcon(QStringLiteral(":/images/app-project-new.svg")), tr("New Project"), this);
    m_actionNewProject->setObjectName(QStringLiteral("newProjectAction"));
    m_actionNewProject->setShortcut(QKeySequence::New);
    addAction(m_actionNewProject);
    SARibbonCategory* projectPage = ribbonBar()->addCategoryPage(tr("Home"));
    SARibbonPanel* projectPanel = projectPage->addPanel(tr("Project"));
    projectPanel->addLargeAction(m_actionNewProject);
    m_actionOpenProject = new QAction(
        QIcon(QStringLiteral(":/images/app-project-open.svg")), tr("Open Project"), this);
    m_actionOpenProject->setObjectName(QStringLiteral("openProjectAction"));
    m_actionOpenProject->setEnabled(false);
    projectPanel->addLargeAction(m_actionOpenProject);
    m_actionSaveProject = new QAction(
        QIcon(QStringLiteral(":/images/app-project-save.svg")), tr("Save Project"), this);
    m_actionSaveProject->setObjectName(QStringLiteral("saveProjectAction"));
    m_actionSaveProject->setEnabled(false);
    projectPanel->addSmallAction(m_actionSaveProject);
    m_actionSaveProjectAs = new QAction(
        QIcon(QStringLiteral(":/images/app-project-save.svg")), tr("Save Project As"), this);
    m_actionSaveProjectAs->setObjectName(QStringLiteral("saveProjectAsAction"));
    m_actionSaveProjectAs->setEnabled(false);
    projectPanel->addSmallAction(m_actionSaveProjectAs);
    m_actionProjectSettings = new QAction(
        QIcon(QStringLiteral(":/images/app-project-settings.svg")), tr("Project Settings"), this);
    m_actionProjectSettings->setObjectName(QStringLiteral("projectSettingsAction"));
    m_actionProjectSettings->setEnabled(false);
    projectPanel->addSmallAction(m_actionProjectSettings);
    m_actionPluginManager = new QAction(QIcon(QStringLiteral(":/images/app-plugin.svg")),
                                        tr("Plugin Manager"),
                                        this);
    m_actionPluginManager->setObjectName(QStringLiteral("pluginManagerAction"));
    QMenu* fileMenu = new QMenu(this);
    fileMenu->addAction(m_actionNewProject);
    fileMenu->addAction(m_actionOpenProject);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actionSaveProject);
    fileMenu->addAction(m_actionSaveProjectAs);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actionProjectSettings);
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction(tr("Exit"));
    exitAction->setObjectName(QStringLiteral("exitAction"));

    QAbstractButton* applicationButton = ribbonBar()->applicationButton();
    if (applicationButton != nullptr)
    {
        applicationButton->setText(tr("File"));
        applicationButton->setAccessibleName(tr("File"));
    }
    QToolButton* fileButton = qobject_cast<QToolButton*>(applicationButton);
    if (fileButton != nullptr)
    {
        fileButton->setMenu(fileMenu);
        fileButton->setPopupMode(QToolButton::InstantPopup);
        fileButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    m_actionUndo = new QAction(QIcon(QStringLiteral(":/images/app-undo.svg")),
                               tr("Undo"),
                               this);
    m_actionUndo->setObjectName(QStringLiteral("undoAction"));
    m_actionUndo->setShortcut(QKeySequence::Undo);
    m_actionUndo->setEnabled(false);
    addAction(m_actionUndo);

    m_actionRedo = new QAction(QIcon(QStringLiteral(":/images/app-redo.svg")),
                               tr("Redo"),
                               this);
    m_actionRedo->setObjectName(QStringLiteral("redoAction"));
    m_actionRedo->setShortcut(QKeySequence::Redo);
    m_actionRedo->setEnabled(false);
    addAction(m_actionRedo);
    SARibbonQuickAccessBar* quickAccessBar = ribbonBar()->quickAccessBar();
    quickAccessBar->addAction(m_actionUndo);
    quickAccessBar->addAction(m_actionRedo);

    SARibbonPanel* viewPanel = projectPage->addPanel(tr("View"));
    m_actionViewPresets = new QAction(
        QIcon(QStringLiteral(":/images/render-standard-views.svg")), tr("View Presets"), this);
    m_actionViewPresets->setObjectName(QStringLiteral("viewPresetsAction"));
    QMenu* viewPresetsMenu = new QMenu(this);
    QActionGroup* viewPresetActionGroup = new QActionGroup(this);
    viewPresetActionGroup->setExclusive(true);
    QAction* frontViewAction = viewPresetsMenu->addAction(tr("Front"));
    frontViewAction->setCheckable(true);
    frontViewAction->setObjectName(QStringLiteral("viewPresetFrontAction"));
    frontViewAction->setData(static_cast<int>(IDOSOrientation::Front));
    viewPresetActionGroup->addAction(frontViewAction);
    QAction* leftViewAction = viewPresetsMenu->addAction(tr("Left"));
    leftViewAction->setCheckable(true);
    leftViewAction->setObjectName(QStringLiteral("viewPresetLeftAction"));
    leftViewAction->setData(static_cast<int>(IDOSOrientation::Left));
    viewPresetActionGroup->addAction(leftViewAction);
    QAction* topViewAction = viewPresetsMenu->addAction(tr("Top"));
    topViewAction->setCheckable(true);
    topViewAction->setObjectName(QStringLiteral("viewPresetTopAction"));
    topViewAction->setData(static_cast<int>(IDOSOrientation::Top));
    viewPresetActionGroup->addAction(topViewAction);
    QAction* backViewAction = viewPresetsMenu->addAction(tr("Back"));
    backViewAction->setCheckable(true);
    backViewAction->setObjectName(QStringLiteral("viewPresetBackAction"));
    backViewAction->setData(static_cast<int>(IDOSOrientation::Back));
    backViewAction->setProperty("threeDimensionalOnly", true);
    viewPresetActionGroup->addAction(backViewAction);
    QAction* rightViewAction = viewPresetsMenu->addAction(tr("Right"));
    rightViewAction->setCheckable(true);
    rightViewAction->setObjectName(QStringLiteral("viewPresetRightAction"));
    rightViewAction->setData(static_cast<int>(IDOSOrientation::Right));
    rightViewAction->setProperty("threeDimensionalOnly", true);
    viewPresetActionGroup->addAction(rightViewAction);
    QAction* bottomViewAction = viewPresetsMenu->addAction(tr("Bottom"));
    bottomViewAction->setCheckable(true);
    bottomViewAction->setObjectName(QStringLiteral("viewPresetBottomAction"));
    bottomViewAction->setData(static_cast<int>(IDOSOrientation::Bottom));
    bottomViewAction->setProperty("threeDimensionalOnly", true);
    viewPresetActionGroup->addAction(bottomViewAction);
    QAction* isometricViewAction = viewPresetsMenu->addAction(tr("Isometric"));
    isometricViewAction->setCheckable(true);
    isometricViewAction->setObjectName(QStringLiteral("viewPresetIsometricAction"));
    isometricViewAction->setData(static_cast<int>(IDOSOrientation::Isometric));
    isometricViewAction->setProperty("threeDimensionalOnly", true);
    viewPresetActionGroup->addAction(isometricViewAction);
    m_actionViewPresets->setMenu(viewPresetsMenu);
    viewPanel->addSmallAction(m_actionViewPresets, QToolButton::InstantPopup);

    m_actionResetView = new QAction(
        QIcon(QStringLiteral(":/images/render-fit.svg")), tr("Reset View"), this);
    m_actionResetView->setObjectName(QStringLiteral("resetViewAction"));
    viewPanel->addSmallAction(m_actionResetView);

    m_actionFitAll = new QAction(
        QIcon(QStringLiteral(":/images/render-fit.svg")), tr("Fit All"), this);
    m_actionFitAll->setObjectName(QStringLiteral("fitAllAction"));
    viewPanel->addSmallAction(m_actionFitAll);
    m_actionBackgroundColor = new QAction(
        QIcon(QStringLiteral(":/images/render-scene.svg")), tr("Background Color"), this);
    m_actionBackgroundColor->setObjectName(QStringLiteral("backgroundColorAction"));
    viewPanel->addSmallAction(m_actionBackgroundColor);

    m_actionOrientationMarker = new QAction(
        QIcon(QStringLiteral(":/images/render-coordinate.svg")), tr("Orientation Marker"), this);
    m_actionOrientationMarker->setObjectName(QStringLiteral("orientationMarkerAction"));
    m_actionOrientationMarker->setCheckable(true);
    viewPanel->addSmallAction(m_actionOrientationMarker);
    m_actionLegend = new QAction(
        QIcon(QStringLiteral(":/images/render-legend.svg")), tr("Legend"), this);
    m_actionLegend->setObjectName(QStringLiteral("legendAction"));
    m_actionLegend->setCheckable(true);
    viewPanel->addSmallAction(m_actionLegend);
    connect(m_actionOrientationMarker, &QAction::toggled,
            this, &IDOSMainWindow::onOrientationMarkerToggled);
    connect(m_actionLegend, &QAction::toggled, this, &IDOSMainWindow::onLegendToggled);

    SARibbonPanel* windowPanel = projectPage->addPanel(tr("Window"));
    m_actionNew3DWindow = new QAction(
        QIcon(QStringLiteral(":/images/render-view.svg")), tr("New 3D Window"), this);
    m_actionNew3DWindow->setObjectName(QStringLiteral("new3DWindowAction"));
    windowPanel->addSmallAction(m_actionNew3DWindow);
    m_actionNew2DWindow = new QAction(
        QIcon(QStringLiteral(":/images/gui-case-views.svg")), tr("New 2D Window"), this);
    m_actionNew2DWindow->setObjectName(QStringLiteral("new2DWindowAction"));
    windowPanel->addSmallAction(m_actionNew2DWindow);
    m_actionWindowLayout = new QAction(
        QIcon(QStringLiteral(":/images/gui-data-tree.svg")), tr("Window Layout"), this);
    m_actionWindowLayout->setObjectName(QStringLiteral("windowLayoutAction"));
    QMenu* windowLayoutMenu = new QMenu(this);
    QAction* singleWindowAction = windowLayoutMenu->addAction(
        QIcon(QStringLiteral(":/images/render-view.svg")), tr("Single Window"));
    singleWindowAction->setObjectName(QStringLiteral("singleWindowAction"));
    singleWindowAction->setData(QStringLiteral("single"));
    QAction* sideBySideWindowsAction = windowLayoutMenu->addAction(
        QIcon(QStringLiteral(":/images/render-section.svg")), tr("Side by Side"));
    sideBySideWindowsAction->setObjectName(QStringLiteral("sideBySideWindowsAction"));
    sideBySideWindowsAction->setData(QStringLiteral("side-by-side"));
    QAction* stackWindowsAction = windowLayoutMenu->addAction(
        QIcon(QStringLiteral(":/images/render-section-position.svg")), tr("Stack Windows"));
    stackWindowsAction->setObjectName(QStringLiteral("stackWindowsAction"));
    stackWindowsAction->setData(QStringLiteral("stacked"));
    windowLayoutMenu->addSeparator();
    QMenu* gridLayoutMenu = windowLayoutMenu->addMenu(
        QIcon(QStringLiteral(":/images/gui-data-tree.svg")), tr("Grid Layout"));
    gridLayoutMenu->setObjectName(QStringLiteral("gridLayoutMenu"));
    QAction* gridLayout1x2Action = gridLayoutMenu->addAction(tr("1 × 2"));
    gridLayout1x2Action->setObjectName(QStringLiteral("gridLayout1x2Action"));
    gridLayout1x2Action->setData(QStringLiteral("grid:1:2"));
    QAction* gridLayout2x1Action = gridLayoutMenu->addAction(tr("2 × 1"));
    gridLayout2x1Action->setObjectName(QStringLiteral("gridLayout2x1Action"));
    gridLayout2x1Action->setData(QStringLiteral("grid:2:1"));
    QAction* gridLayout2x2Action = gridLayoutMenu->addAction(tr("2 × 2"));
    gridLayout2x2Action->setObjectName(QStringLiteral("gridLayout2x2Action"));
    gridLayout2x2Action->setData(QStringLiteral("grid:2:2"));
    QAction* gridLayout2x3Action = gridLayoutMenu->addAction(tr("2 × 3"));
    gridLayout2x3Action->setObjectName(QStringLiteral("gridLayout2x3Action"));
    gridLayout2x3Action->setData(QStringLiteral("grid:2:3"));
    QAction* gridLayout3x2Action = gridLayoutMenu->addAction(tr("3 × 2"));
    gridLayout3x2Action->setObjectName(QStringLiteral("gridLayout3x2Action"));
    gridLayout3x2Action->setData(QStringLiteral("grid:3:2"));
    QAction* gridLayout3x3Action = gridLayoutMenu->addAction(tr("3 × 3"));
    gridLayout3x3Action->setObjectName(QStringLiteral("gridLayout3x3Action"));
    gridLayout3x3Action->setData(QStringLiteral("grid:3:3"));
    m_actionWindowLayout->setMenu(windowLayoutMenu);
    windowPanel->addSmallAction(m_actionWindowLayout, QToolButton::InstantPopup);

    SARibbonPanel* outputPanel = projectPage->addPanel(tr("Output"));
    m_actionCaptureScreenshot = new QAction(
        QIcon(QStringLiteral(":/images/render-view.svg")), tr("Capture Screenshot"), this);
    m_actionCaptureScreenshot->setObjectName(QStringLiteral("captureScreenshotAction"));
    outputPanel->addSmallAction(m_actionCaptureScreenshot);
    m_actionExportImage = new QAction(
        QIcon(QStringLiteral(":/images/app-export.svg")), tr("Export Image"), this);
    m_actionExportImage->setObjectName(QStringLiteral("exportImageAction"));
    outputPanel->addSmallAction(m_actionExportImage);

    SARibbonPanel* extensionPanel = projectPage->addPanel(tr("Extensions"));
    m_actionAssistant = new QAction(
        QIcon(QStringLiteral(":/images/assistant-chat.svg")), tr("AI Assistant"), this);
    m_actionAssistant->setObjectName(QStringLiteral("aiAssistantAction"));
    extensionPanel->addSmallAction(m_actionAssistant);
    extensionPanel->addSmallAction(m_actionPluginManager);
    m_actionPythonConsole = new QAction(
        QIcon(QStringLiteral(":/images/python-console.svg")), tr("Python Console"), this);
    m_actionPythonConsole->setObjectName(QStringLiteral("pythonConsoleAction"));
    extensionPanel->addSmallAction(m_actionPythonConsole);

    connect(m_actionNewProject, &QAction::triggered,
            this, &IDOSMainWindow::onNewProject);
    connect(m_actionPluginManager, &QAction::triggered,
            this, &IDOSMainWindow::onPluginManagerTriggered);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    connect(m_actionUndo, &QAction::triggered,
            this, &IDOSMainWindow::onUndoTriggered);
    connect(m_actionRedo, &QAction::triggered,
            this, &IDOSMainWindow::onRedoTriggered);
    connect(viewPresetActionGroup, &QActionGroup::triggered,
            this, &IDOSMainWindow::onViewPresetTriggered);
    connect(m_actionResetView, &QAction::triggered,
            this, &IDOSMainWindow::onResetViewTriggered);
    connect(m_actionFitAll, &QAction::triggered,
            this, &IDOSMainWindow::onFitAllTriggered);
    connect(m_actionBackgroundColor, &QAction::triggered,
            this, &IDOSMainWindow::onBackgroundColorTriggered);
    connect(m_actionCaptureScreenshot, &QAction::triggered,
            this, &IDOSMainWindow::onCaptureScreenshotTriggered);
    connect(m_actionExportImage, &QAction::triggered,
            this, &IDOSMainWindow::onExportImageTriggered);
    connect(m_actionAssistant, &QAction::triggered,
            this, &IDOSMainWindow::onAssistantTriggered);
    connect(m_actionPythonConsole, &QAction::triggered,
            this, &IDOSMainWindow::onPythonConsoleTriggered);
    connect(m_actionNew3DWindow, &QAction::triggered,
            this, &IDOSMainWindow::onNew3DWindowTriggered);
    connect(m_actionNew2DWindow, &QAction::triggered,
            this, &IDOSMainWindow::onNew2DWindowTriggered);
    connect(windowLayoutMenu, &QMenu::triggered,
            this, &IDOSMainWindow::onWindowLayoutTriggered);
    connect(gridLayoutMenu, &QMenu::triggered,
            this, &IDOSMainWindow::onWindowLayoutTriggered);
    onCurrentViewChanged(activeViewId());
    onViewDecorationsChanged(activeView()->axesVisible(),
                             activeView()->legendAvailable(),
                             activeView()->legendVisible());
}

IDOSMainWindow::~IDOSMainWindow()
{
    SARibbonBar* applicationRibbonBar = ribbonBar();
    if (applicationRibbonBar != nullptr)
    {
        const QList<SARibbonCategory*> categories = applicationRibbonBar->categoryPages(true);
        IDOS_DEBUG(tr("Main window teardown started: ribbonBar=0x%1, categoryCount=%2")
                       .arg(QString::number(reinterpret_cast<quintptr>(applicationRibbonBar), 16))
                       .arg(categories.size()));
        for (int index = 0; index < categories.size(); ++index)
        {
            SARibbonCategory* category = categories.at(index);
            IDOS_DEBUG(tr("Ribbon category before plugin unload: index=%1, objectName=%2, "
                          "title=%3, category=0x%4, parent=0x%5")
                           .arg(index)
                           .arg(category->objectName(),
                                category->categoryName(),
                                QString::number(reinterpret_cast<quintptr>(category), 16),
                                QString::number(reinterpret_cast<quintptr>(category->parent()), 16)));
        }
    }

    IDOS_DEBUG(tr("Unloading all plugins during main window teardown."));
    m_pluginRegistry->unloadAll();
    if (applicationRibbonBar != nullptr)
    {
        const QList<SARibbonCategory*> categories = applicationRibbonBar->categoryPages(true);
        IDOS_DEBUG(tr("Plugin unload finished during main window teardown: remainingRibbonCategories=%1")
                       .arg(categories.size()));
        for (int index = 0; index < categories.size(); ++index)
        {
            SARibbonCategory* category = categories.at(index);
            IDOS_DEBUG(tr("Ribbon category before base teardown: index=%1, objectName=%2, "
                          "title=%3, category=0x%4, parent=0x%5")
                           .arg(index)
                           .arg(category->objectName(),
                                category->categoryName(),
                                QString::number(reinterpret_cast<quintptr>(category), 16),
                                QString::number(reinterpret_cast<quintptr>(category->parent()), 16)));
        }
    }
    m_pluginRegistry->setIDosInterface(nullptr);
    m_pluginRegistry = nullptr;
    delete m_appInterface;
    m_appInterface = nullptr;
    m_treeServer->setProject(nullptr);
    if (m_project)
    {
        disconnect(m_project, nullptr, this, nullptr);
    }
    disconnect(m_commandManager.data(), nullptr, this, nullptr);
    delete m_treeServer;
    delete m_treeRegistry;
    disconnect(m_renderServer, nullptr, this, nullptr);
    delete m_renderServer;
    delete m_renderRegistry;
    delete m_toolRegistry;
    m_toolRegistry = nullptr;
    for (const QPointer<IDOSRenderView>& view : m_views)
    {
        if (view != nullptr)
        {
            disconnect(view, nullptr, this, nullptr);
        }
    }
    IDOS_DEBUG(tr("Main window teardown body completed."));
}

IDOSTreeView* IDOSMainWindow::dataTreeView() const
{
    return m_dataTreeView;
}

IDOSTreeView* IDOSMainWindow::caseTreeView() const
{
    return m_caseTreeView;
}

void IDOSMainWindow::setProject(IDOSProject* project)
{
    if (m_project == project)
    {
        return;
    }

    IDOS_DEBUG(tr("Changing active project: previous=0x%1, next=0x%2")
                   .arg(QString::number(reinterpret_cast<quintptr>(m_project), 16),
                        QString::number(reinterpret_cast<quintptr>(project), 16)));

    if (m_project)
    {
        disconnect(m_project, nullptr, this, nullptr);
    }
    disconnect(m_commandManager.data(), nullptr, this, nullptr);
    m_project = project;
    m_viewCaseIds.clear();
    m_viewContextIds.clear();
    m_commandManager = project != nullptr ? project->commandManager() : nullptr;
    m_renderServer->setProject(project);
    m_assistantWidget->setProject(project);
    m_treeServer->setProject(project);
    m_dataTreeView->refreshCheckStates();
    m_caseTreeView->refreshCheckStates();
    updateTreeSelection(activeViewId());
    if (project)
    {
        connect(project, &QObject::destroyed, this, &IDOSMainWindow::onProjectDestroyed);
        if (m_commandManager != nullptr)
        {
            connect(m_commandManager.data(), &IDOSCommandManager::stateChanged,
                    this, &IDOSMainWindow::onCommandStateChanged);
        }
    }

    onCommandStateChanged();
    IDOS_INFO(tr("Active project changed."));
    emit projectChanged(project);
}

IDOSTreeModel* IDOSMainWindow::dataTreeModel() const
{
    return m_dataTreeModel;
}

IDOSTreeModel* IDOSMainWindow::caseTreeModel() const
{
    return m_caseTreeModel;
}

bool IDOSMainWindow::createProject(const IDOSProjectMetadata& metadata,
                                   bool confirmDiscard)
{
    if (confirmDiscard && !confirmDiscardProject())
    {
        IDOS_INFO(tr("Project creation canceled by the user."));
        return false;
    }

    IDOSProject* project = new IDOSProject(this);
    if (!project->setMetadata(metadata))
    {
        IDOS_ERROR(tr("Project creation failed because its metadata is invalid."));
        delete project;
        QMessageBox::warning(this,
                             tr("New Project"),
                             tr("Project metadata is invalid."));
        return false;
    }

    IDOSProject* previous = m_project;
    setProject(project);
    m_dataTreeView->expandToDepth(0);
    IDOS_MESSAGE(tr("New project created."), IDOSLogLevel::Info);
    if (previous && previous->parent() == this)
    {
        previous->deleteLater();
    }
    return true;
}

bool IDOSMainWindow::confirmDiscardProject()
{
    if (!m_project || m_project->objects().isEmpty())
    {
        return true;
    }
    QMessageBox warning(
        QMessageBox::Warning, tr("Unsaved Project"),
        tr("The current project has not been saved. Continuing will discard it. Do you want to continue?"),
        QMessageBox::Discard | QMessageBox::Cancel, this);
    warning.button(QMessageBox::Discard)->setText(tr("Discard"));
    warning.button(QMessageBox::Cancel)->setText(tr("Cancel"));
    warning.setDefaultButton(QMessageBox::Cancel);
    return warning.exec() == QMessageBox::Discard;
}

void IDOSMainWindow::onPluginManagerTriggered()
{
    if (m_pluginManagerWidget == nullptr)
    {
        return;
    }

    IDOS_DEBUG(tr("Opening the plugin manager."));
    m_pluginManagerWidget->refresh();
    m_pluginManagerWidget->show();
    m_pluginManagerWidget->raise();
    m_pluginManagerWidget->activateWindow();
}

void IDOSMainWindow::onViewPresetTriggered(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    if (activeView() == nullptr)
    {
        return;
    }
    if (activeView()->isTwoDimensional() && action->property("threeDimensionalOnly").toBool())
    {
        return;
    }

    const int presetValue = action->data().toInt();
    const IDOSOrientation orientation = static_cast<IDOSOrientation>(presetValue);
    activeView()->setOrientation(orientation);
    updateViewPresetActions();
}

void IDOSMainWindow::onCurrentViewChanged(const QString& viewId)
{
    const bool hasActiveView = activeView() != nullptr;
    m_actionViewPresets->setEnabled(hasActiveView);
    m_actionResetView->setEnabled(hasActiveView);
    m_actionFitAll->setEnabled(hasActiveView);
    m_actionBackgroundColor->setEnabled(hasActiveView);
    m_actionCaptureScreenshot->setEnabled(hasActiveView);
    m_actionExportImage->setEnabled(hasActiveView);
    updateViewPresetActions();
    updateTreeSelection(viewId);
}

void IDOSMainWindow::updateViewPresetActions()
{
    if (m_actionViewPresets == nullptr || m_actionViewPresets->menu() == nullptr)
    {
        return;
    }
    IDOSRenderView* view = activeView();
    for (QAction* preset : m_actionViewPresets->menu()->actions())
    {
        if (preset->isSeparator() || !preset->data().isValid())
        {
            continue;
        }
        const bool isSupportedByActiveView =
            view != nullptr && (!view->isTwoDimensional()
                                || !preset->property("threeDimensionalOnly").toBool());
        preset->setEnabled(isSupportedByActiveView);
        if (isSupportedByActiveView)
        {
            preset->setChecked(preset->data().toInt() == static_cast<int>(view->orientation()));
        }
    }
}

void IDOSMainWindow::onActiveViewContextObjectChanged(const QString& viewId,
                                                       const QString& objectId)
{
    Q_UNUSED(objectId)
    if (viewId == activeViewId())
    {
        updateTreeSelection(viewId);
    }
}

void IDOSMainWindow::onFocusedDockWidgetChanged(ads::CDockWidget*, ads::CDockWidget* currentDock)
{
    if (currentDock == nullptr)
    {
        return;
    }

    QMap<QString, QPointer<ads::CDockWidget>>::const_iterator viewIterator = m_viewDocks.constBegin();
    while (viewIterator != m_viewDocks.constEnd())
    {
        if (viewIterator.value() == currentDock)
        {
            setActiveView(viewIterator.key());
            return;
        }
        ++viewIterator;
    }
}

void IDOSMainWindow::onViewDecorationsChanged(bool orientationMarkerVisible,
                                               bool legendAvailable,
                                               bool legendVisible)
{
    const QSignalBlocker orientationBlocker(m_actionOrientationMarker);
    const QSignalBlocker legendBlocker(m_actionLegend);
    m_actionOrientationMarker->setEnabled(activeView() != nullptr);
    m_actionOrientationMarker->setChecked(orientationMarkerVisible);
    m_actionLegend->setEnabled(legendAvailable);
    m_actionLegend->setChecked(legendVisible);
}

void IDOSMainWindow::onOrientationMarkerToggled(bool checked)
{
    if (activeView() != nullptr)
    {
        activeView()->setAxesVisible(checked);
    }
}

void IDOSMainWindow::onLegendToggled(bool checked)
{
    if (activeView() != nullptr)
    {
        activeView()->setLegendVisible(checked);
    }
}

void IDOSMainWindow::onResetViewTriggered()
{

    if (activeView() != nullptr)
    {
        activeView()->setOrientation(activeView()->isTwoDimensional()
                                                     ? activeView()->orientation()
                                                     : IDOSOrientation::Isometric);
        updateViewPresetActions();
    }
}

void IDOSMainWindow::onFitAllTriggered()
{
    if (activeView() != nullptr)
    {
        activeView()->resetCamera();
    }
}

void IDOSMainWindow::onBackgroundColorTriggered()
{
    if (activeView() == nullptr)
    {
        return;
    }

    const QColor selectedColor = QColorDialog::getColor(
        activeView()->background(), this, tr("Background Color"));
    if (selectedColor.isValid())
    {
        activeView()->setBackground(selectedColor);
    }
}

void IDOSMainWindow::onCaptureScreenshotTriggered()
{
    if (activeView() == nullptr)
    {
        return;
    }

    const QImage screenshot = activeView()->captureImage();
    if (screenshot.isNull())
    {
        return;
    }

    QApplication::clipboard()->setImage(screenshot);
    activeView()->flashScreenshot();
    statusBar()->showMessage(tr("Screenshot copied to clipboard."), 3000);
}

void IDOSMainWindow::onExportImageTriggered()
{
    if (activeView() == nullptr)
    {
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Export Image"), QString(), tr("PNG Image (*.png)"));
    if (filePath.isEmpty())
    {
        return;
    }

    QString outputPath = filePath;
    if (QFileInfo(outputPath).suffix().isEmpty())
    {
        outputPath.append(QStringLiteral(".png"));
    }

    const QImage image = activeView()->captureImage();
    if (image.isNull() || !image.save(outputPath, "PNG"))
    {
        QMessageBox::warning(this,
                             tr("Export Image"),
                             tr("Could not save image to %1.").arg(outputPath));
        return;
    }

    statusBar()->showMessage(tr("Image exported to %1.").arg(outputPath), 5000);
}

void IDOSMainWindow::onAssistantTriggered()
{
    if (m_assistantDock == nullptr || m_dockManager == nullptr)
    {
        return;
    }

    m_assistantDock->toggleView(true);
    m_dockManager->setDockWidgetFocused(m_assistantDock);
}

void IDOSMainWindow::onPythonConsoleTriggered()
{
    if (m_pythonConsoleDock == nullptr || m_dockManager == nullptr)
    {
        return;
    }

    m_pythonConsoleDock->toggleView(true);
    m_dockManager->setDockWidgetFocused(m_pythonConsoleDock);
    m_pythonConsoleWidget->setFocus();
}

void IDOSMainWindow::onNew3DWindowTriggered()
{
    createRenderView(false);
}

void IDOSMainWindow::onNew2DWindowTriggered()
{
    createRenderView(true);
}

void IDOSMainWindow::onWindowLayoutTriggered(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    m_currentWindowLayout = action->data().toString();
    applyWindowLayout(m_currentWindowLayout);
}

void IDOSMainWindow::onRenderViewDockClosed()
{
    ads::CDockWidget* dock = qobject_cast<ads::CDockWidget*>(sender());
    if (dock == nullptr)
    {
        return;
    }

    QMap<QString, QPointer<ads::CDockWidget>>::iterator it = m_viewDocks.begin();
    while (it != m_viewDocks.end())
    {
        if (it.value() == dock)
        {
            const QString viewId = it.key();
            if (viewId != QStringLiteral("main3d"))
            {
                removeView(viewId);
                m_viewCaseIds.remove(viewId);
                m_viewDocks.erase(it);
                dock->deleteLater();
                applyWindowLayout(m_currentWindowLayout);
            }
            return;
        }
        ++it;
    }
}

void IDOSMainWindow::createRenderView(bool twoDimensional)
{
    if (m_dockManager == nullptr)
    {
        return;
    }

    const int viewIndex = m_nextViewIndex++;
    const QString viewId = QStringLiteral("view_%1").arg(viewIndex, 4, 10, QLatin1Char('0'));
    QWidget* renderViewWidget = createView(viewId, twoDimensional);
    if (renderViewWidget == nullptr)
    {
        return;
    }
    const QString viewTitle = twoDimensional
                                  ? tr("2D View") + QStringLiteral(" %1").arg(viewIndex)
                                  : tr("3D View") + QStringLiteral(" %1").arg(viewIndex);
    ads::CDockWidget* dock = new ads::CDockWidget(m_dockManager, viewTitle);
    dock->setObjectName(QStringLiteral("renderViewDock_%1").arg(viewIndex));
    dock->setIcon(QIcon(twoDimensional
                            ? QStringLiteral(":/images/gui-case-views.svg")
                            : QStringLiteral(":/images/render-view.svg")));
    dock->setWidget(renderViewWidget);
    dock->setFeatures(ads::CDockWidget::DockWidgetClosable |
                      ads::CDockWidget::DockWidgetMovable |
                      ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidget(ads::CenterDockWidgetArea, dock,
                                 m_renderDock->dockAreaWidget());
    m_viewDocks.insert(viewId, dock);
    connect(dock, &ads::CDockWidget::closed,
            this, &IDOSMainWindow::onRenderViewDockClosed);
    setActiveView(viewId);

    if (m_currentWindowLayout == QStringLiteral("single"))
    {
        m_currentWindowLayout = QStringLiteral("side-by-side");
    }
    applyWindowLayout(m_currentWindowLayout);
    m_dockManager->setDockWidgetFocused(dock);
}

void IDOSMainWindow::applyWindowLayout(const QString& layout)
{
    if (m_dockManager == nullptr)
    {
        return;
    }

    QList<ads::CDockWidget*> docks;
    QMap<QString, QPointer<ads::CDockWidget>>::const_iterator it = m_viewDocks.constBegin();
    while (it != m_viewDocks.constEnd())
    {
        if (!it.value().isNull())
        {
            docks.append(it.value().data());
        }
        ++it;
    }
    if (docks.isEmpty())
    {
        return;
    }

    if (layout == QStringLiteral("single"))
    {
        m_dockManager->setUpdatesEnabled(false);
        for (int index = 0; index < docks.size(); ++index)
        {
            docks.at(index)->toggleView(index == 0);
        }
        m_dockManager->setUpdatesEnabled(true);
        m_dockManager->update();
        return;
    }

    if (layout == QStringLiteral("stacked"))
    {
        m_dockManager->setUpdatesEnabled(false);
        for (int index = 0; index < docks.size(); ++index)
        {
            docks.at(index)->toggleView(true);
        }
        ads::CDockAreaWidget* targetArea = docks.first()->dockAreaWidget();
        for (int index = 1; index < docks.size(); ++index)
        {
            m_dockManager->addDockWidgetTabToArea(docks.at(index), targetArea);
        }
        m_dockManager->setUpdatesEnabled(true);
        m_dockManager->update();
        return;
    }

    int rowCount = 1;
    int columnCount = docks.size();
    const QStringList layoutParts = layout.split(QLatin1Char(':'));
    if (layoutParts.size() == 3 && layoutParts.first() == QStringLiteral("grid"))
    {
        rowCount = layoutParts.at(1).toInt();
        columnCount = layoutParts.at(2).toInt();
    }
    if (layout == QStringLiteral("side-by-side"))
    {
        rowCount = 1;
        columnCount = docks.size();
    }

    if (rowCount < 1 || columnCount < 1)
    {
        return;
    }

    const int requiredRowCount = (docks.size() + columnCount - 1) / columnCount;
    rowCount = qMax(rowCount, requiredRowCount);
    const int visibleDockCount = docks.size();
    ads::CDockManager::setConfigFlag(ads::CDockManager::EqualSplitOnInsertion, true);
    m_dockManager->setUpdatesEnabled(false);
    for (int index = 0; index < docks.size(); ++index)
    {
        docks.at(index)->toggleView(true);
    }

    ads::CDockAreaWidget* targetArea = docks.first()->dockAreaWidget();
    for (int index = 1; index < visibleDockCount; ++index)
    {
        m_dockManager->addDockWidgetTabToArea(docks.at(index), targetArea);
    }

    const int firstRowDockCount = qMin(columnCount, visibleDockCount);
    for (int column = 1; column < firstRowDockCount; ++column)
    {
        const int dockIndex = column;
        m_dockManager->addDockWidget(ads::RightDockWidgetArea,
                                     docks.at(dockIndex),
                                     docks.at(dockIndex - 1)->dockAreaWidget());
    }

    for (int column = 0; column < columnCount; ++column)
    {
        for (int row = 1; row < rowCount; ++row)
        {
            const int dockIndex = row * columnCount + column;
            if (dockIndex >= visibleDockCount)
            {
                break;
            }

            const int anchorIndex = dockIndex - columnCount;
            m_dockManager->addDockWidget(ads::BottomDockWidgetArea,
                                         docks.at(dockIndex),
                                         docks.at(anchorIndex)->dockAreaWidget());
        }
    }
    m_dockManager->setUpdatesEnabled(true);
    m_dockManager->update();
    ads::CDockManager::setConfigFlag(ads::CDockManager::EqualSplitOnInsertion, false);
}

void IDOSMainWindow::onNewProject()
{
    IDOSNewProjectDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }
    createProject(dialog.projectMetadata(), true);
}

void IDOSMainWindow::onUndoTriggered()
{
    if (m_commandManager != nullptr)
    {
        m_commandManager->undo();
    }
}

void IDOSMainWindow::onRedoTriggered()
{
    if (m_commandManager != nullptr)
    {
        m_commandManager->redo();
    }
}

void IDOSMainWindow::onCommandStateChanged()
{
    IDOSCommandManager* commandManager = m_commandManager.data();

    m_actionUndo->setEnabled(commandManager != nullptr && commandManager->canUndo());
    m_actionRedo->setEnabled(commandManager != nullptr && commandManager->canRedo());
    m_actionUndo->setText(commandManager != nullptr ? commandManager->undoText() : tr("Undo"));
    m_actionRedo->setText(commandManager != nullptr ? commandManager->redoText() : tr("Redo"));
}

void IDOSMainWindow::onDataTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    if (m_syncingTreeSelection)
    {
        return;
    }

    QString objectId;
    IDOSDataObject* currentObject = nullptr;
    if (m_project != nullptr && current.isValid())
    {
        IDOSTreeItem* item = m_dataTreeModel->nodeFromIndex(current);
        objectId = item != nullptr ? item->dataId() : QString();
        currentObject = m_dataTreeModel->objectFromIndex(current);
    }
    if (m_project != nullptr)
    {
        m_project->setCurrentObject(currentObject);
    }

    if (current.isValid() && objectId.isEmpty())
    {
        return;
    }
    const QString viewId = activeViewId();
    if (!viewId.isEmpty())
    {
        const QModelIndex caseIndex = objectId.isEmpty()
                                          ? QModelIndex()
                                          : m_caseTreeModel->indexFromDataId(objectId);
        const QString caseId = caseIdFromCaseTreeIndex(caseIndex);
        setViewTreeContext(viewId, objectId, caseId);
        updateTreeSelection(viewId);
    }
}

void IDOSMainWindow::onDataTreeItemClicked(const QModelIndex& index)
{
    if (m_project == nullptr || m_syncingTreeSelection)
    {
        return;
    }
    m_project->setCurrentObject(m_dataTreeModel->objectFromIndex(index));
}

void IDOSMainWindow::onCaseTreeItemClicked(const QModelIndex& index)
{
    if (m_project == nullptr || m_syncingTreeSelection)
    {
        return;
    }
    m_project->setCurrentObject(m_caseTreeModel->objectFromIndex(index));
}

void IDOSMainWindow::onCaseTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    if (m_syncingTreeSelection)
    {
        return;
    }

    if (m_project != nullptr)
    {
        m_project->setCurrentObject(m_caseTreeModel->objectFromIndex(current));
    }

    const QString viewId = activeViewId();
    if (viewId.isEmpty())
    {
        return;
    }

    const QString objectId = objectIdFromCaseTreeIndex(current);
    const QString caseId = caseIdFromCaseTreeIndex(current);
    setViewTreeContext(viewId, objectId, caseId);
    updateTreeSelection(viewId);
}

void IDOSMainWindow::onRenderObjectActivated(const QString& viewId, const QString& objectId)
{
    setActiveView(viewId);
    if (m_project != nullptr)
    {
        m_project->setCurrentObject(m_project->object(objectId));
    }
    const QModelIndex caseIndex = caseTreeIndexForContext(objectId, QString());
    const QString caseId = caseIdFromCaseTreeIndex(caseIndex);
    setViewTreeContext(viewId, objectId, caseId);
    updateTreeSelection(viewId);
}

void IDOSMainWindow::updateTreeSelection(const QString& viewId)
{
    if (m_dataTreeView == nullptr || m_caseTreeView == nullptr)
    {
        return;
    }

    const QString objectId = viewId == activeViewId()
                                 ? viewContextId()
                                 : QString();
    const QString caseId = m_viewCaseIds.value(viewId);
    const QModelIndex dataIndex = objectId.isEmpty()
                                      ? QModelIndex()
                                      : m_dataTreeModel->indexFromDataId(objectId);
    const QModelIndex caseIndex = caseTreeIndexForContext(objectId, caseId);
    m_syncingTreeSelection = true;
    selectTreeIndex(m_dataTreeView, dataIndex);
    selectTreeIndex(m_caseTreeView, caseIndex);
    m_syncingTreeSelection = false;
}

void IDOSMainWindow::setViewTreeContext(const QString& viewId,
                                        const QString& objectId,
                                        const QString& caseId)
{
    if (viewId.isEmpty())
    {
        return;
    }

    if (caseId.isEmpty())
    {
        m_viewCaseIds.remove(viewId);
    }
    else
    {
        m_viewCaseIds.insert(viewId, caseId);
    }

    if (viewId == activeViewId())
    {
        setViewContextId(objectId);
    }
}

void IDOSMainWindow::selectTreeIndex(QTreeView* treeView, const QModelIndex& index)
{
    if (treeView == nullptr || treeView->selectionModel() == nullptr)
    {
        return;
    }

    QSignalBlocker selectionBlocker(treeView->selectionModel());
    if (!index.isValid())
    {
        treeView->selectionModel()->clear();
        return;
    }

    treeView->selectionModel()->setCurrentIndex(
        index,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    QModelIndex parentIndex = index.parent();
    while (parentIndex.isValid())
    {
        treeView->expand(parentIndex);
        parentIndex = parentIndex.parent();
    }
    treeView->scrollTo(index);
}

QModelIndex IDOSMainWindow::caseTreeIndexForContext(const QString& objectId, const QString& caseId) const
{
    if (!caseId.isEmpty())
    {
        const QModelIndex caseIndex = m_caseTreeModel->indexFromDataId(caseId);
        if (!objectId.isEmpty())
        {
            const QModelIndex referenceIndex = findCaseReferenceIndex(caseIndex, objectId);
            if (referenceIndex.isValid())
            {
                return referenceIndex;
            }
        }
        if (caseIndex.isValid())
        {
            return caseIndex;
        }
    }

    if (!objectId.isEmpty())
    {
        return m_caseTreeModel->indexFromDataId(objectId);
    }
    return QModelIndex();
}

QModelIndex IDOSMainWindow::findCaseReferenceIndex(const QModelIndex& parentIndex,
                                                   const QString& objectId) const
{
    if (!parentIndex.isValid() || objectId.isEmpty())
    {
        return QModelIndex();
    }

    for (int row = 0; row < m_caseTreeModel->rowCount(parentIndex); ++row)
    {
        const QModelIndex childIndex = m_caseTreeModel->index(row, 0, parentIndex);
        IDOSTreeItem* referenceNode = m_caseTreeModel->nodeFromIndex(childIndex);
        if (referenceNode != nullptr && referenceNode->isReference() && referenceNode->dataId() == objectId)
        {
            return childIndex;
        }

        const QModelIndex nestedIndex = findCaseReferenceIndex(childIndex, objectId);
        if (nestedIndex.isValid())
        {
            return nestedIndex;
        }
    }
    return QModelIndex();
}

QString IDOSMainWindow::objectIdFromCaseTreeIndex(const QModelIndex& index) const
{
    IDOSTreeItem* item = m_caseTreeModel->nodeFromIndex(index);
    return item != nullptr && item->dataId() != item->caseId() ? item->dataId() : QString();
}

QString IDOSMainWindow::caseIdFromCaseTreeIndex(const QModelIndex& index) const
{
    IDOSTreeItem* item = m_caseTreeModel->nodeFromIndex(index);
    return item != nullptr ? item->caseId() : QString();
}
void IDOSMainWindow::onDataTreeItemActivated(const QModelIndex& index)
{
    if (m_project == nullptr)
    {
        return;
    }

    IDOSTreeItem* partNode = m_dataTreeModel->nodeFromIndex(index);
    if (partNode == nullptr || partNode->partKey() != QStringLiteral("idos.well.logs"))
    {
        return;
    }

    IDOSWell* well = qobject_cast<IDOSWell*>(m_project->object(partNode->dataId()));
    if (well == nullptr)
    {
        return;
    }

    IDOSWellLogTrackView* view = findOrCreateWellLogTrackView(well);
    if (view == nullptr)
    {
        return;
    }

    view->setSelectedChannelName(partNode->itemKey());
}

IDOSWellLogTrackView* IDOSMainWindow::findOrCreateWellLogTrackView(IDOSWell* well)
{
    const QString dockObjectName = QStringLiteral("wellLogTrackDock_%1").arg(well->objectId());
    ads::CDockWidget* dock = m_dockManager->findDockWidget(dockObjectName);
    if (dock != nullptr)
    {
        IDOSWellLogTrackView* view = qobject_cast<IDOSWellLogTrackView*>(dock->widget());
        if (view != nullptr)
        {
            view->setWell(well);
            dock->toggleView(true);
            m_dockManager->setDockWidgetFocused(dock);
        }
        return view;
    }

    IDOSWellLogTrackView* view = new IDOSWellLogTrackView(m_dockManager);
    view->setWell(well);
    dock = new ads::CDockWidget(m_dockManager, tr("Well Log Tracks - %1").arg(well->name()));
    dock->setObjectName(dockObjectName);
    dock->setWidget(view, ads::CDockWidget::ForceNoScrollArea);
    dock->setFeatures(ads::CDockWidget::DockWidgetClosable |
                      ads::CDockWidget::DockWidgetMovable |
                      ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidget(ads::CenterDockWidgetArea, dock,
                                 m_renderDock->dockAreaWidget());
    m_dockManager->setDockWidgetFocused(dock);
    return view;
}

void IDOSMainWindow::closeEvent(QCloseEvent* event)
{
    IDOS_DEBUG(tr("Main window close event received."));
    if (!confirmDiscardProject())
    {
        IDOS_INFO(tr("Main window close canceled because the project has unsaved changes."));
        event->ignore();
        return;
    }
    IDOS_INFO(tr("Main window close accepted."));
    SARibbonMainWindow::closeEvent(event);
}

void IDOSMainWindow::onProjectDestroyed()
{
    IDOS_WARN(tr("Active project was destroyed before the main window."));
    m_project = nullptr;
    m_commandManager = nullptr;
    m_viewCaseIds.clear();
    m_viewContextIds.clear();
    m_treeServer->setProject(nullptr);
    m_renderServer->setProject(nullptr);
    m_assistantWidget->setProject(nullptr);
    emit projectChanged(nullptr);
    updateTreeSelection(activeViewId());
    onCommandStateChanged();
}

IDOSRenderView* IDOSMainWindow::activeView() const
{
    return m_views.value(m_activeViewId).data();
}

QString IDOSMainWindow::activeViewId() const
{
    return m_activeViewId;
}

IDOSRenderView* IDOSMainWindow::createView(const QString& id, bool twoDimensional)
{
    if (id.isEmpty() || m_views.contains(id))
    {
        return nullptr;
    }
    IDOSRenderView* view = new IDOSRenderView(this);
    view->setProperty("viewId", id);
    m_views.insert(id, view);
    connect(view, &IDOSRenderView::focused, this, &IDOSMainWindow::onRenderViewFocused);
    connect(view, &IDOSRenderView::decorationsChanged, this, &IDOSMainWindow::onRenderViewDecorations);
    connect(view, &IDOSRenderView::activated, this, &IDOSMainWindow::onRenderViewActivated);
    connect(view, &QObject::destroyed, this, &IDOSMainWindow::onRenderViewDestroyed);
    m_renderServer->addView(view);
    view->setTwoDimensional(twoDimensional);
    view->setOrientation(twoDimensional ? IDOSOrientation::Top : IDOSOrientation::Isometric);
    IDOSToolBar* toolBar = new IDOSToolBar();
    toolBar->setRegistry(m_toolRegistry);
    if (!view->setToolBar(toolBar))
    {
        delete toolBar;
    }
    if (m_activeViewId.isEmpty())
    {
        setActiveView(id);
    }
    return view;
}

void IDOSMainWindow::removeView(const QString& id)
{
    IDOSRenderView* view = m_views.take(id).data();
    if (view != nullptr)
    {
        disconnect(view, nullptr, this, nullptr);
        m_renderServer->removeView(view);
    }
    m_viewContextIds.remove(id);
    if (m_activeViewId == id)
    {
        m_activeViewId.clear();
        if (!m_views.isEmpty())
        {
            setActiveView(m_views.firstKey());
        }
        else if (m_actionViewPresets != nullptr)
        {
            onCurrentViewChanged(QString());
            onViewDecorationsChanged(false, false, false);
        }
    }
}

void IDOSMainWindow::setActiveView(const QString& id)
{
    if (!m_views.contains(id) || m_views.value(id) == nullptr || m_activeViewId == id)
    {
        return;
    }
    m_activeViewId = id;
    for (QMap<QString, QPointer<IDOSRenderView>>::const_iterator iterator = m_views.constBegin();
         iterator != m_views.constEnd(); ++iterator)
    {
        if (iterator.value() != nullptr)
        {
            iterator.value()->setActive(iterator.key() == id);
        }
    }
    activeView()->setSelection(m_renderServer->objectIds(activeView(), viewContextId()));
    if (m_dataTreeView != nullptr)
    {
        m_dataTreeView->setTargetView(activeView());
    }
    if (m_caseTreeView != nullptr)
    {
        m_caseTreeView->setTargetView(activeView());
    }
    if (m_actionViewPresets != nullptr)
    {
        onCurrentViewChanged(id);
        onViewDecorationsChanged(activeView()->axesVisible(), activeView()->legendAvailable(), activeView()->legendVisible());
    }
}

QString IDOSMainWindow::viewContextId() const
{
    return m_viewContextIds.value(m_activeViewId);
}

void IDOSMainWindow::setViewContextId(const QString& id)
{
    if (activeView() == nullptr || viewContextId() == id)
    {
        return;
    }
    m_viewContextIds.insert(m_activeViewId, id);
    activeView()->setSelection(m_renderServer->objectIds(activeView(), id));
    onActiveViewContextObjectChanged(m_activeViewId, id);
}

void IDOSMainWindow::onRenderViewFocused()
{
    IDOSRenderView* view = qobject_cast<IDOSRenderView*>(sender());
    if (view != nullptr)
    {
        setActiveView(view->property("viewId").toString());
    }
}

void IDOSMainWindow::onRenderViewDecorations()
{
    if (m_actionOrientationMarker != nullptr && activeView() != nullptr && sender() == activeView())
    {
        onViewDecorationsChanged(activeView()->axesVisible(), activeView()->legendAvailable(), activeView()->legendVisible());
    }
}

void IDOSMainWindow::onRenderViewActivated(const QString& id)
{
    IDOSRenderView* view = qobject_cast<IDOSRenderView*>(sender());
    IDOSDataObject* data = m_renderServer->dataSource(view, id);
    if (view != nullptr && data != nullptr)
    {
        onRenderObjectActivated(view->property("viewId").toString(), data->objectId());
    }
}

void IDOSMainWindow::onRenderViewDestroyed(QObject* object)
{
    removeView(object->property("viewId").toString());
}

void IDOSMainWindow::onRenderDataRemoving(const QString& id)
{
    const QStringList views = m_viewContextIds.keys();
    for (const QString& viewId : views)
    {
        if (m_viewContextIds.value(viewId) == id)
        {
            m_viewContextIds.remove(viewId);
        }
        if (m_viewCaseIds.value(viewId) == id)
        {
            m_viewCaseIds.remove(viewId);
        }
    }
}

void IDOSMainWindow::onRenderObjectsChanged(IDOSRenderView* view, const QStringList& ids, bool dataChanged)
{
    Q_UNUSED(dataChanged);
    if (m_project == nullptr || view == nullptr)
    {
        return;
    }
    ads::CDockWidget* dock = m_viewDocks.value(view->property("viewId").toString()).data();
    if (dock == nullptr)
    {
        return;
    }
    for (const QString& id : ids)
    {
        IDOSDataObject* data = m_project->object(id);
        if (data != nullptr && data->isVisible() && !m_renderServer->objectIds(view, id).isEmpty())
        {
            dock->setWindowTitle(tr("3D View - %1").arg(data->name()));
        }
    }
}

IDOSProject* IDOSMainWindow::project() const
{
    return m_project;
}

IDOSDataRegistry* IDOSMainWindow::dataRegistry() const
{
    return m_dataRegistry;
}

IDOSTreeRegistry* IDOSMainWindow::treeRegistry() const
{
    return m_treeRegistry;
}

IDOSRenderRegistry* IDOSMainWindow::renderRegistry() const
{
    return m_renderRegistry;
}

IDOSToolRegistry* IDOSMainWindow::toolRegistry() const
{
    return m_toolRegistry;
}

IDOSCommandRegistry* IDOSMainWindow::commandRegistry() const
{
    return m_assistantWidget->service()->commandRegistry();
}
