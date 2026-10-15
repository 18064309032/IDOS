#include <memory>

#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QScopedValueRollback>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QSignalBlocker>
#include <QSize>
#include <QStackedWidget>
#include <QStandardPaths>
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

#include "command/idoscasereferencecommand.h"
#include "command/idoscreatecasecommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosgridfiltercommand.h"
#include "command/idosimportcommands.h"
#include "command/idosimportrestartresultscommand.h"
#include "command/idosrenameobjectcommand.h"
#include "command/idostransferobjectscommand.h"
#include "data/case/idoscaseobject.h"
#include "data/well/idoswell.h"
#include "idosappinterface.h"
#include "idosapplication.h"
#include "idosassistantservice.h"
#include "idosassistantwidget.h"
#include "idoscommandmanager.h"
#include "idoscommandmetadata.h"
#include "idoscommandregistry.h"
#include "idosdebuginfowidget.h"
#include "idosnewprojectdialog.h"
#include "idospluginregistry.h"
#include "idosproject.h"
#include "idosprojectmetadata.h"
#include "idosprojectstartwidget.h"
#include "idospythonconsolewidget.h"
#include "idosrenderobject.h"
#include "idosrenderregistry.h"
#include "idosrenderscene.h"
#include "idosrenderserver.h"
#include "idosrendertypes.h"
#include "idosrenderview.h"
#include "idosruntimeinfowidget.h"
#include "idossimulation.h"
#include "idostoolbar.h"
#include "idostoolregistry.h"
#include "idostreeserver.h"
#include "logger/idoslogger.h"
#include "plugin/idospluginmanagerwidget.h"
#include "providers/case/idossimulationcasetreeguiprovider.h"
#include "providers/case/idossimulationcasetreemetadata.h"
#include "providers/data/idosimporteddatatreeguiprovider.h"
#include "providers/data/idosimporteddatatreemetadata.h"
#include "providers/fluidrock/idosfluidrocktabletreeguiprovider.h"
#include "providers/fluidrock/idosfluidrocktabletreemetadata.h"
#include "providers/grid/idosgridboundaryrendermetadata.h"
#include "providers/grid/idosgridfilterrendermetadata.h"
#include "providers/grid/idosgridrendermetadata.h"
#include "providers/grid/idosgridsurfacerendermetadata.h"
#include "providers/grid/idosgridtreeguiprovider.h"
#include "providers/grid/idosgridtreemetadata.h"
#include "providers/idosdataregistry.h"
#include "providers/idosrenderprovider.h"
#include "providers/idostreeregistry.h"
#include "providers/render/idosrendertoolmetadata.h"
#include "providers/seismic/idosseismicvolumerendermetadata.h"
#include "providers/well/idoswelldisplaytreeguiprovider.h"
#include "providers/well/idoswellrendermetadata.h"
#include "providers/well/idoswelltreeguiprovider.h"
#include "providers/well/idoswelltreemetadata.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"
#include "tree/idostreeview.h"
#include "well/idoswelllogtrackview.h"

#include "idosmainwindow.h"

IDOSMainWindow::IDOSMainWindow(QWidget* parent)
    :
    SARibbonMainWindow(parent)
    , m_actionNewProject(nullptr)
    , m_actionOpenProject(nullptr)
    , m_actionSaveProject(nullptr)
    , m_actionSaveProjectAs(nullptr)
    , m_actionCloseProject(nullptr)
    , m_actionProjectSettings(nullptr)
    , m_actionUndo(nullptr)
    , m_actionRedo(nullptr)
    , m_actionPluginManager(nullptr)
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
    , m_simulation(new IDOSSimulation(this))
    , m_debugInfoWidget(nullptr)
    , m_assistantWidget(nullptr)
    , m_pythonConsoleWidget(nullptr)
    , m_projectStatusLabel(nullptr)
    , m_projectStatusDot(nullptr)
    , m_viewStatusLabel(nullptr)
    , m_objectCountStatusLabel(nullptr)
    , m_dockManager(nullptr)
    , m_renderDock(nullptr)
    , m_centralPageStack(nullptr)
    , m_startPage(nullptr)
    , m_renderPage(nullptr)
    , m_outputDock(nullptr)
    , m_debugDock(nullptr)
    , m_pythonConsoleDock(nullptr)
    , m_assistantDock(nullptr)
    , m_viewCaseIds()
    , m_nextViewIndex(1)
    , m_currentWindowLayout(QStringLiteral("single"))
    , m_syncingTreeSelection(false)
    , m_restoringPresentation(true)
{
    IDOSApplication* application = qobject_cast<IDOSApplication*>(QApplication::instance());
    m_dataRegistry = application != nullptr ? application->dataRegistry() : new IDOSDataRegistry(this);
    if (application == nullptr)
    {
        IDOSApplication::initDataRegistry(m_dataRegistry);
    }
    initProviders();
    initMainWindow();
    initDockManager();
    initRenderView();
    initRibbonAction();

    initDockWidgets();
    m_simulation->initialize();
    IDOS_MESSAGE(tr("Application started."), IDOSLogLevel::Info);
    initPluginManager();
    const QList<IDOSTreeView*> trees{m_dataTreeView, m_caseTreeView};
    for (IDOSTreeView* tree : trees)
    {
        connect(tree, &QTreeView::expanded, this, &IDOSMainWindow::onPresentationChanged);
        connect(tree, &QTreeView::collapsed, this, &IDOSMainWindow::onPresentationChanged);
        connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &IDOSMainWindow::onPresentationChanged);
    }
    m_defaultViewState = activeView()->presentationState();
    restoreWindowPreferences(true);
    m_restoringPresentation = false;
    if (!qEnvironmentVariableIsSet("IDOS_DISABLE_ASSISTANT_AUTOSTART"))
    {
        QTimer::singleShot(0, m_assistantWidget, &IDOSAssistantWidget::startAssistant);
    }
}

void IDOSMainWindow::initProviders()
{
    IDOSApplication::initProviders();
    m_dataTree->setName(QStringLiteral("data"));
    m_caseTree->setName(QStringLiteral("case"));
    m_treeRegistry->add(new IDOSWellTreeMetadata());
    m_treeRegistry->add(new IDOSGridTreeMetadata());
    m_treeRegistry->add(new IDOSSimulationCaseTreeMetadata(m_treeRegistry));
    m_treeRegistry->add(new IDOSFluidRockTableTreeMetadata(IDOSFluidRockTable::Category::Fluid));
    m_treeRegistry->add(new IDOSFluidRockTableTreeMetadata(IDOSFluidRockTable::Category::RockPhysics));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.microseismic.set"), QStringLiteral("data.microseismic")));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.seismic.volume"), QStringLiteral("data.seismic")));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.fracture.schedule"), QStringLiteral("data.hydraulicFractures")));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.naturalFracture.set"), QStringLiteral("data.naturalFractures")));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.production.history"), QStringLiteral("data.production")));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.fracture.fluidCatalog"), QStringLiteral("data.fluids")));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.fracture.proppantCatalog"), QStringLiteral("data.hydraulicFractures")));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.grid.surface"), QStringLiteral("data.surfaceProperties")));
    m_treeRegistry->add(new IDOSImportedDataTreeMetadata(
        QStringLiteral("idos.grid.boundary"), QStringLiteral("data.boundaries")));
    m_treeRegistry->addGuiProvider(std::make_shared<IDOSWellTreeGuiProvider>());
    m_treeRegistry->addGuiProvider(std::make_shared<IDOSWellDisplayTreeGuiProvider>(this));
    m_toolRegistry->add(new IDOSRenderToolMetadata());
    m_treeRegistry->addGuiProvider(std::make_shared<IDOSGridTreeGuiProvider>());
    const std::shared_ptr<IDOSSimulationCaseTreeGuiProvider> caseGuiProvider =
        std::make_shared<IDOSSimulationCaseTreeGuiProvider>();
    connect(caseGuiProvider.get(), &IDOSSimulationCaseTreeGuiProvider::generateInputRequested,
            m_simulation, &IDOSSimulation::onGenerateInputRequested);
    connect(caseGuiProvider.get(), &IDOSSimulationCaseTreeGuiProvider::propertyCurvesRequested,
            m_simulation, &IDOSSimulation::onPropertyCurvesRequested);
    m_treeRegistry->addGuiProvider(caseGuiProvider);
    m_treeRegistry->addGuiProvider(std::make_shared<IDOSFluidRockTableTreeGuiProvider>());
    m_treeRegistry->addGuiProvider(std::make_shared<IDOSImportedDataTreeGuiProvider>());
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
    m_renderRegistry->add(new IDOSGridFilterRenderMetadata());
    m_renderRegistry->add(new IDOSGridSurfaceRenderMetadata());
    m_renderRegistry->add(new IDOSGridBoundaryRenderMetadata());
    m_renderRegistry->add(new IDOSSeismicVolumeRenderMetadata());
    m_renderRegistry->add(new IDOSWellRenderMetadata());
    connect(m_renderServer, &IDOSRenderServer::objectsChanged, this, &IDOSMainWindow::onRenderObjectsChanged);
    connect(m_renderServer, &IDOSRenderServer::dataAboutToBeRemoved, this, &IDOSMainWindow::onRenderDataRemoving);
    connect(m_renderServer, &IDOSRenderServer::currentViewChanged, this,
            &IDOSMainWindow::onRenderCurrentViewChanged);
}

void IDOSMainWindow::initMainWindow()
{
    setWindowTitle(tr("IDOS"));
    setWindowIcon(QIcon(":/images/app-logo.svg"));
    statusBar()->setStyleSheet(QStringLiteral(
        "QStatusBar { background-color: #F7F9FC; border-top: 1px solid #D6DEE8; }"
        "QStatusBar::item { border: 0; }"
        "QLabel#statusIndicator { color: #43556B; padding: 0 12px; font-size: 9pt; }"
        "QLabel#statusProject { color: #263B53; font-weight: 600; padding-left: 0; }"
        "QLabel#projectStatusDot { min-width: 8px; max-width: 8px; min-height: 8px; "
        "max-height: 8px; border-radius: 4px; background-color: #94A3B8; }"
        "QFrame#statusSeparator { color: #D5DEE8; max-width: 1px; margin: 3px 2px; }"));
    statusBar()->showMessage(tr("Ready"));
    m_projectStatusLabel = new QLabel(statusBar());
    m_projectStatusLabel->setObjectName(QStringLiteral("statusProject"));
    m_projectStatusDot = new QLabel(statusBar());
    m_projectStatusDot->setObjectName(QStringLiteral("projectStatusDot"));
    QWidget* projectStatusWidget = new QWidget(statusBar());
    QHBoxLayout* projectStatusLayout = new QHBoxLayout(projectStatusWidget);
    projectStatusLayout->setContentsMargins(10, 0, 10, 0);
    projectStatusLayout->setSpacing(8);
    projectStatusLayout->addWidget(m_projectStatusDot);
    projectStatusLayout->addWidget(m_projectStatusLabel);
    m_viewStatusLabel = new QLabel(statusBar());
    m_viewStatusLabel->setObjectName(QStringLiteral("statusIndicator"));
    m_objectCountStatusLabel = new QLabel(statusBar());
    m_objectCountStatusLabel->setObjectName(QStringLiteral("statusIndicator"));
    QFrame* projectSeparator = new QFrame(statusBar());
    projectSeparator->setObjectName(QStringLiteral("statusSeparator"));
    projectSeparator->setFrameShape(QFrame::VLine);
    projectSeparator->setFrameShadow(QFrame::Plain);
    QFrame* viewSeparator = new QFrame(statusBar());
    viewSeparator->setObjectName(QStringLiteral("statusSeparator"));
    viewSeparator->setFrameShape(QFrame::VLine);
    viewSeparator->setFrameShadow(QFrame::Plain);
    statusBar()->addPermanentWidget(projectStatusWidget);
    statusBar()->addPermanentWidget(projectSeparator);
    statusBar()->addPermanentWidget(m_viewStatusLabel);
    statusBar()->addPermanentWidget(viewSeparator);
    statusBar()->addPermanentWidget(m_objectCountStatusLabel);
    updateStatusBarIndicators();
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
    m_renderPage = createView(QStringLiteral("main3d"), false);
    if (m_renderPage == nullptr)
    {
        return;
    }
    m_renderDock = new ads::CDockWidget(m_dockManager, tr("3D View"));
    m_renderDock->setObjectName(QStringLiteral("main3DDock"));
    m_renderDock->setIcon(QIcon(QStringLiteral(":/images/render-view.svg")));
    m_renderDock->setFeatures(ads::CDockWidget::NoDockWidgetFeatures);
    m_renderPage->setWindowTitle(m_renderDock->windowTitle());
    connect(m_renderDock, &QWidget::windowTitleChanged, m_renderPage, &QWidget::setWindowTitle);

    m_centralPageStack = new QStackedWidget(m_renderDock);
    m_startPage = new IDOSProjectStartWidget(m_centralPageStack);
    connect(m_startPage, &IDOSProjectStartWidget::newProjectRequested,
            this, &IDOSMainWindow::onNewProject);
    connect(m_startPage, &IDOSProjectStartWidget::openProjectRequested,
            this, &IDOSMainWindow::onOpenProject);
    connect(m_startPage, &IDOSProjectStartWidget::recentProjectOpenRequested,
            this, &IDOSMainWindow::onStartPageRecentProjectOpenRequested);
    connect(m_startPage, &IDOSProjectStartWidget::recentProjectRemoveRequested,
            this, &IDOSMainWindow::onStartPageRecentProjectRemoveRequested);
    m_centralPageStack->addWidget(m_startPage);
    m_centralPageStack->addWidget(m_renderPage);
    m_renderDock->setWidget(m_centralPageStack);
    m_dockManager->setCentralWidget(m_renderDock);
    m_viewDocks.insert(QStringLiteral("main3d"), m_renderDock);
    updateCentralPage();
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
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCaseReferenceCommandMetadata(QStringLiteral("copy"))));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCaseReferenceCommandMetadata(QStringLiteral("move"))));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCaseReferenceCommandMetadata(QStringLiteral("remove"))));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateWellCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteGridCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSGridFilterCommandMetadata(QStringLiteral("create"))));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSGridFilterCommandMetadata(QStringLiteral("update"))));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSGridFilterCommandMetadata(QStringLiteral("delete"))));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeletePropertyCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteWellCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSTransferObjectsCommandMetadata(QStringLiteral("copy"))));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSTransferObjectsCommandMetadata(QStringLiteral("move"))));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSRenameObjectCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportRestartResultsCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportGridCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportPropertyCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportWellDataCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportFluidRockTablesCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSAttachFluidRockTableCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportProjectDataCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportSummaryResultsCommandMetadata()));
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
    caseDock->setWidget(m_caseTreeView, ads::CDockWidget::ForceNoScrollArea);
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
    projectPage->setObjectName(QStringLiteral("idosHomeCategory"));
    SARibbonPanel* projectPanel = projectPage->addPanel(tr("Project"));
    projectPanel->addLargeAction(m_actionNewProject);
    m_actionOpenProject = new QAction(
        QIcon(QStringLiteral(":/images/app-project-open.svg")), tr("Open Project"), this);
    m_actionOpenProject->setObjectName(QStringLiteral("openProjectAction"));
    m_actionOpenProject->setShortcut(QKeySequence::Open);
    connect(m_actionOpenProject, &QAction::triggered, this, &IDOSMainWindow::onOpenProject);
    updateRecentProjects();
    projectPanel->addLargeAction(m_actionOpenProject);
    m_actionSaveProject = new QAction(
        QIcon(QStringLiteral(":/images/app-project-save.svg")), tr("Save Project"), this);
    m_actionSaveProject->setObjectName(QStringLiteral("saveProjectAction"));
    m_actionSaveProject->setEnabled(false);
    m_actionSaveProject->setShortcut(QKeySequence::Save);
    connect(m_actionSaveProject, &QAction::triggered, this, &IDOSMainWindow::onSaveProject);
    projectPanel->addLargeAction(m_actionSaveProject);
    SARibbonPanel* projectManagementPanel = projectPage->addPanel(tr("Project Management"));
    m_actionSaveProjectAs = new QAction(
        QIcon(QStringLiteral(":/images/app-project-save.svg")), tr("Save Project As"), this);
    m_actionSaveProjectAs->setObjectName(QStringLiteral("saveProjectAsAction"));
    m_actionSaveProjectAs->setEnabled(false);
    m_actionSaveProjectAs->setShortcut(QKeySequence::SaveAs);
    connect(m_actionSaveProjectAs, &QAction::triggered, this, &IDOSMainWindow::onSaveProjectAs);
    projectManagementPanel->addSmallAction(m_actionSaveProjectAs);
    m_actionCloseProject = new QAction(
        QIcon(QStringLiteral(":/images/app-project-close.svg")), tr("Close Project"), this);
    m_actionCloseProject->setObjectName(QStringLiteral("closeProjectAction"));
    m_actionCloseProject->setEnabled(false);
    connect(m_actionCloseProject, &QAction::triggered, this, &IDOSMainWindow::onCloseProject);
    projectManagementPanel->addSmallAction(m_actionCloseProject);
    m_actionProjectSettings = new QAction(
        QIcon(QStringLiteral(":/images/app-project-settings.svg")), tr("Project Settings"), this);
    m_actionProjectSettings->setObjectName(QStringLiteral("projectSettingsAction"));
    m_actionProjectSettings->setEnabled(false);
    projectManagementPanel->addSmallAction(m_actionProjectSettings);
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
    fileMenu->addAction(m_actionCloseProject);
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

    SARibbonPanel* simulationPanel = projectPage->addPanel(tr("Simulation"));
    simulationPanel->setObjectName(QStringLiteral("idosHomePanel_simulation"));
    SARibbonPanel* resultsPanel = projectPage->addPanel(tr("Results"));
    resultsPanel->setObjectName(QStringLiteral("idosHomePanel_results"));
    SARibbonPanel* outputPanel = projectPage->addPanel(tr("Output"));
    outputPanel->setObjectName(QStringLiteral("idosHomePanel_output"));
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
}

IDOSMainWindow::~IDOSMainWindow()
{
    delete m_simulation;
    m_simulation = nullptr;
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
    m_pluginRegistry->unloadAll(true);
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
    if (m_commandManager != nullptr)
    {
        disconnect(m_commandManager.data(), nullptr, this, nullptr);
    }
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
    if (m_commandManager != nullptr)
    {
        disconnect(m_commandManager.data(), nullptr, this, nullptr);
    }
    QScopedValueRollback<bool> presentationGuard(m_restoringPresentation, true);
    m_project = nullptr;
    m_renderServer->setProject(nullptr);
    const QVariantMap presentation = project != nullptr ? project->presentationState() : QVariantMap();
    prepareProjectViews(presentation);
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
    restoreProjectPresentation(presentation);
    if (project)
    {
        connect(project, &QObject::destroyed, this, &IDOSMainWindow::onProjectDestroyed);
        connect(project, &IDOSProject::modifiedChanged, this, &IDOSMainWindow::onProjectModified);
        connect(project, &IDOSProject::metadataChanged, this, &IDOSMainWindow::onProjectModified);
        connect(project, &IDOSProject::objectAdded, this, &IDOSMainWindow::updateStatusBarIndicators);
        connect(project, &IDOSProject::objectRemoved, this, &IDOSMainWindow::updateStatusBarIndicators);
        connect(project, &IDOSProject::objectsAdded, this, &IDOSMainWindow::updateStatusBarIndicators);
        connect(project, &IDOSProject::objectsRemoved, this, &IDOSMainWindow::updateStatusBarIndicators);
        if (m_commandManager != nullptr)
        {
            connect(m_commandManager.data(), &IDOSCommandManager::stateChanged,
                    this, &IDOSMainWindow::onCommandStateChanged);
        }
    }

    onCommandStateChanged();
    onProjectModified();
    updateCentralPage();
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
    collectProjectPresentation();
    if (!m_project || !m_project->isModified())
    {
        return true;
    }
    QMessageBox warning(
        QMessageBox::Warning, tr("Unsaved Project"),
        tr("Save changes to the current project before continuing?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
    warning.button(QMessageBox::Save)->setText(tr("Save"));
    warning.button(QMessageBox::Discard)->setText(tr("Discard"));
    warning.button(QMessageBox::Cancel)->setText(tr("Cancel"));
    warning.setDefaultButton(QMessageBox::Cancel);
    const int decision = warning.exec();
    return decision == QMessageBox::Discard || (decision == QMessageBox::Save && saveProject());
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


void IDOSMainWindow::onCurrentViewChanged(const QString& viewId)
{
    const bool hasActiveView = activeView() != nullptr;
    m_actionCaptureScreenshot->setEnabled(hasActiveView);
    m_actionExportImage->setEnabled(hasActiveView);
    updateTreeSelection(viewId);
}


void IDOSMainWindow::onActiveViewContextObjectChanged(const QString& viewId,
                                                       const QString& objectId)
{
    Q_UNUSED(objectId)
    if (!m_syncingTreeSelection && viewId == activeViewId())
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

    onPresentationChanged();
    m_currentWindowLayout = action->data().toString();
    applyWindowLayout(m_currentWindowLayout);
    if (m_currentWindowLayout == QStringLiteral("single"))
    {
        setActiveView(QStringLiteral("main3d"));
    }
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
    renderViewWidget->setWindowTitle(dock->windowTitle());
    connect(dock, &QWidget::windowTitleChanged, renderViewWidget, &QWidget::setWindowTitle);
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
    QScopedValueRollback<bool> selectionGuard(m_syncingTreeSelection, true);

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
        const QString caseId = m_project != nullptr ? m_project->currentCaseId() : QString();
        setViewTreeContext(viewId, objectId, caseId);
        updateTreeSelection(viewId, m_dataTreeView);
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
    m_project->setCurrentCaseId(caseIdFromCaseTreeIndex(index));
}

void IDOSMainWindow::onCaseTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    if (m_syncingTreeSelection)
    {
        return;
    }
    QScopedValueRollback<bool> selectionGuard(m_syncingTreeSelection, true);

    if (m_project != nullptr)
    {
        m_project->setCurrentObject(m_caseTreeModel->objectFromIndex(current));
        m_project->setCurrentCaseId(caseIdFromCaseTreeIndex(current));
    }

    const QString viewId = activeViewId();
    if (viewId.isEmpty())
    {
        return;
    }

    const QString objectId = objectIdFromCaseTreeIndex(current);
    const QString caseId = caseIdFromCaseTreeIndex(current);
    setViewTreeContext(viewId, objectId, caseId);
    updateTreeSelection(viewId, m_caseTreeView);
}

void IDOSMainWindow::onRenderObjectActivated(const QString& viewId, const QString& objectId)
{
    setActiveView(viewId);
    if (m_project != nullptr)
    {
        m_project->setCurrentObject(m_project->object(objectId));
    }
    const QModelIndex caseIndex = caseTreeIndexForContext(
        objectId, m_project != nullptr ? m_project->currentCaseId() : QString());
    const QString caseId = caseIdFromCaseTreeIndex(caseIndex);
    if (m_project != nullptr)
    {
        m_project->setCurrentCaseId(caseId);
    }
    setViewTreeContext(viewId, objectId, caseId);
    updateTreeSelection(viewId);
}

void IDOSMainWindow::updateTreeSelection(const QString& viewId, QTreeView* source)
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
    QScopedValueRollback<bool> selectionGuard(m_syncingTreeSelection, true);
    if (source != m_dataTreeView)
    {
        selectTreeIndex(m_dataTreeView, dataIndex);
    }
    if (source != m_caseTreeView)
    {
        selectTreeIndex(m_caseTreeView, caseIndex);
    }
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
    connect(view, &IDOSWellLogTrackView::presentationChanged, this, &IDOSMainWindow::onPresentationChanged);
    connect(dock, &ads::CDockWidget::closed, this, &IDOSMainWindow::onPresentationChanged);
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
    saveWindowPreferences();
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
    onProjectModified();
    updateStatusBarIndicators();
    updateCentralPage();
    emit projectChanged(nullptr);
    updateTreeSelection(activeViewId());
    onCommandStateChanged();
}

IDOSRenderView* IDOSMainWindow::activeView() const
{
    return m_renderServer->currentView();
}

QString IDOSMainWindow::activeViewId() const
{
    IDOSRenderView* view = activeView();
    return view != nullptr ? view->property("viewId").toString() : QString();
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
    connect(view, &IDOSRenderView::viewStateChanged, this, &IDOSMainWindow::onPresentationChanged);
    connect(view->scene(), &IDOSRenderScene::changed, this, &IDOSMainWindow::onPresentationChanged);
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
    onPresentationChanged();
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
    m_viewCaseIds.remove(id);
    onPresentationChanged();
}

void IDOSMainWindow::setActiveView(const QString& id)
{
    if (!m_views.contains(id) || m_views.value(id) == nullptr)
    {
        return;
    }
    m_renderServer->setCurrentView(m_views.value(id));
}

void IDOSMainWindow::onRenderCurrentViewChanged(IDOSRenderView* view)
{
    onPresentationChanged();
    updateStatusBarIndicators();
    const QString viewId = view != nullptr ? view->property("viewId").toString() : QString();
    if (m_project != nullptr)
    {
        if (!m_project->setCurrentCaseId(m_viewCaseIds.value(viewId)))
        {
            m_viewCaseIds.remove(viewId);
            m_project->setCurrentCaseId(QString());
        }
        m_project->setCurrentObject(m_project->object(m_viewContextIds.value(viewId)));
    }
    if (m_dataTreeView != nullptr)
    {
        m_dataTreeView->setTargetView(view);
    }
    if (m_caseTreeView != nullptr)
    {
        m_caseTreeView->setTargetView(view);
    }
    if (m_actionCaptureScreenshot != nullptr)
    {
        onCurrentViewChanged(activeViewId());
    }
}

QString IDOSMainWindow::viewContextId() const
{
    return m_viewContextIds.value(activeViewId());
}

void IDOSMainWindow::setViewContextId(const QString& id)
{
    if (activeView() == nullptr || viewContextId() == id)
    {
        return;
    }
    m_viewContextIds.insert(activeViewId(), id);
    onActiveViewContextObjectChanged(activeViewId(), id);
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
    if (sender() == activeView())
    {
        if (m_dataTreeView != nullptr)
        {
            m_dataTreeView->refreshCheckStates();
        }
        if (m_caseTreeView != nullptr)
        {
            m_caseTreeView->refreshCheckStates();
        }
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

IDOSRenderServer* IDOSMainWindow::renderServer() const
{
    return m_renderServer;
}

IDOSToolRegistry* IDOSMainWindow::toolRegistry() const
{
    return m_toolRegistry;
}

IDOSCommandRegistry* IDOSMainWindow::commandRegistry() const
{
    return m_assistantWidget->service()->commandRegistry();
}

SARibbonPanel* IDOSMainWindow::homeRibbonPanel(const QString& groupId) const
{
    if (ribbonBar() == nullptr)
    {
        return nullptr;
    }
    SARibbonCategory* home = ribbonBar()->categoryByObjectName(QStringLiteral("idosHomeCategory"));
    if (home == nullptr)
    {
        return nullptr;
    }
    return home->panelByObjectName(QStringLiteral("idosHomePanel_") + groupId);
}
void IDOSMainWindow::onProjectModified()
{
    m_actionSaveProject->setEnabled(m_project != nullptr);
    m_actionSaveProjectAs->setEnabled(m_project != nullptr);
    m_actionCloseProject->setEnabled(m_project != nullptr);
    const QString projectName = m_project != nullptr ? m_project->metadata().name().trimmed() : QString();
    const bool modified = m_project != nullptr && m_project->isModified();
    QString title = projectName.isEmpty() ? tr("IDOS") : projectName;
    if (modified)
    {
        title.append(QLatin1Char('*'));
    }
    setWindowTitle(title);
    setWindowModified(modified);
    updateStatusBarIndicators();
}

void IDOSMainWindow::updateStatusBarIndicators()
{
    if (m_projectStatusLabel == nullptr ||
        m_viewStatusLabel == nullptr ||
        m_objectCountStatusLabel == nullptr)
    {
        return;
    }

    const bool hasProject = m_project != nullptr;
    const QString projectName = hasProject
        ? m_project->metadata().name().trimmed()
        : QString();
    const QString projectStatus = !hasProject
        ? tr("No Project")
        : QStringLiteral("%1  ·  %2")
              .arg(projectName.isEmpty() ? tr("Untitled Project") : projectName,
                   m_project->isModified() ? tr("Modified") : tr("Saved"));
    m_projectStatusLabel->setText(projectStatus);
    m_projectStatusLabel->setToolTip(hasProject ? m_project->filePath() : tr("No Project Open"));
    const QString projectDotColor = !hasProject
        ? QStringLiteral("#94A3B8")
        : (m_project->isModified() ? QStringLiteral("#D97706") : QStringLiteral("#15966A"));
    m_projectStatusDot->setStyleSheet(QStringLiteral(
        "QLabel#projectStatusDot { background-color: %1; }").arg(projectDotColor));

    IDOSRenderView* view = activeView();
    const QString viewStatus = view == nullptr
        ? tr("No Active View")
        : (view->isTwoDimensional() ? tr("2D View") : tr("3D View"));
    m_viewStatusLabel->setText(viewStatus);
    m_viewStatusLabel->setToolTip(tr("Active render view mode"));

    const int objectCount = hasProject ? m_project->objects().size() : 0;
    m_objectCountStatusLabel->setText(tr("Objects: %1").arg(objectCount));
    m_objectCountStatusLabel->setToolTip(tr("Number of data objects in the active project"));
}

bool IDOSMainWindow::saveProject(const QString& path)
{
    QPointer<IDOSProject> target(m_project);
    if (target == nullptr)
    {
        return false;
    }
    QString destination = path.isEmpty() ? target->filePath() : path;
    if (destination.isEmpty())
    {
        destination = QFileDialog::getSaveFileName(this, tr("Save Project"), QString(), tr("IDOS Projects (*.idos)"));
    }
    if (destination.isEmpty() || target == nullptr || target.data() != m_project)
    {
        return false;
    }
    if (QFileInfo(destination).suffix().isEmpty())
    {
        destination += QStringLiteral(".idos");
    }
    collectProjectPresentation(true);
    QString error;
    if (!target->save(destination, m_dataRegistry, error))
    {
        QMessageBox::warning(this, tr("Save Project"), error);
        return false;
    }
    onProjectModified();
    saveWindowPreferences();
    rememberProject(destination);
    IDOS_MESSAGE(tr("Project saved: %1").arg(destination), IDOSLogLevel::Info);
    return true;
}

bool IDOSMainWindow::openProject(const QString& path, bool confirmDiscard)
{
    QString error;
    IDOSProject* loaded = IDOSProject::load(path, m_dataRegistry, error);
    if (loaded == nullptr)
    {
        QMessageBox::warning(this, tr("Open Project"), error);
        return false;
    }
    if (confirmDiscard && !confirmDiscardProject())
    {
        delete loaded;
        return false;
    }
    QElapsedTimer displayTiming;
    displayTiming.start();
    IDOSProject* previous = m_project;
    loaded->setParent(this);
    setProject(loaded);
    rememberProject(path);
    if (previous != nullptr && previous->parent() == this)
    {
        previous->deleteLater();
    }
    IDOS_DEBUG(tr("Project display setup completed in %1 ms.").arg(displayTiming.elapsed()));
    IDOS_MESSAGE(tr("Project opened: %1").arg(path), IDOSLogLevel::Info);
    return true;
}

void IDOSMainWindow::onOpenProject()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Open Project"), QString(), tr("IDOS Projects (*.idos)"));
    if (!path.isEmpty())
    {
        openProject(path);
    }
}

void IDOSMainWindow::onSaveProject()
{
    saveProject();
}

void IDOSMainWindow::onSaveProjectAs()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("Save Project As"),
        m_project != nullptr ? m_project->filePath() : QString(), tr("IDOS Projects (*.idos)"));
    if (!path.isEmpty())
    {
        saveProject(path);
    }
}

void IDOSMainWindow::onCloseProject()
{
    if (m_project == nullptr || !confirmDiscardProject())
    {
        return;
    }

    IDOSProject* projectToClose = m_project;
    setProject(nullptr);
    if (projectToClose->parent() == this)
    {
        projectToClose->deleteLater();
    }
}

void IDOSMainWindow::onPresentationChanged()
{
    if (!m_restoringPresentation && m_project != nullptr)
    {
        m_project->markPresentationModified();
    }
}

void IDOSMainWindow::collectProjectPresentation(bool force)
{
    if (m_project == nullptr || m_restoringPresentation)
    {
        return;
    }
    QVariantMap state;
    state.insert(QStringLiteral("version"), 1);
    state.insert(QStringLiteral("render"), m_renderServer->presentationState());
    state.insert(QStringLiteral("dataTree"), m_dataTreeView->presentationState());
    state.insert(QStringLiteral("caseTree"), m_caseTreeView->presentationState());
    state.insert(QStringLiteral("windowLayout"), m_currentWindowLayout);
    QVariantMap closedViews;
    for (const QString& id : m_viewDocks.keys())
    {
        ads::CDockWidget* dock = m_viewDocks.value(id).data();
        if (dock != nullptr)
        {
            closedViews.insert(id, dock->isClosed());
        }
    }
    state.insert(QStringLiteral("closedViews"), closedViews);
    ads::CDockWidget* focused = m_dockManager->focusedDockWidget();
    ads::CDockWidget* currentDock = m_viewDocks.value(activeViewId()).data();
    state.insert(QStringLiteral("activeDock"), focused != nullptr ? focused->objectName()
        : currentDock != nullptr ? currentDock->objectName() : QString());
    QVariantMap contexts;
    for (const QString& id : m_views.keys())
    {
        QVariantMap context;
        context.insert(QStringLiteral("dataId"), m_viewContextIds.value(id));
        context.insert(QStringLiteral("caseId"), m_viewCaseIds.value(id));
        contexts.insert(id, context);
    }
    state.insert(QStringLiteral("contexts"), contexts);
    QMap<QString, QVariant> logViews;
    for (IDOSWellLogTrackView* view : logTrackViews())
    {
        ads::CDockWidget* dock = m_dockManager->findDockWidget(
            QStringLiteral("wellLogTrackDock_%1").arg(view->presentationState().value(QStringLiteral("wellId")).toString()));
        if (dock != nullptr && !dock->isClosed())
        {
            const QVariantMap saved = view->presentationState();
            logViews.insert(saved.value(QStringLiteral("wellId")).toString(), saved);
        }
    }
    state.insert(QStringLiteral("logViews"), logViews.values());
    // An untouched old project should not become dirty solely because it lacks a UI snapshot.
    if (force || !m_project->presentationState().isEmpty() || m_project->isModified())
    {
        m_project->setPresentationState(state);
    }
}

void IDOSMainWindow::prepareProjectViews(const QVariantMap& state)
{
    for (IDOSWellLogTrackView* view : logTrackViews())
    {
        const QString wellId = view->presentationState().value(QStringLiteral("wellId")).toString();
        ads::CDockWidget* dock = m_dockManager->findDockWidget(QStringLiteral("wellLogTrackDock_%1").arg(wellId));
        if (dock != nullptr)
        {
            m_dockManager->removeDockWidget(dock);
            delete dock;
        }
    }
    const QStringList previousIds = m_views.keys();
    for (const QString& id : previousIds)
    {
        if (id == QStringLiteral("main3d"))
        {
            continue;
        }
        ads::CDockWidget* dock = m_viewDocks.take(id).data();
        removeView(id);
        if (dock != nullptr)
        {
            disconnect(dock, nullptr, this, nullptr);
            m_dockManager->removeDockWidget(dock);
            delete dock;
        }
    }
    m_nextViewIndex = 1;
    IDOSRenderView* main = m_views.value(QStringLiteral("main3d")).data();
    if (main != nullptr && !m_defaultViewState.isEmpty())
    {
        main->restorePresentationState(m_defaultViewState);
    }
    m_currentWindowLayout = QStringLiteral("single");
    const QVariantList views = state.value(QStringLiteral("version")).toInt() == 1
        ? state.value(QStringLiteral("render")).toMap().value(QStringLiteral("views")).toList() : QVariantList();
    int count = 0;
    for (const QVariant& value : views)
    {
        const QVariantMap entry = value.toMap();
        const QString id = entry.value(QStringLiteral("id")).toString();
        if (m_views.contains(id) || !id.startsWith(QStringLiteral("view_")) || ++count > 32)
        {
            continue;
        }
        bool valid = false;
        const int index = id.mid(5).toInt(&valid);
        if (!valid || index < 1 || index > 100000 || id != QStringLiteral("view_%1").arg(index, 4, 10, QLatin1Char('0')))
        {
            continue;
        }
        m_nextViewIndex = index;
        createRenderView(entry.value(QStringLiteral("view")).toMap().value(QStringLiteral("twoDimensional")).toBool());
    }
    for (const QString& id : m_views.keys())
    {
        if (id.startsWith(QStringLiteral("view_")))
        {
            m_nextViewIndex = qMax(m_nextViewIndex, id.mid(5).toInt() + 1);
        }
    }
    setActiveView(QStringLiteral("main3d"));
}

void IDOSMainWindow::restoreProjectPresentation(const QVariantMap& state)
{
    if (state.value(QStringLiteral("version")).toInt() == 1)
    {
        const QVariantMap contexts = state.value(QStringLiteral("contexts")).toMap();
        for (const QString& id : m_views.keys())
        {
            const QVariantMap context = contexts.value(id).toMap();
            const QString dataId = context.value(QStringLiteral("dataId")).toString();
            const QString caseId = context.value(QStringLiteral("caseId")).toString();
            if (m_project != nullptr && m_project->object(dataId) != nullptr)
            {
                m_viewContextIds.insert(id, dataId);
            }
            if (m_project != nullptr && m_project->object(caseId) != nullptr)
            {
                m_viewCaseIds.insert(id, caseId);
            }
        }
        const QVariantList logViews = state.value(QStringLiteral("logViews")).toList();
        int logCount = 0;
        for (const QVariant& value : logViews)
        {
            const QVariantMap saved = value.toMap();
            IDOSWell* well = m_project != nullptr ? qobject_cast<IDOSWell*>(m_project->object(saved.value(QStringLiteral("wellId")).toString())) : nullptr;
            if (well != nullptr && ++logCount <= 32)
            {
                findOrCreateWellLogTrackView(well)->restorePresentationState(saved);
            }
        }
        m_currentWindowLayout = state.value(QStringLiteral("windowLayout"), QStringLiteral("single")).toString();
        const QStringList layouts{QStringLiteral("single"), QStringLiteral("side-by-side"), QStringLiteral("stacked"),
            QStringLiteral("grid:1:2"), QStringLiteral("grid:2:1"), QStringLiteral("grid:2:2"),
            QStringLiteral("grid:2:3"), QStringLiteral("grid:3:2"), QStringLiteral("grid:3:3")};
        if (!layouts.contains(m_currentWindowLayout))
        {
            m_currentWindowLayout = QStringLiteral("single");
        }
        applyWindowLayout(m_currentWindowLayout);
        restoreWindowPreferences(false);
        m_renderServer->restorePresentationState(state.value(QStringLiteral("render")).toMap());
        onRenderCurrentViewChanged(activeView());
        const QVariantMap closedViews = state.value(QStringLiteral("closedViews")).toMap();
        for (const QString& id : closedViews.keys())
        {
            ads::CDockWidget* dock = m_viewDocks.value(id).data();
            if (dock != nullptr)
            {
                dock->toggleView(!closedViews.value(id).toBool());
            }
        }
        ads::CDockWidget* activeDock = m_dockManager->findDockWidget(state.value(QStringLiteral("activeDock")).toString());
        if (activeDock == nullptr || activeDock->isClosed())
        {
            activeDock = m_viewDocks.value(activeViewId()).data();
        }
        if (activeDock != nullptr)
        {
            activeDock->toggleView(true);
            activeDock->setAsCurrentTab();
            m_dockManager->setDockWidgetFocused(activeDock);
        }
        QScopedValueRollback<bool> selectionGuard(m_syncingTreeSelection, true);
        m_dataTreeView->restorePresentationState(state.value(QStringLiteral("dataTree")).toMap());
        m_caseTreeView->restorePresentationState(state.value(QStringLiteral("caseTree")).toMap());
    }
    else
    {
        m_dataTreeView->expandToDepth(0);
        m_caseTreeView->expandToDepth(0);
        if (m_project != nullptr)
        {
            m_viewCaseIds.insert(activeViewId(), m_project->currentCaseId());
            if (m_project->currentObject() != nullptr)
            {
                m_viewContextIds.insert(activeViewId(), m_project->currentObject()->objectId());
            }
        }
        applyWindowLayout(m_currentWindowLayout);
        restoreWindowPreferences(false);
    }
    m_dataTreeView->refreshCheckStates();
    m_caseTreeView->refreshCheckStates();
}

QString IDOSMainWindow::windowSettingsPath()
{
    const QString overridePath = qEnvironmentVariable("IDOS_UI_SETTINGS_PATH");
    if (!overridePath.isEmpty())
    {
        return overridePath;
    }
    if (qEnvironmentVariableIsSet("IDOS_DISABLE_UI_SETTINGS"))
    {
        return QString();
    }
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(directory);
    return directory + QStringLiteral("/ui.ini");
}

QString IDOSMainWindow::layoutSettingsKey() const
{
    QStringList ids = m_views.keys();
    for (IDOSWellLogTrackView* view : logTrackViews())
    {
        ids.append(QStringLiteral("log_") + view->presentationState().value(QStringLiteral("wellId")).toString());
    }
    ids.sort();
    return QStringLiteral("dockLayouts/") + ids.join(QLatin1Char('_'));
}

void IDOSMainWindow::saveWindowPreferences()
{
    const QString path = windowSettingsPath();
    if (path.isEmpty())
    {
        return;
    }
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue(QStringLiteral("window/geometry"), saveGeometry());
    settings.setValue(QStringLiteral("window/state"), saveState(1));
    settings.setValue(layoutSettingsKey(), m_dockManager->saveState(1));
    settings.sync();
}

void IDOSMainWindow::restoreWindowPreferences(bool geometry)
{
    const QString path = windowSettingsPath();
    if (path.isEmpty())
    {
        return;
    }
    QSettings settings(path, QSettings::IniFormat);
    if (geometry)
    {
        restoreGeometry(settings.value(QStringLiteral("window/geometry")).toByteArray());
        restoreState(settings.value(QStringLiteral("window/state")).toByteArray(), 1);
        bool visibleOnScreen = false;
        for (QScreen* screen : QGuiApplication::screens())
        {
            if (screen->availableGeometry().intersects(frameGeometry()))
            {
                visibleOnScreen = true;
                break;
            }
        }
        if (!visibleOnScreen && QGuiApplication::primaryScreen() != nullptr)
        {
            const QRect available = QGuiApplication::primaryScreen()->availableGeometry();
            resize(size().boundedTo(available.size()));
            move(available.center() - rect().center());
        }
    }
    const QByteArray layout = settings.value(layoutSettingsKey()).toByteArray();
    if (!layout.isEmpty())
    {
        m_dockManager->restoreState(layout, 1);
    }
}

void IDOSMainWindow::rememberProject(const QString& projectPath)
{
    const QString settingsPath = windowSettingsPath();
    if (settingsPath.isEmpty())
    {
        return;
    }

    QSettings settings(settingsPath, QSettings::IniFormat);
    QStringList recent = settings.value(QStringLiteral("recentProjects")).toStringList();
    const QString absolutePath = QFileInfo(projectPath).absoluteFilePath();
    recent.removeAll(absolutePath);
    recent.prepend(absolutePath);
    while (recent.size() > 10)
    {
        recent.removeLast();
    }
    settings.setValue(QStringLiteral("recentProjects"), recent);
    settings.sync();
    updateRecentProjects();
}

void IDOSMainWindow::updateRecentProjects()
{
    if (m_startPage == nullptr)
    {
        return;
    }

    QStringList recent;
    const QString settingsPath = windowSettingsPath();
    if (!settingsPath.isEmpty())
    {
        QSettings settings(settingsPath, QSettings::IniFormat);
        recent = settings.value(QStringLiteral("recentProjects")).toStringList();
    }
    m_startPage->setRecentProjects(recent);
}

void IDOSMainWindow::updateCentralPage()
{
    if (m_centralPageStack == nullptr || m_startPage == nullptr || m_renderPage == nullptr)
    {
        return;
    }
    const bool hasProject = m_project != nullptr;
    m_centralPageStack->setCurrentWidget(hasProject ? m_renderPage : m_startPage);
    if (m_renderDock != nullptr)
    {
        m_renderDock->setWindowTitle(hasProject ? tr("3D View") : tr("Start"));
    }
}

void IDOSMainWindow::onStartPageRecentProjectOpenRequested(const QString& path)
{
    if (!path.isEmpty())
    {
        openProject(path);
    }
}

void IDOSMainWindow::onStartPageRecentProjectRemoveRequested(const QString& projectPath)
{
    const QString settingsPath = windowSettingsPath();
    if (settingsPath.isEmpty() || projectPath.isEmpty())
    {
        return;
    }

    QSettings settings(settingsPath, QSettings::IniFormat);
    QStringList recent = settings.value(QStringLiteral("recentProjects")).toStringList();
    recent.removeAll(projectPath);
    recent.removeAll(QFileInfo(projectPath).absoluteFilePath());
    settings.setValue(QStringLiteral("recentProjects"), recent);
    settings.sync();
    updateRecentProjects();
}

QList<IDOSWellLogTrackView*> IDOSMainWindow::logTrackViews() const
{
    QList<IDOSWellLogTrackView*> views;
    const QMap<QString, ads::CDockWidget*> docks = m_dockManager->dockWidgetsMap();
    for (ads::CDockWidget* dock : docks)
    {
        IDOSWellLogTrackView* view = qobject_cast<IDOSWellLogTrackView*>(dock->widget());
        if (view != nullptr)
        {
            views.append(view);
        }
    }
    return views;
}
