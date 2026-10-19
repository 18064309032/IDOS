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
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPointer>
#include <QScopedValueRollback>
#include <QScreen>
#include <QScrollBar>
#include <QSet>
#include <QSettings>
#include <QSignalBlocker>
#include <QSize>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QUuid>
#include <QVBoxLayout>
#include <QVariant>
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

#include "case/idoscasedialog.h"
#include "case/idoscasetreemenuprovider.h"
#include "case/idoscasetreerootprovider.h"
#include "case/idossimulationcasetreemetadata.h"
#include "case/idossimulationsummarycurveswidget.h"
#include "command/idoscasereferencecommand.h"
#include "command/idoscommandmanager.h"
#include "command/idoscommandmetadata.h"
#include "command/idoscommandregistry.h"
#include "command/idosconvertcoordinatescommand.h"
#include "command/idoscreatecasecommand.h"
#include "command/idoscreategridfiltercommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletedatacommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletegridfiltercommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosimportcasecommand.h"
#include "command/idosimportdatacommand.h"
#include "command/idosimportfluidrocktablescommand.h"
#include "command/idosimportgridcommand.h"
#include "command/idosimportgridpropertycommand.h"
#include "command/idosimportrestartresultscommand.h"
#include "command/idosimportsummaryresultscommand.h"
#include "command/idosimportwelldatacommand.h"
#include "command/idosinspectobjectcommand.h"
#include "command/idoslistobjectscommand.h"
#include "command/idosrenameobjectcommand.h"
#include "command/idostransferobjectscommand.h"
#include "command/idosupdategridfiltercommand.h"
#include "command/idosupdateprojectmetadatacommand.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/case/idossimulationsummaryresultsobject.h"
#include "data/fluidrock/idosfluidrocktableobject.h"
#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "data/idosdataregistry.h"
#include "data/idosdatatreemenuprovider.h"
#include "data/idosdatatreerootprovider.h"
#include "data/idosimporteddatatreemetadata.h"
#include "data/idosnaturalfracturerendermetadata.h"
#include "data/seismic/idosseismicvolumeobject.h"
#include "data/well/idoswellobject.h"
#include "fluidrock/idosfluidrocktabletreemetadata.h"
#include "grid/idosgridboundaryrendermetadata.h"
#include "grid/idosgridfilterrendermetadata.h"
#include "grid/idosgridfilterwidget.h"
#include "grid/idosgridrendermetadata.h"
#include "grid/idosgridrenderprovider.h"
#include "grid/idosgridsettingwidget.h"
#include "grid/idosgridsurfacerendermetadata.h"
#include "grid/idosgridtreemetadata.h"
#include "idosappinterface.h"
#include "idosapplication.h"
#include "idosapptreeaccess.h"
#include "idosassistantservice.h"
#include "idosassistantwidget.h"
#include "idosdataobject.h"
#include "idosdebuginfowidget.h"
#include "idosnewprojectdialog.h"
#include "idosproject.h"
#include "idosprojectmetadata.h"
#include "idosprojectstartwidget.h"
#include "idoswaitingdialog.h"
#include "idospythonconsolewidget.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idosrendertypes.h"
#include "idosrenderview.h"
#include "idosruntimeinfowidget.h"
#include "logger/idoslogger.h"
#include "plugin/idospluginmanagerwidget.h"
#include "plugin/idospluginregistry.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataproviderregistry.h"
#include "render/idosclippingplanetoolprovider.h"
#include "render/idosdistancemeasurementtoolprovider.h"
#include "render/idospointprobetoolprovider.h"
#include "render/idospointzoomtoolprovider.h"
#include "render/idosrenderproviderregistry.h"
#include "render/idosrenderserver.h"
#include "render/idosrendertoolmetadata.h"
#include "render/idosselectiontoolprovider.h"
#include "render/idosslicebypointstoolprovider.h"
#include "seismic/idosseismicvolumerendermetadata.h"
#include "seismic/idosseismicvolumerenderprovider.h"
#include "simulation/idossimulation.h"
#include "tools/idostoolbar.h"
#include "tools/idostoolproviderregistry.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeitemprovider.h"
#include "tree/idostreeitemproviderregistry.h"
#include "tree/idostreemodel.h"
#include "tree/idostreeview.h"
#include "well/idoswelldisplaywidget.h"
#include "well/idoswelllogtrackview.h"
#include "well/idoswellrendermetadata.h"
#include "well/idoswellrenderprovider.h"
#include "well/idoswelltreemetadata.h"

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
    , m_treeRegistry(new IDOSTreeItemProviderRegistry(this))
    , m_gridWidget(nullptr)
    , m_filterWidget(nullptr)
    , m_wellWidget(nullptr)
    , m_treeAccess(nullptr)
    , m_dataTreeModel(new IDOSDataTreeItemModel(m_treeRegistry, this))
    , m_caseTreeModel(new IDOSCaseTreeItemModel(m_treeRegistry, this))
    , m_dataTreeView(nullptr)
    , m_dataTreeMenuProvider(nullptr)
    , m_caseTreeView(nullptr)
    , m_caseTreeMenuProvider(nullptr)
    , m_renderRegistry(new IDOSRenderProviderRegistry(this))
    , m_toolRegistry(new IDOSToolProviderRegistry(this))
    , m_renderServer(new IDOSRenderServer(m_renderRegistry, this))
    , m_runtimeInfoWidget(nullptr)
    , m_pluginRegistry(nullptr)
    , m_pluginManagerWidget(nullptr)
    , m_appInterface(new IDOSAppInterface(this))
    , m_simulation(new IDOSSimulation(this))
    , m_debugInfoWidget(nullptr)
    , m_commandRegistry(new IDOSCommandRegistry(this))
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
    , m_simulationDock(nullptr)
    , m_debugDock(nullptr)
    , m_pythonConsoleDock(nullptr)
    , m_assistantDock(nullptr)
    , m_viewCaseIds()
    , m_nextViewIndex(1)
    , m_currentWindowLayout(QStringLiteral("single"))
    , m_dataImportActive(false)
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
    m_defaultViewState = activeView()->saveState();
    restoreWindowPreferences(true);
    if (m_simulationDock != nullptr)
    {
        m_simulationDock->toggleView(true);
        m_simulationDock->setAsCurrentTab();
    }
    m_restoringPresentation = false;
    if (!qEnvironmentVariableIsSet("IDOS_DISABLE_ASSISTANT_AUTOSTART"))
    {
        QTimer::singleShot(0, m_assistantWidget, &IDOSAssistantWidget::startAssistant);
    }
}

void IDOSMainWindow::initProviders()
{
    IDOSApplication::initProviders();

    m_dataTreeModel->setRootProvider(new IDOSDataTreeRootProvider());
    m_caseTreeModel->setRootProvider(new IDOSCaseTreeRootProvider());
    m_treeRegistry->registerMetadata(new IDOSWellTreeMetadata());
    m_treeRegistry->registerMetadata(new IDOSGridTreeMetadata());
    m_treeRegistry->registerMetadata(new IDOSSimulationCaseTreeMetadata(m_treeRegistry));
    m_treeRegistry->registerMetadata(new IDOSFluidRockTableTreeMetadata(IDOSFluidRockTableObject::Category::Fluid));
    m_treeRegistry->registerMetadata(new IDOSFluidRockTableTreeMetadata(IDOSFluidRockTableObject::Category::RockPhysics));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.microseismic.set"), QStringLiteral("data.microseismic")));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.seismic.volume"), QStringLiteral("data.seismic")));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.fracture.schedule"), QStringLiteral("data.hydraulicFractures")));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.naturalFracture.set"), QStringLiteral("data.naturalFractures")));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.production.history"), QStringLiteral("data.production")));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.fracture.fluidCatalog"), QStringLiteral("data.fluids")));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.fracture.proppantCatalog"), QStringLiteral("data.hydraulicFractures")));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.grid.surface"), QStringLiteral("data.surfaceProperties")));
    m_treeRegistry->registerMetadata(new IDOSImportedDataTreeMetadata(QStringLiteral("idos.grid.boundary"), QStringLiteral("data.boundaries")));

    m_toolRegistry->registerMetadata(new IDOSRenderToolMetadata());
    m_toolRegistry->registerMetadata(new IDOSPointZoomToolProviderMetadata());
    m_toolRegistry->registerMetadata(new IDOSPointProbeToolProviderMetadata());
    m_toolRegistry->registerMetadata(new IDOSDistanceMeasurementToolProviderMetadata());
    m_toolRegistry->registerMetadata(new IDOSSelectionToolProviderMetadata());
    m_toolRegistry->registerMetadata(new IDOSClippingPlaneToolProviderMetadata());
    m_toolRegistry->registerMetadata(new IDOSSliceByPointsToolProviderMetadata());
    m_caseTreeView = new IDOSCaseTreeView(this);
    m_caseTreeMenuProvider = new IDOSCaseTreeMenuProvider(m_caseTreeView);

    m_renderRegistry->registerMetadata(new IDOSGridRenderMetadata());
    m_renderRegistry->registerMetadata(new IDOSGridFilterRenderMetadata());
    m_renderRegistry->registerMetadata(new IDOSGridSurfaceRenderMetadata());
    m_renderRegistry->registerMetadata(new IDOSGridBoundaryRenderMetadata());
    m_renderRegistry->registerMetadata(new IDOSNaturalFractureRenderMetadata());
    m_renderRegistry->registerMetadata(new IDOSSeismicVolumeRenderMetadata());
    m_renderRegistry->registerMetadata(new IDOSWellRenderMetadata());

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
    m_dataTreeView = new IDOSDataTreeView(this);
    m_dataTreeView->setModel(m_dataTreeModel);
    connect(m_dataTreeView, &QTreeView::doubleClicked, this, &IDOSMainWindow::onDataTreeItemActivated);
    connect(m_dataTreeView, &QTreeView::clicked, this, &IDOSMainWindow::onDataTreeItemClicked);
    connect(m_dataTreeView->selectionModel(), &QItemSelectionModel::currentChanged,this, &IDOSMainWindow::onDataTreeCurrentChanged);

    m_dataTreeMenuProvider = new IDOSDataTreeMenuProvider(m_dataTreeView);
    m_dataTreeView->setMenuProvider(m_dataTreeMenuProvider);
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

    m_assistantWidget = new IDOSAssistantWidget(m_commandRegistry, this);
    IDOSCommandRegistry* commandRegistry = m_commandRegistry;
    if (commandRegistry != nullptr)
    {
        commandRegistry->registerMetadata(new IDOSCaseReferenceCommandMetadata(QStringLiteral("copy")));
        commandRegistry->registerMetadata(new IDOSCaseReferenceCommandMetadata(QStringLiteral("move")));
        commandRegistry->registerMetadata(new IDOSCaseReferenceCommandMetadata(QStringLiteral("remove")));
        commandRegistry->registerMetadata(new IDOSCreateCaseCommandMetadata());
        commandRegistry->registerMetadata(new IDOSCreateWellCommandMetadata());
        commandRegistry->registerMetadata(new IDOSListObjectsCommandMetadata());
        commandRegistry->registerMetadata(new IDOSInspectObjectCommandMetadata());
        commandRegistry->registerMetadata(new IDOSDeleteCaseCommandMetadata());
        commandRegistry->registerMetadata(new IDOSDeleteGridCommandMetadata());
        commandRegistry->registerMetadata(new IDOSCreateGridFilterCommandMetadata());
        commandRegistry->registerMetadata(new IDOSUpdateGridFilterCommandMetadata());
        commandRegistry->registerMetadata(new IDOSDeleteGridFilterCommandMetadata());
        commandRegistry->registerMetadata(new IDOSDeleteDataCommandMetadata());
        commandRegistry->registerMetadata(new IDOSDeletePropertyCommandMetadata());
        commandRegistry->registerMetadata(new IDOSDeleteWellCommandMetadata());
        commandRegistry->registerMetadata(new IDOSTransferObjectsCommandMetadata(QStringLiteral("copy")));
        commandRegistry->registerMetadata(new IDOSTransferObjectsCommandMetadata(QStringLiteral("move")));
        commandRegistry->registerMetadata(new IDOSRenameObjectCommandMetadata());
        commandRegistry->registerMetadata(new IDOSImportRestartResultsCommandMetadata());
        commandRegistry->registerMetadata(new IDOSImportCaseCommandMetadata());
        commandRegistry->registerMetadata(new IDOSImportGridCommandMetadata());
        commandRegistry->registerMetadata(new IDOSImportGridPropertyCommandMetadata());
        commandRegistry->registerMetadata(new IDOSImportWellDataCommandMetadata());
        commandRegistry->registerMetadata(new IDOSUpdateProjectMetadataCommandMetadata());
        commandRegistry->registerMetadata(new IDOSConvertCoordinatesCommandMetadata());
        commandRegistry->registerMetadata(new IDOSImportFluidRockTablesCommandMetadata());
        commandRegistry->registerMetadata(new IDOSAttachFluidRockTableCommandMetadata());
        commandRegistry->registerMetadata(new IDOSImportDataCommandMetadata());
        commandRegistry->registerMetadata(new IDOSImportSummaryResultsCommandMetadata());
    }
    m_assistantDock = new ads::CDockWidget(m_dockManager, tr("Assistant"));
    m_assistantDock->setObjectName(QStringLiteral("assistantDock"));
    m_assistantDock->setIcon(QIcon(QStringLiteral(":/images/assistant-chat.svg")));
    m_assistantDock->tabWidget()->setIconSize(QSize(16, 16));
    m_assistantDock->setWidget(m_assistantWidget, ads::CDockWidget::ForceNoScrollArea);
    m_assistantDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidgetTabToArea(m_assistantDock, outputArea);

    // ---- 工况树 ----
    m_caseTreeView->setBlankMenuEnabled(true);
    m_caseTreeView->setModel(m_caseTreeModel);
    connect(m_caseTreeView, &QTreeView::clicked, this, &IDOSMainWindow::onCaseTreeItemClicked);
    connect(m_caseTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &IDOSMainWindow::onCaseTreeCurrentChanged);
    m_caseTreeView->setMenuProvider(m_caseTreeMenuProvider);
    ads::CDockWidget* caseDock = new ads::CDockWidget(m_dockManager, tr("Case"));
    caseDock->setObjectName(QStringLiteral("caseTreeDock"));
    caseDock->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    caseDock->setWidget(m_caseTreeView, ads::CDockWidget::ForceNoScrollArea);
    caseDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidget(ads::BottomDockWidgetArea, caseDock, dataArea);

    updateTreeSelection(activeViewId());
    outputArea->setCurrentDockWidget(m_outputDock);
}

void IDOSMainWindow::setSimulationWorkspaceWidget(QWidget* widget)
{
    if (widget == nullptr || m_dockManager == nullptr || m_renderDock == nullptr)
    {
        return;
    }
    if (m_simulationDock == nullptr)
    {
        m_simulationDock = new ads::CDockWidget(m_dockManager, tr("Simulation Run"));
        m_simulationDock->setObjectName(QStringLiteral("simulationWorkspaceDock"));
        m_simulationDock->setIcon(QIcon(QStringLiteral(":/images/simulation-workflow.svg")));
        m_simulationDock->setFeatures(ads::CDockWidget::DockWidgetClosable
                                      | ads::CDockWidget::DockWidgetMovable
                                      | ads::CDockWidget::DockWidgetFloatable);
        m_dockManager->addDockWidget(ads::RightDockWidgetArea, m_simulationDock,
                                     m_renderDock->dockAreaWidget());
        connect(m_simulationDock,
                &ads::CDockWidget::viewToggled,
                this,
                &IDOSMainWindow::simulationWorkspaceVisibilityChanged,
                Qt::UniqueConnection);
    }
    m_simulationDock->setWidget(widget, ads::CDockWidget::ForceNoScrollArea);
    m_simulationDock->setMinimumWidth(320);
    m_simulationDock->setMinimumHeight(400);
}

void IDOSMainWindow::setSimulationWorkspaceVisible(bool visible)
{
    if (m_simulationDock == nullptr)
    {
        return;
    }
    m_simulationDock->toggleView(visible);
}

bool IDOSMainWindow::simulationWorkspaceVisible() const
{
    return m_simulationDock != nullptr && !m_simulationDock->isClosed();
}

void IDOSMainWindow::showSimulationWorkspace()
{
    if (m_simulationDock == nullptr)
    {
        return;
    }
    setSimulationWorkspaceVisible(true);
    m_dockManager->setDockWidgetFocused(m_simulationDock);
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
    SARibbonPanel* caseViewPanel = projectPage->addPanel(tr("Case View"));
    caseViewPanel->setObjectName(QStringLiteral("idosHomePanel_caseView"));
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
    connect(m_actionProjectSettings, &QAction::triggered,
            this, &IDOSMainWindow::onProjectSettings);
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
    clearTreeDialogs();
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
    delete m_treeAccess;
    m_treeAccess = nullptr;
    m_dataTreeModel->clear();
    m_caseTreeModel->clear();
    if (m_project)
    {
        disconnect(m_project, nullptr, this, nullptr);
    }
    if (m_commandManager != nullptr)
    {
        disconnect(m_commandManager.data(), nullptr, this, nullptr);
        m_commandManager->setRegistry(nullptr);
    }
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

IDOSDataTreeView* IDOSMainWindow::dataTreeView() const
{
    return m_dataTreeView;
}

IDOSCaseTreeView* IDOSMainWindow::caseTreeView() const
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
        m_commandManager->setRegistry(nullptr);
    }
    QScopedValueRollback<bool> presentationGuard(m_restoringPresentation, true);
    clearTreeDialogs();
    delete m_treeAccess;
    m_treeAccess = nullptr;
    m_project = nullptr;
    m_renderServer->setProject(nullptr);
    const QVariantMap presentation = project != nullptr ? project->saveState() : QVariantMap();
    prepareProjectViews(presentation);
    m_project = project;
    m_viewCaseIds.clear();
    m_viewContextIds.clear();
    m_commandManager = project != nullptr ? project->commandManager() : nullptr;
    if (m_commandManager != nullptr)
    {
        m_commandManager->setRegistry(m_commandRegistry);
    }
    m_renderServer->setProject(project);
    m_assistantWidget->setProject(project);
    m_treeAccess = new IDOSAppTreeAccess(project, this);
    m_treeAccess->setDataModel(m_dataTreeModel);
    m_treeAccess->setCaseModel(m_caseTreeModel);
    m_treeAccess->setRenderServer(m_renderServer);
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

IDOSDataTreeItemModel* IDOSMainWindow::dataTreeModel() const
{
    return m_dataTreeModel;
}

IDOSCaseTreeItemModel* IDOSMainWindow::caseTreeModel() const
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
                            :
                            QStringLiteral(":/images/render-view.svg")));
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
        IDOSTreeItem* item = m_dataTreeModel->item(current);
        objectId = item != nullptr ? item->provider()->dataId() : QString();
        currentObject = m_dataTreeModel->object(current);
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
    m_project->setCurrentObject(m_dataTreeModel->object(index));
}

void IDOSMainWindow::onCaseTreeItemClicked(const QModelIndex& index)
{
    if (m_project == nullptr || m_syncingTreeSelection)
    {
        return;
    }
    m_project->setCurrentObject(m_caseTreeModel->object(index));
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
        m_project->setCurrentObject(m_caseTreeModel->object(current));
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
                                 :
                                 QString();
    const QString caseId = m_viewCaseIds.value(viewId);
    const QModelIndex dataIndex = objectId.isEmpty()
                                      ? QModelIndex()
                                      : m_dataTreeModel->index(objectId);
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
        const QModelIndex caseIndex = m_caseTreeModel->index(caseId);
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
        return m_caseTreeModel->index(objectId);
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
        IDOSTreeItem* referenceNode = m_caseTreeModel->item(childIndex);
        if (referenceNode != nullptr && referenceNode->provider()->isReference() && referenceNode->provider()->dataId() == objectId)
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
    IDOSTreeItem* item = m_caseTreeModel->item(index);
    return item != nullptr && item->provider()->dataId() != item->provider()->caseId() ? item->provider()->dataId() : QString();
}

QString IDOSMainWindow::caseIdFromCaseTreeIndex(const QModelIndex& index) const
{
    IDOSTreeItem* item = m_caseTreeModel->item(index);
    return item != nullptr ? item->provider()->caseId() : QString();
}
void IDOSMainWindow::onDataTreeItemActivated(const QModelIndex& index)
{
    if (m_project == nullptr)
    {
        return;
    }

    IDOSTreeItem* partNode = m_dataTreeModel->item(index);
    if (partNode == nullptr || partNode->provider()->partKey() != QStringLiteral("idos.well.logs"))
    {
        return;
    }

    IDOSWellObject* well = qobject_cast<IDOSWellObject*>(m_project->object(partNode->provider()->dataId()));
    if (well == nullptr)
    {
        return;
    }

    IDOSWellLogTrackView* view = findOrCreateWellLogTrackView(well);
    if (view == nullptr)
    {
        return;
    }

    view->setSelectedChannelName(partNode->provider()->itemKey());
}

IDOSWellLogTrackView* IDOSMainWindow::findOrCreateWellLogTrackView(IDOSWellObject* well)
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
    if (m_dataImportActive)
    {
        event->ignore();
        return;
    }
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
    clearTreeDialogs();
    m_project = nullptr;
    m_commandManager = nullptr;
    m_viewCaseIds.clear();
    m_viewContextIds.clear();
    delete m_treeAccess;
    m_treeAccess = nullptr;
    m_dataTreeModel->clear();
    m_caseTreeModel->clear();
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
    connect(view, &IDOSRenderView::activated, this, &IDOSMainWindow::onRenderViewActivated);
    connect(view, &IDOSRenderView::stateChanged, this, &IDOSMainWindow::onPresentationChanged);
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

void IDOSMainWindow::onRenderViewActivated(const QString& id)
{
    IDOSRenderView* view = qobject_cast<IDOSRenderView*>(sender());
    IDOSDataObject* data = m_renderServer->dataObject(view, id);
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
        if (data != nullptr && m_renderServer->checkState(id, view) == Qt::Checked)
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

IDOSTreeItemProviderRegistry* IDOSMainWindow::treeRegistry() const
{
    return m_treeRegistry;
}

IDOSRenderProviderRegistry* IDOSMainWindow::renderRegistry() const
{
    return m_renderRegistry;
}

IDOSRenderServer* IDOSMainWindow::renderServer() const
{
    return m_renderServer;
}

IDOSToolProviderRegistry* IDOSMainWindow::toolRegistry() const
{
    return m_toolRegistry;
}

IDOSCommandRegistry* IDOSMainWindow::commandRegistry() const
{
    return m_commandRegistry;
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
    m_actionProjectSettings->setEnabled(m_project != nullptr);
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

void IDOSMainWindow::onProjectSettings()
{
    QPointer<IDOSProject> target(m_project);
    if (target == nullptr || target->commandManager() == nullptr)
    {
        return;
    }
    IDOSNewProjectDialog dialog(this);
    dialog.setProjectMetadata(target->metadata());
    if (dialog.exec() != QDialog::Accepted || target == nullptr || target != m_project)
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("metadata"), QJsonObject::fromVariantMap(dialog.projectMetadata().projectData()));
    IDOSCommandManager* manager = target->commandManager();
    if (!manager->execute(new IDOSUpdateProjectMetadataCommand(target, arguments)))
    {
        QMessageBox::warning(this, tr("Project Settings"), manager->error());
    }
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
        :
        QString();
    const QString projectStatus = !hasProject
        ? tr("No Project")
        :
        QStringLiteral("%1  ·  %2")
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
    state.insert(QStringLiteral("render"), m_renderServer->saveState());
    state.insert(QStringLiteral("dataTree"), m_dataTreeView->saveState());
    state.insert(QStringLiteral("caseTree"), m_caseTreeView->saveState());
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
            QStringLiteral("wellLogTrackDock_%1").arg(view->saveState().value(QStringLiteral("wellId")).toString()));
        if (dock != nullptr && !dock->isClosed())
        {
            const QVariantMap saved = view->saveState();
            logViews.insert(saved.value(QStringLiteral("wellId")).toString(), saved);
        }
    }
    state.insert(QStringLiteral("logViews"), logViews.values());
    // An untouched old project should not become dirty solely because it lacks a UI snapshot.
    if (force || !m_project->saveState().isEmpty() || m_project->isModified())
    {
        m_project->restoreState(state);
    }
}

void IDOSMainWindow::prepareProjectViews(const QVariantMap& state)
{
    for (IDOSWellLogTrackView* view : logTrackViews())
    {
        const QString wellId = view->saveState().value(QStringLiteral("wellId")).toString();
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
        main->restoreState(m_defaultViewState);
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
            IDOSWellObject* well = m_project != nullptr ? qobject_cast<IDOSWellObject*>(m_project->object(saved.value(QStringLiteral("wellId")).toString())) : nullptr;
            if (well != nullptr && ++logCount <= 32)
            {
                findOrCreateWellLogTrackView(well)->restoreState(saved);
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
        if (!m_renderServer->restoreState(state.value(QStringLiteral("render")).toMap()))
        {
            IDOS_MESSAGE(tr("The saved render state could not be restored."), IDOSLogLevel::Warn);
        }
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
        m_dataTreeView->restoreState(state.value(QStringLiteral("dataTree")).toMap());
        m_caseTreeView->restoreState(state.value(QStringLiteral("caseTree")).toMap());
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
    if (m_simulationDock != nullptr)
    {
        m_simulationDock->toggleView(true);
        m_simulationDock->setAsCurrentTab();
    }
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
        ids.append(QStringLiteral("log_") + view->saveState().value(QStringLiteral("wellId")).toString());
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

void IDOSMainWindow::onDataDeleteDatasets(IDOSTreeView* sourceView, const QStringList& requestedObjectIds)
{
    QPointer<IDOSTreeView> view(sourceView);
    if (view.isNull())
    {
        return;
    }
    QPointer<IDOSProject> project(m_project);
    const QStringList ids = requestedObjectIds;
    QStringList names;
    for (const QString& id : ids)
    {
        IDOSDataObject* object = project->object(id);
        if (!IDOSDeleteDataCommand::supportsObject(object))
        {
            return;
        }
        names.append(object->name());
    }
    if (ids.isEmpty())
    {
        return;
    }
    const QMessageBox::StandardButton answer =
        QMessageBox::question(view.data(), tr("Delete Data"),
                              tr("Delete %1 selected dataset(s)?\nTheir case references will also be removed. You can "
                                 "undo this operation.\n\n%2")
                                  .arg(ids.size())
                                  .arg(names.join(QStringLiteral("\n"))),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes || view.isNull() || project.isNull() || m_project != project.data() ||
        project->commandManager() == nullptr)
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("objectIds"), QJsonArray::fromStringList(ids));
    const QJsonObject response = project->commandManager()->execute(QStringLiteral("data.delete"), arguments);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        QMessageBox::warning(view.data(), tr("Delete Data"), response.value(QStringLiteral("error")).toString());
    }
}

void IDOSMainWindow::onDataAddToCase(IDOSTreeView* sourceView, const QJsonArray& requestedItems)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(m_project);
    const QJsonArray records = requestedItems;
    QStringList labels;
    QStringList ids;
    for (IDOSDataObject* object : project->objects())
    {
        if (qobject_cast<IDOSSimulationCaseObject*>(object) != nullptr)
        {
            labels.append(tr("%1 [%2]").arg(object->name(), object->objectId().left(8)));
            ids.append(object->objectId());
        }
    }
    if (ids.isEmpty())
    {
        QMessageBox::information(view, tr("Add to Case"), tr("Create a case before adding data."));
        return;
    }
    bool accepted = false;
    const QString label =
        QInputDialog::getItem(view, tr("Add to Case"), tr("Destination case:"), labels, 0, false, &accepted);
    if (!accepted || project == nullptr || project.data() != m_project)
    {
        return;
    }
    QJsonArray items;
    for (const QJsonValue& value : records)
    {
        QJsonObject record = value.toObject();
        record.remove(QStringLiteral("sourceCaseId"));
        items.append(record);
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("items"), items);
    arguments.insert(QStringLiteral("targetCaseId"), ids.value(labels.indexOf(label)));
    if (!project->commandManager()->execute(
            new IDOSCaseReferenceCommand(project.data(), QStringLiteral("copy"), arguments)))
    {
        QMessageBox::warning(view, tr("Add to Case"), project->commandManager()->error());
    }
}

void IDOSMainWindow::onDataRenameObject(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(m_project);
    const QString id = requestedDataId;
    IDOSDataObject* object = project->object(id);
    if (object == nullptr)
    {
        return;
    }
    bool accepted = false;
    const QString name =
        QInputDialog::getText(view, tr("Rename Object"), tr("Name:"), QLineEdit::Normal, object->name(), &accepted);
    if (accepted && project != nullptr && project.data() == m_project &&
        !project->commandManager()->execute(new IDOSRenameObjectCommand(project.data(), id, name)))
    {
        QMessageBox::warning(view, tr("Rename Object"), project->commandManager()->error());
    }
}

void IDOSMainWindow::onDataGridImportProperty(IDOSTreeView* sourceView, const QString& requestedDataId,
                                              const QString& requestedCaseId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString gridId = requestedDataId;
    if (qobject_cast<IDOSGridObject*>(m_project->object(gridId)) != nullptr)
    {
        dataImportProperty(view, gridId, requestedCaseId);
    }
}

void IDOSMainWindow::onDataGridImportDynamicProperties(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString gridId = requestedDataId;
    if (qobject_cast<IDOSGridObject*>(m_project->object(gridId)) != nullptr)
    {
        dataImportDynamicProperties(view, gridId);
    }
}

void IDOSMainWindow::onDataGridDeleteProperty(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString id = requestedDataId;
    if (qobject_cast<IDOSGridPropertyObject*>(m_project->object(id)) != nullptr)
    {
        dataDeleteProperty(view, id);
    }
}

void IDOSMainWindow::onDataGridDelete(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString id = requestedDataId;
    if (qobject_cast<IDOSGridObject*>(m_project->object(id)) != nullptr)
    {
        dataDeleteGrid(view, id);
    }
}

IDOSRenderObject* IDOSMainWindow::dataGridRenderObject(const QString& gridId,
                                                            IDOSRenderView* view) const
{
    if (view == nullptr)
    {
        return nullptr;
    }
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        IDOSDataObject* source = m_renderServer->dataObject(view, object->id());
        if (source != nullptr && source->objectId() == gridId &&
            dynamic_cast<IDOSGridRenderProvider*>(object->provider()) != nullptr)
        {
            return object;
        }
    }
    return nullptr;
}

int IDOSMainWindow::dataGridPropertyCount(const IDOSTreeView* view, const QString& gridId) const
{
    if (view == nullptr || m_project == nullptr || gridId.isEmpty())
    {
        return 0;
    }
    int count = 0;
    for (IDOSDataObject* object : m_project->objects())
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            ++count;
        }
    }
    return count;
}

bool IDOSMainWindow::dataDeleteGrid(IDOSTreeView* view, const QString& gridId)
{
    if (view == nullptr || m_project == nullptr || gridId.isEmpty())
    {
        return false;
    }
    QPointer<IDOSProject> target(m_project);
    IDOSDataObject* obj = target->object(gridId);
    IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(obj);
    if (grid == nullptr)
    {
        return false;
    }
    const int propertyCount = dataGridPropertyCount(view, gridId);
    const QMessageBox::StandardButton btn =
        QMessageBox::question(view, tr("Delete Grid"),
                               tr("Are you sure you want to delete grid \"%1\"?\n"
                                  "It will also delete %2 propert(ies) under this grid "
                                  "and all saved filters, and remove it from all cases that reference it.")
                                   .arg(grid->name())
                                   .arg(propertyCount),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeleteGridCommand(target, gridId));
}

bool IDOSMainWindow::dataDeleteProperty(IDOSTreeView* view, const QString& propertyId)
{
    if (view == nullptr || m_project == nullptr || propertyId.isEmpty())
    {
        return false;
    }
    QPointer<IDOSProject> target(m_project);
    IDOSDataObject* obj = target->object(propertyId);
    if (qobject_cast<IDOSGridPropertyObject*>(obj) == nullptr)
    {
        return false;
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(view, tr("Delete Property"),
                               tr("Are you sure you want to delete property \"%1\"?\n"
                                  "It will be removed from all cases that reference it.")
                                   .arg(obj->name()),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeletePropertyCommand(target, propertyId));
}

bool IDOSMainWindow::dataImportProperty(IDOSTreeView* view, const QString& targetGridId,
                                            const QString& targetCaseId)
{
    if (view == nullptr || m_project == nullptr)
    {
        return false;
    }
    QPointer<IDOSProject> target(m_project);
    // .DATA imports properties together with the parsed simulation deck.
    const QString file = QFileDialog::getOpenFileName(
        view, tr("Import Grid Property"), QString(),
        tr("ECLIPSE Properties (*.GRDECL *.grdecl *.INC *.inc *.INIT *.init);;"
           "ECLIPSE Data Files (*.DATA *.data);;All Files (*)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("targetGridId"), targetGridId);
    arguments.insert(QStringLiteral("targetCaseId"), targetCaseId);
    IDOSImportGridPropertyCommand* command =
        new IDOSImportGridPropertyCommand(target, arguments);
    const bool success = executeDataImport(commandManager, command, file);
    if (!success)
    {
        QMessageBox::warning(view, tr("Import Grid Property"), commandManager->error());
        return false;
    }

    const int importedCount = commandManager->result().value(QStringLiteral("objectCount")).toInt();
    dataReportGridImportSuccess(view, tr("Imported %1 properties.").arg(importedCount));
    return true;
}

bool IDOSMainWindow::executeDataImport(IDOSCommandManager* manager, IDOSCommand* command,
                                     const QString& filePath)
{
    const QScopedValueRollback<bool> importing(m_dataImportActive, true);
    IDOSWaitingDialog loading(this);
    loading.setObjectName(QStringLiteral("dataLoadingDialog"));
    loading.begin(tr("Importing Data"), tr("Reading data and updating the project…"), QFileInfo(filePath).fileName());
    QElapsedTimer timer;
    timer.start();
    const bool success = manager->execute(command);
    if (qEnvironmentVariableIsSet("IDOS_IMPORT_PROFILE"))
    {
        qInfo("IMPORT_PROFILE phase=command-total elapsed_ms=%.3f", timer.nsecsElapsed() / 1000000.0);
    }
    return success;
}

bool IDOSMainWindow::dataImportDynamicProperties(IDOSTreeView* view, const QString& targetGridId)
{
    if (view == nullptr || m_project == nullptr)
    {
        return false;
    }
    QPointer<IDOSProject> target(m_project);
    const QString file = QFileDialog::getOpenFileName(
        view, tr("Import Dynamic Properties"), QString(),
        tr("Unified Eclipse restart files (*.UNRST *.unrst)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    const bool success = executeDataImport(commandManager,
        new IDOSImportRestartResultsCommand(target, targetGridId, file), file);
    if (!success)
    {
        QMessageBox::warning(view, tr("Import Dynamic Properties"), commandManager->error());
        return false;
    }

    const QJsonObject result = commandManager->result();
    const QString message = tr("Imported %1 dynamic properties across %2 report steps (%3–%4).")
        .arg(result.value(QStringLiteral("propertyCount")).toInt())
        .arg(result.value(QStringLiteral("reportStepCount")).toInt())
        .arg(result.value(QStringLiteral("firstReportStep")).toInt())
        .arg(result.value(QStringLiteral("lastReportStep")).toInt());
    dataReportGridImportSuccess(view, message);
    return true;
}

void IDOSMainWindow::dataReportGridImportSuccess(IDOSTreeView* view, const QString& message)
{
    IDOS_MESSAGE(message, IDOSLogLevel::Info);
    QMainWindow* mainWindow = view != nullptr ? qobject_cast<QMainWindow*>(view->window()) : nullptr;
    if (mainWindow != nullptr && mainWindow->statusBar() != nullptr)
    {
        mainWindow->statusBar()->showMessage(message, 5000);
    }
}

void IDOSMainWindow::onDataGridDeleteFilter(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(m_project);
    const QString filterId = requestedDataId;
    IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(project->object(filterId));
    if (filter == nullptr)
    {
        return;
    }
    const QMessageBox::StandardButton answer =
        QMessageBox::question(view, tr("Delete Filter"), tr("Delete filter \"%1\"?").arg(filter->name()),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::Yes && project != nullptr && project->commandManager() != nullptr)
    {
        QJsonObject arguments;
        arguments.insert(QStringLiteral("filterId"), filterId);
        if (!project->commandManager()->execute(
                new IDOSDeleteGridFilterCommand(project.data(), arguments)))
        {
            QMessageBox::warning(view, tr("Delete Filter"), project->commandManager()->error());
        }
    }
}

void IDOSMainWindow::onDataGridRenameFilter(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(m_project->object(requestedDataId));
    if (filter == nullptr)
    {
        return;
    }
    QInputDialog* dialog = new QInputDialog(view);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Rename Filter"));
    dialog->setLabelText(tr("Filter name:"));
    dialog->setTextValue(filter->name());
    dialog->setProperty("filterId", filter->objectId());
    dialog->setProperty("project", QVariant::fromValue<QObject*>(m_project));
    connect(dialog, &QDialog::accepted, this, &IDOSMainWindow::onDataGridFilterRenameAccepted);
    connect(m_project, &QObject::destroyed, dialog, &QObject::deleteLater);
    dialog->open();
}

void IDOSMainWindow::onDataGridFilterRenameAccepted()
{
    QInputDialog* dialog = qobject_cast<QInputDialog*>(sender());
    IDOSTreeView* view = dialog != nullptr ? qobject_cast<IDOSTreeView*>(dialog->parentWidget()) : nullptr;
    if (view == nullptr || m_project == nullptr || dialog->property("project").value<QObject*>() != m_project)
    {
        return;
    }
    const QString filterId = dialog->property("filterId").toString();
    if (qobject_cast<IDOSGridFilterObject*>(m_project->object(filterId)) == nullptr)
    {
        return;
    }
    m_project->commandManager()->execute(new IDOSRenameObjectCommand(m_project, filterId, dialog->textValue()));
}

void IDOSMainWindow::onDataViewWellData(IDOSTreeView* sourceView, const QString& requestedDataId,
                                        const QString& requestedPart, const QString& requestedItemKey)
{
    IDOSTreeView* view = sourceView;
    IDOSWellObject* well =
        view != nullptr && m_project != nullptr ? qobject_cast<IDOSWellObject*>(m_project->object(requestedDataId)) : nullptr;
    if (well == nullptr)
    {
        return;
    }
    QDialog* dialog = new QDialog(view);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setObjectName(QStringLiteral("wellInputDataWindow"));
    dialog->setWindowTitle(tr("Well Data - %1").arg(well->name()));
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    QTableWidget* table = new QTableWidget(dialog);
    table->setEditTriggers(QTableWidget::NoEditTriggers);
    const QString part = requestedPart;
    if (part == QStringLiteral("idos.well.summary"))
    {
        table->setColumnCount(2);
        table->setHorizontalHeaderLabels({tr("Data"), tr("Count")});
        QStringList labels;
        QList<int> counts;
        if (well->hasPath())
        {
            labels.append(tr("Trajectory points"));
            counts.append(well->path().pointCount());
        }
        if (well->hasLogs())
        {
            labels.append(tr("Log curves"));
            counts.append(well->logs().channelCount());
        }
        if (well->hasMarkers())
        {
            labels.append(tr("Markers"));
            counts.append(well->markers().markerCount());
        }
        if (well->hasCompletions())
        {
            labels.append(tr("Completions"));
            counts.append(well->completions().size());
        }
        if (well->hasCheckshot())
        {
            labels.append(tr("Checkshot samples"));
            counts.append(well->checkshot().depths().size());
        }
        table->setRowCount(labels.size());
        for (int row = 0; row < labels.size(); ++row)
        {
            table->setItem(row, 0, new QTableWidgetItem(labels.at(row)));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(counts.at(row))));
        }
    }
    else if (part == QStringLiteral("idos.well.trajectory"))
    {
        const QVector<IDOSWellPathPoint> points = well->path().points();
        QStringList attributeHeaders;
        for (const IDOSWellPathPoint& point : points)
        {
            const QStringList keys = point.attributes().keys();
            for (const QString& key : keys)
            {
                if (!attributeHeaders.contains(key))
                {
                    attributeHeaders.append(key);
                }
            }
        }
        table->setColumnCount(5 + attributeHeaders.size());
        QStringList headers = {tr("Measured Depth"), tr("X"), tr("Y"), tr("Z"), tr("True Vertical Depth")};
        headers.append(attributeHeaders);
        table->setHorizontalHeaderLabels(headers);
        table->setRowCount(points.size());
        for (int row = 0; row < points.size(); ++row)
        {
            const IDOSWellPathPoint& point = points.at(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(point.md(), 'g', 15)));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(point.x(), 'g', 15)));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(point.y(), 'g', 15)));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(point.z(), 'g', 15)));
            table->setItem(row, 4, new QTableWidgetItem(QString::number(point.tvd(), 'g', 15)));
            const QVariantMap attributes = point.attributes();
            for (int column = 0; column < attributeHeaders.size(); ++column)
            {
                table->setItem(row, 5 + column,
                               new QTableWidgetItem(attributes.value(attributeHeaders.at(column)).toString()));
            }
        }
    }
    else if (part == QStringLiteral("idos.well.zones"))
    {
        table->setColumnCount(2);
        table->setHorizontalHeaderLabels({tr("Horizon"), tr("Measured depth (m)")});
        QList<IDOSWellMarker> markers = well->markers().markers();
        const QString markerName = requestedItemKey;
        if (!markerName.isEmpty())
        {
            for (int index = markers.size() - 1; index >= 0; --index)
            {
                if (markers.at(index).name() != markerName)
                {
                    markers.removeAt(index);
                }
            }
        }
        table->setRowCount(markers.size());
        for (int row = 0; row < markers.size(); ++row)
        {
            table->setItem(row, 0, new QTableWidgetItem(markers.at(row).name()));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(markers.at(row).depth(), 'g', 15)));
        }
    }
    else if (part == QStringLiteral("idos.well.completions"))
    {
        const QList<IDOSWellCompletion> completions = well->completions();
        QStringList attributeHeaders;
        for (const IDOSWellCompletion& completion : completions)
        {
            const QStringList keys = completion.attributes().keys();
            for (const QString& key : keys)
            {
                if (!attributeHeaders.contains(key))
                {
                    attributeHeaders.append(key);
                }
            }
        }
        table->setColumnCount(5 + attributeHeaders.size());
        QStringList headers = {tr("Top depth (m)"), tr("Bottom depth (m)"), tr("Grid layer"), tr("Status"),
                               tr("Event date")};
        headers.append(attributeHeaders);
        table->setHorizontalHeaderLabels(headers);
        table->setRowCount(completions.size());
        for (int row = 0; row < completions.size(); ++row)
        {
            const IDOSWellCompletion& completion = completions.at(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(completion.top(), 'g', 15)));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(completion.bottom(), 'g', 15)));
            table->setItem(
                row, 2, new QTableWidgetItem(completion.k() > 0 ? QString::number(completion.k()) : tr("Not mapped")));
            table->setItem(row, 3, new QTableWidgetItem(completion.isOpen() ? tr("Open") : tr("Shut")));
            table->setItem(row, 4, new QTableWidgetItem(completion.eventDate()));
            const QVariantMap attributes = completion.attributes();
            for (int column = 0; column < attributeHeaders.size(); ++column)
            {
                const QVariant value = attributes.value(attributeHeaders.at(column));
                table->setItem(row, 5 + column, new QTableWidgetItem(value.toString()));
            }
        }
    }
    else if (part == QStringLiteral("idos.well.logs"))
    {
        const QString channelName = requestedItemKey;
        QList<IDOSWellLogChannel> channels;
        for (const IDOSWellLogChannel& channel : well->logs().channels())
        {
            if (channelName.isEmpty() || channel.name() == channelName)
            {
                channels.append(channel);
            }
        }
        table->setColumnCount(channels.size() + 1);
        QStringList headers = {tr("Depth (m)")};
        for (const IDOSWellLogChannel& channel : channels)
        {
            const QString unit =
                channel.unit().isEmpty() ? QString() : QStringLiteral(" (") + channel.unit() + QLatin1Char(')');
            headers.append(channel.name() + unit);
        }
        table->setHorizontalHeaderLabels(headers);
        table->setRowCount(channels.isEmpty() ? 0 : channels.first().sampleCount());
        for (int row = 0; row < table->rowCount(); ++row)
        {
            table->setItem(row, 0, new QTableWidgetItem(QString::number(channels.first().depths().at(row), 'g', 15)));
            for (int column = 0; column < channels.size(); ++column)
            {
                const double value = channels.at(column).values().at(row);
                table->setItem(row, column + 1,
                               new QTableWidgetItem(qIsNaN(value) ? QString() : QString::number(value, 'g', 15)));
            }
        }
    }
    else
    {
        const IDOSWellCheckshot& checkshot = well->checkshot();
        table->setColumnCount(2);
        const QString depthUnit = checkshot.depthUnit().isEmpty() ? tr("Unspecified unit") : checkshot.depthUnit();
        const QString timeUnit = checkshot.timeUnit().isEmpty() ? tr("Unspecified unit") : checkshot.timeUnit();
        const QString timeName = checkshot.timeType() == QStringLiteral("TWT")   ? tr("Two-way time")
                                 : checkshot.timeType() == QStringLiteral("OWT") ? tr("One-way time")
                                                                                 : tr("Time (type unspecified)");
        const QString depthName = checkshot.depthType() == QStringLiteral("TVD")
                                      ? tr("True vertical depth (%1)").arg(depthUnit)
                                      : tr("Depth (%1)").arg(depthUnit);
        table->setHorizontalHeaderLabels({depthName, tr("%1 (%2)").arg(timeName, timeUnit)});
        table->setRowCount(checkshot.depths().size());
        for (int row = 0; row < checkshot.depths().size(); ++row)
        {
            table->setItem(row, 0, new QTableWidgetItem(QString::number(checkshot.depths().at(row), 'g', 15)));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(checkshot.times().at(row), 'g', 15)));
        }
    }
    table->resizeColumnsToContents();
    layout->addWidget(table);
    dialog->resize(620, 400);
    dialog->show();
}

void IDOSMainWindow::onDataNewWell(IDOSTreeView* sourceView)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    IDOSMainWindow::dataNewWell(m_project, view);
}

void IDOSMainWindow::onDataImportWell(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    if (sourceView == nullptr || m_project == nullptr)
    {
        return;
    }
    if (requestedDataId.isEmpty())
    {
        dataImportWellData(m_project, sourceView);
    }
    else if (qobject_cast<IDOSWellObject*>(m_project->object(requestedDataId)) != nullptr)
    {
        dataImportWellData(m_project, requestedDataId, sourceView);
    }
}

void IDOSMainWindow::onDataDeleteWell(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString id = requestedDataId;
    if (qobject_cast<IDOSWellObject*>(m_project->object(id)) != nullptr)
    {
        IDOSMainWindow::dataDeleteWell(m_project, id, view);
    }
}

int IDOSMainWindow::dataImportWellData(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull())
    {
        return 0;
    }
    const QStringList files = QFileDialog::getOpenFileNames(
        parent, tr("Import Well Data"), QString(),
        IDOSDataProviderRegistry::instance().fileFilters(QStringLiteral("idos.well.")));
    int imported = 0;
    QStringList errors;
    for (const QString& file : files)
    {
        if (target.isNull())
        {
            break;
        }
        QJsonObject arguments;
        arguments.insert(QStringLiteral("filePath"), file);
        IDOSCommandManager* manager = target->commandManager();
        if (executeDataImport(manager, new IDOSImportWellDataCommand(target.data(), arguments), file))
        {
            ++imported;
        }
        else
        {
            errors.append(manager->error());
        }
    }
    if (!errors.isEmpty())
    {
        QMessageBox::warning(parent, tr("Import Well Data"), errors.join(QStringLiteral("\n")));
    }
    return imported;
}

int IDOSMainWindow::dataImportWellData(IDOSProject* project, const QString& targetWellId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || qobject_cast<IDOSWellObject*>(target->object(targetWellId)) == nullptr)
    {
        return 0;
    }
    const QString file = QFileDialog::getOpenFileName(
        parent, tr("Import Well Data"), QString(),
        IDOSDataProviderRegistry::instance().fileFilters(QStringLiteral("idos.well.")));
    if (file.isEmpty() || target.isNull())
    {
        return 0;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("targetWellId"), targetWellId);
    IDOSCommandManager* manager = target->commandManager();
    if (!executeDataImport(manager, new IDOSImportWellDataCommand(target.data(), arguments), file))
    {
        QMessageBox::warning(parent, tr("Import Well Data"), manager->error());
        return 0;
    }
    return 1;
}

bool IDOSMainWindow::dataNewWell(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> targetProject(project);
    if (targetProject.isNull())
    {
        return false;
    }
    QInputDialog dialog(parent);
    dialog.setObjectName(QStringLiteral("newWellDialog"));
    dialog.setWindowTitle(tr("New Well"));
    dialog.setWindowIcon(QIcon(QStringLiteral(":/images/gui-well-new.svg")));
    dialog.setLabelText(tr("Well name:"));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setOkButtonText(tr("Create"));
    dialog.setCancelButtonText(tr("Cancel"));
    while (dialog.exec() == QDialog::Accepted)
    {
        if (targetProject.isNull())
        {
            return false;
        }
        const QString name = dialog.textValue().trimmed();
        if (name.isEmpty())
        {
            dialog.setLabelText(tr("Please enter a well name."));
            continue;
        }
        bool exists = false;
        for (IDOSDataObject* object : targetProject->objects())
        {
            IDOSWellObject* existing = qobject_cast<IDOSWellObject*>(object);
            if (existing != nullptr && existing->name().compare(name, Qt::CaseInsensitive) == 0)
            {
                exists = true;
                break;
            }
        }
        if (exists)
        {
            dialog.setLabelText(tr("A well named \"%1\" already exists. Please enter a different name.").arg(name));
            continue;
        }
        IDOSCommandManager* commandManager = targetProject->commandManager();
        if (commandManager == nullptr)
        {
            return false;
        }

        return commandManager->execute(new IDOSCreateWellCommand(targetProject, name));
    }
    return false;
}

bool IDOSMainWindow::dataDeleteWell(IDOSProject* project, const QString& wellId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || wellId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->object(wellId);
    IDOSWellObject* well = qobject_cast<IDOSWellObject*>(obj);
    if (well == nullptr)
    {
        return false;
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(parent, tr("Delete Well"),
                               tr("Are you sure you want to delete well \"%1\"?\n"
                                  "It will be removed from all cases that reference it.")
                                   .arg(well->name()),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeleteWellCommand(target, wellId));
}

void IDOSMainWindow::onCaseGenerateInput(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString caseId = requestedDataId;
    if (qobject_cast<IDOSCaseObject*>(m_project->object(caseId)) != nullptr)
    {
        m_simulation->onGenerateInputRequested(m_project, caseId);
    }
}

void IDOSMainWindow::onCaseViewPropertyCurves(IDOSTreeView* sourceView, const QString& requestedCaseId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString caseId = requestedCaseId;
    IDOSSimulationCaseObject* simulationCase = qobject_cast<IDOSSimulationCaseObject*>(m_project->object(caseId));
    if (simulationCase != nullptr && !simulationCase->curveTables().isEmpty())
    {
        m_simulation->onPropertyCurvesRequested(m_project, caseId);
    }
}

void IDOSMainWindow::onCaseImportSummaryResults(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString caseId = requestedDataId;
    if (qobject_cast<IDOSSimulationCaseObject*>(m_project->object(caseId)) == nullptr)
    {
        return;
    }
    const QString filePath =
        QFileDialog::getOpenFileName(view, tr("Import Summary Results"), QString(),
                                     tr("ECLIPSE Summary Specification (*.SMSPEC *.smspec);;All Files (*)"));
    if (filePath.isEmpty())
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), filePath);
    arguments.insert(QStringLiteral("targetCaseId"), caseId);
    IDOSCommandManager* manager = m_project->commandManager();
    if (manager == nullptr || !executeDataImport(manager, new IDOSImportSummaryResultsCommand(m_project, arguments), filePath))
    {
        QMessageBox::warning(view, tr("Import Summary Results"),
                             manager != nullptr ? manager->error()
                                                : tr("The command manager is unavailable."));
    }
}

void IDOSMainWindow::onCaseViewSummaryResults(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString summaryId = requestedDataId;
    IDOSSimulationSummaryResultsObject* results = qobject_cast<IDOSSimulationSummaryResultsObject*>(m_project->object(summaryId));
    if (results != nullptr)
    {
        IDOSSimulationSummaryCurvesWidget* widget = new IDOSSimulationSummaryCurvesWidget(results, view);
        widget->show();
    }
}

void IDOSMainWindow::onCaseRenameCase(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString caseId = requestedDataId;
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(m_project->object(caseId));
    if (caseObject == nullptr)
    {
        return;
    }
    QInputDialog dialog(view);
    dialog.setObjectName(QStringLiteral("renameCaseDialog"));
    dialog.setWindowTitle(tr("Rename Case"));
    dialog.setLabelText(tr("Case name:"));
    dialog.setTextValue(caseObject->name());
    dialog.setInputMode(QInputDialog::TextInput);
    QPointer<IDOSProject> targetProject(m_project);
    if (dialog.exec() != QDialog::Accepted || m_project == nullptr)
    {
        return;
    }
    const QString newName = dialog.textValue().trimmed();
    if (newName.isEmpty())
    {
        QMessageBox::warning(view, tr("Rename Case"), tr("Please enter a case name."));
        return;
    }
    caseObject = targetProject != nullptr ? qobject_cast<IDOSCaseObject*>(targetProject->object(caseId)) : nullptr;
    if (caseObject == nullptr)
    {
        return;
    }
    if (newName.compare(caseObject->name(), Qt::CaseInsensitive) != 0 && caseNameExists(view, newName))
    {
        QMessageBox::warning(view, tr("Rename Case"), tr("A case with this name already exists."));
        return;
    }
    IDOSCommandManager* manager = targetProject != nullptr ? targetProject->commandManager() : nullptr;
    if (manager != nullptr && !manager->execute(new IDOSRenameObjectCommand(targetProject.data(), caseId, newName)))
    {
        QMessageBox::warning(view, tr("Rename Case"), manager->error());
    }
}

void IDOSMainWindow::onCaseNewCase(IDOSTreeView* sourceView)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    caseNewCase(view);
}

void IDOSMainWindow::onCaseImportCase(IDOSTreeView* sourceView)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    caseImportCase(view);
}

void IDOSMainWindow::onCaseImportGrid(IDOSTreeView* sourceView, const QString& requestedCaseId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString caseId = requestedCaseId;
    if (qobject_cast<IDOSCaseObject*>(m_project->object(caseId)) != nullptr)
    {
        caseImportGrid(view, caseId);
    }
}

void IDOSMainWindow::onCaseDeleteCase(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    const QString id = requestedDataId;
    if (qobject_cast<IDOSCaseObject*>(m_project->object(id)) != nullptr)
    {
        caseDeleteCase(view, id);
    }
}

int IDOSMainWindow::caseGridPropertyCount(const IDOSTreeView* view, const QString& gridId) const
{
    if (view == nullptr || m_project == nullptr || gridId.isEmpty())
    {
        return 0;
    }
    int count = 0;
    for (IDOSDataObject* object : m_project->objects())
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            ++count;
        }
    }
    return count;
}

QString IDOSMainWindow::caseReferencedGridId(const IDOSCaseObject* caseObj)
{
    if (caseObj == nullptr)
    {
        return QString();
    }
    const QList<IDOSCaseItemRef> refs = caseObj->itemRefs();
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.grid"))
        {
            return ref.objectId();
        }
    }
    return QString();
}

bool IDOSMainWindow::caseNameExists(const IDOSTreeView* view, const QString& name) const
{
    if (view == nullptr || m_project == nullptr)
    {
        return false;
    }
    for (IDOSDataObject* object : m_project->objects())
    {
        if (qobject_cast<IDOSCaseObject*>(object) != nullptr &&
            object->name().trimmed().compare(name, Qt::CaseInsensitive) == 0)
        {
            return true;
        }
    }
    return false;
}

bool IDOSMainWindow::caseNewCase(IDOSTreeView* view)
{
    if (view == nullptr || m_project == nullptr)
    {
        return false;
    }
    QPointer<IDOSProject> target(m_project);
    IDOSCaseDialog dialog(QString(), QString(), {}, target, view);
    if (dialog.exec() != QDialog::Accepted || target.isNull())
    {
        return false;
    }
    if (caseNameExists(view, dialog.caseName()))
    {
        QMessageBox::warning(view, tr("New Case"), tr("A case with this name already exists."));
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSCreateCaseCommand(target, dialog.caseName()));
}

bool IDOSMainWindow::caseImportCase(IDOSTreeView* view)
{
    if (view == nullptr || m_project == nullptr)
    {
        return false;
    }
    QPointer<IDOSProject> target(m_project);
    const QString file = QFileDialog::getOpenFileName(view, tr("Select ECLIPSE Case"), QString(),
                                                      tr("ECLIPSE Cases (*.DATA *.data);;All Files (*)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.case.simulation.eclipse"));
    if (!reader)
    {
        IDOS_ERROR(tr("The simulation case reader is unavailable."));
        QMessageBox::warning(view, tr("Import Case"), tr("The simulation case reader is unavailable."));
        return false;
    }
    std::unique_ptr<IDOSWaitingDialog> loading(new IDOSWaitingDialog(this));
    loading->setObjectName(QStringLiteral("dataLoadingDialog"));
    loading->begin(tr("Loading Case"), tr("Reading the case and preparing its data…"), QFileInfo(file).fileName());
    const QScopedValueRollback<bool> importing(m_dataImportActive, true);
    QList<IDOSDataObject*> objects = reader->readInBackground(file);
    loading.reset();
    IDOSSimulationCaseObject* simulationCase = nullptr;
    for (IDOSDataObject* object : objects)
    {
        IDOSSimulationCaseObject* candidate = qobject_cast<IDOSSimulationCaseObject*>(object);
        if (candidate != nullptr)
        {
            simulationCase = candidate;
            break;
        }
    }
    if (target.isNull())
    {
        qDeleteAll(objects);
        return false;
    }
    if (simulationCase == nullptr)
    {
        IDOS_ERROR(tr("Simulation case parsing failed for %1: %2")
                       .arg(file, reader->lastError()));
        qDeleteAll(objects);
        QMessageBox::warning(view, tr("Import Case"), reader->lastError());
        return false;
    }
    IDOSCaseDialog dialog(simulationCase->name(), simulationCase->sourceFile(), simulationCase->pendingWellNames(),
                          target, view);
    if (dialog.exec() != QDialog::Accepted || target.isNull())
    {
        qDeleteAll(objects);
        return false;
    }
    if (caseNameExists(view, dialog.caseName()))
    {
        IDOS_WARN(tr("Case import rejected because the name already exists: %1")
                      .arg(dialog.caseName()));
        qDeleteAll(objects);
        QMessageBox::warning(view, tr("Import Case"), tr("A case with this name already exists."));
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        qDeleteAll(objects);
        return false;
    }

    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("caseName"), dialog.caseName());
    IDOSImportCaseCommand* command =
        new IDOSImportCaseCommand(target, arguments);
    if (!command->adoptParsedObjects(objects))
    {
        delete command;
        qDeleteAll(objects);
        return false;
    }
    const bool success = executeDataImport(commandManager, command, file);
    if (!success)
    {
        QMessageBox::warning(view, tr("Import Case"), commandManager->error());
    }
    return success;
}

bool IDOSMainWindow::caseDeleteCase(IDOSTreeView* view, const QString& caseId)
{
    if (view == nullptr || m_project == nullptr || caseId.isEmpty())
    {
        return false;
    }
    QPointer<IDOSProject> target(m_project);
    IDOSDataObject* obj = target->object(caseId);
    IDOSCaseObject* caseObj = qobject_cast<IDOSCaseObject*>(obj);
    if (caseObj == nullptr)
    {
        return false;
    }
    // 网格归工况私有：收集该工况引用的网格及其属性数量用于确认提示
    QStringList gridIds;
    int propertyTotal = 0;
    const QList<IDOSCaseItemRef> refs = caseObj->itemRefs();
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.grid"))
        {
            gridIds.append(ref.objectId());
            propertyTotal += caseGridPropertyCount(view, ref.objectId());
        }
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(view, tr("Delete Case"),
                               tr("Are you sure you want to delete case \"%1\"?\n"
                                  "Its %2 grid(s) and %3 propert(ies) will also be deleted. "
                                  "Unshared fluid tables imported with this case will also be deleted. "
                                  "Shared tables and referenced wells are kept.")
                                   .arg(caseObj->name())
                                   .arg(gridIds.size())
                                   .arg(propertyTotal),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeleteCaseCommand(target, caseId));
}

bool IDOSMainWindow::caseImportGrid(IDOSTreeView* view, const QString& targetCaseId)
{
    if (view == nullptr || m_project == nullptr || targetCaseId.isEmpty())
    {
        return false;
    }
    QPointer<IDOSProject> target(m_project);
    IDOSCaseObject* caseObj = qobject_cast<IDOSCaseObject*>(target->object(targetCaseId));
    if (caseObj == nullptr)
    {
        return false;
    }
    // 一个工况仅持有一个网格：已有网格时确认替换
    const QString existingGridId = caseReferencedGridId(caseObj);
    bool replacing = false;
    if (!existingGridId.isEmpty())
    {
        IDOSGridObject* existingGrid = qobject_cast<IDOSGridObject*>(target->object(existingGridId));
        const QString gridName = existingGrid != nullptr ? existingGrid->name() : existingGridId;
        const int propertyCount = caseGridPropertyCount(view, existingGridId);
        const QMessageBox::StandardButton btn = QMessageBox::question(
            view, tr("Import Grid"),
            tr("Case \"%1\" already has grid \"%2\".\n"
               "Importing a new grid will delete it along with %3 propert(ies). Continue?")
                .arg(caseObj->name())
                .arg(gridName)
                .arg(propertyCount),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn != QMessageBox::Yes || target.isNull())
        {
            return false;
        }
        replacing = true;
    }
    const QString file = QFileDialog::getOpenFileName(
        view, tr("Import Grid"), QString(),
        tr("ECLIPSE Grids (*.EGRID *.egrid *.GRID *.grid *.GRDECL *.grdecl);;All Files (*.*)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("targetCaseId"), targetCaseId);
    arguments.insert(QStringLiteral("replaceExistingGrid"), replacing);
    IDOSImportGridCommand* command =
        new IDOSImportGridCommand(target, arguments);
    const bool success = executeDataImport(commandManager, command, file);
    if (!success)
    {
        QMessageBox::warning(view, tr("Import Grid"), commandManager->error());
    }
    return success;
}

void IDOSMainWindow::onCaseRenameObject(IDOSTreeView* sourceView, const QString& requestedDataId)
{
    IDOSTreeView* view = sourceView;
    if (view == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(m_project);
    const QString id = requestedDataId;
    IDOSDataObject* object = project->object(id);
    if (object == nullptr)
    {
        return;
    }
    bool accepted = false;
    const QString name =
        QInputDialog::getText(view, tr("Rename Object"), tr("Name:"), QLineEdit::Normal, object->name(), &accepted);
    if (accepted && project != nullptr && project.data() == m_project &&
        !project->commandManager()->execute(new IDOSRenameObjectCommand(project.data(), id, name)))
    {
        QMessageBox::warning(view, tr("Rename Object"), project->commandManager()->error());
    }
}

void IDOSMainWindow::clearTreeDialogs()
{
    delete m_gridWidget.data();
    delete m_filterWidget.data();
    delete m_wellWidget.data();
}

void IDOSMainWindow::onDataGridSettingsTriggered(IDOSTreeView* view, const QString& requestedGridId,
                                                 const QString& requestedFilterId, const QString& requestedPropertyId,
                                                 bool requestedOpenFilter, IDOSRenderView* targetView)
{
    if (m_project == nullptr || view == nullptr || targetView == nullptr)
    {
        return;
    }
    const QString gridId = requestedGridId;
    const QString filterId = requestedFilterId;
    IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(m_project->object(gridId));
    IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(m_project->object(filterId));
    const QString propertyId = requestedPropertyId;
    IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(m_project->object(propertyId));
    if (grid == nullptr ||
        (!filterId.isEmpty() && (filter == nullptr || filter->gridId() != gridId)) ||
        (!propertyId.isEmpty() && (property == nullptr || property->gridId() != gridId)))
    {
        return;
    }
    const QString targetId = filterId.isEmpty() ? gridId : filterId;
    if (dataGridRenderObject(targetId, targetView) == nullptr)
    {
        m_renderServer->setChecked(targetId, Qt::Checked, targetView);
    }
    if (dataGridRenderObject(targetId, targetView) == nullptr)
    {
        return;
    }
    if (requestedOpenFilter)
    {
        delete m_filterWidget.data();
        m_filterWidget = new IDOSGridFilterWidget(view->window());
        m_filterWidget->setProject(m_project);
        m_filterWidget->setGrid(grid);
        m_filterWidget->setView(targetView);
        m_filterWidget->setFilter(filterId);
        m_filterWidget->show();
    }
    else
    {
        delete m_gridWidget.data();
        m_gridWidget = new IDOSGridSettingWidget(view->window());
        m_gridWidget->setProject(m_project);
        m_gridWidget->setGrid(grid);
        m_gridWidget->setView(targetView);
        m_gridWidget->setPropertyId(requestedPropertyId);
        if (!filterId.isEmpty())
        {
            m_gridWidget->setRenderTarget(filterId);
        }
        m_gridWidget->show();
    }
}

void IDOSMainWindow::onDataGridDisplayTriggered(IDOSTreeView* view, const QString& requestedGridId,
                                                const QString& requestedPropertyId, IDOSRenderView* targetView)
{
    Q_UNUSED(view);
    if (m_project == nullptr || targetView == nullptr)
    {
        return;
    }
    const QString propertyId = requestedPropertyId;
    IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(m_project->object(propertyId));
    if (!propertyId.isEmpty() && (property == nullptr || property->gridId() != requestedGridId))
    {
        return;
    }
    if (dataGridRenderObject(requestedGridId, targetView) == nullptr)
    {
        m_renderServer->setChecked(requestedGridId, Qt::Checked, targetView);
    }
    IDOSRenderObject* object = dataGridRenderObject(requestedGridId, targetView);
    IDOSGridRenderProvider* provider =
        object != nullptr ? dynamic_cast<IDOSGridRenderProvider*>(object->provider()) : nullptr;
    if (provider == nullptr)
    {
        return;
    }
    provider->setProperty(property);
    object->setVisible(true);
    targetView->scene()->notifyChanged(object->id());
}

void IDOSMainWindow::onDataWellSettingsTriggered(IDOSTreeView* view, const QString& requestedWellId,
                                                 bool requestedTrajectory, IDOSRenderView* targetView)
{
    Q_UNUSED(view);
    if (m_project == nullptr)
    {
        return;
    }
    IDOSWellObject* well = qobject_cast<IDOSWellObject*>(m_project->object(requestedWellId));
    if (well == nullptr || targetView == nullptr)
    {
        return;
    }
    if (m_renderServer->objectIds(targetView, requestedWellId).isEmpty())
    {
        m_renderServer->setChecked(requestedWellId, Qt::Checked, targetView);
    }
    bool available = false;
    for (IDOSRenderObject* object : targetView->scene()->objects())
    {
        if (m_renderServer->dataObject(targetView, object->id()) == well &&
            dynamic_cast<IDOSWellRenderProvider*>(object->provider()) != nullptr)
        {
            available = true;
            break;
        }
    }
    if (!available)
    {
        return;
    }
    delete m_wellWidget.data();
    m_wellWidget = new IDOSWellDisplayWidget(this);
    m_wellWidget->setProject(m_project);
    m_wellWidget->setWell(well);
    m_wellWidget->setView(targetView);
    m_wellWidget->setTrajectory(requestedTrajectory);
    m_wellWidget->show();
}

bool IDOSMainWindow::transferTreeItems(IDOSTreeModel* model, const QVariantMap& arguments, bool validateOnly)
{
    IDOSTreeView* view = model == m_dataTreeModel ? static_cast<IDOSTreeView*>(m_dataTreeView)
        : model == m_caseTreeModel ? m_caseTreeView : nullptr;
    if (view == nullptr || m_project == nullptr || m_project->commandManager() == nullptr)
    {
        return false;
    }
    QJsonObject commandArguments;
    commandArguments.insert(QStringLiteral("items"), QJsonArray::fromVariantList(arguments.value(QStringLiteral("items")).toList()));
    commandArguments.insert(QStringLiteral("targetCaseId"), arguments.value(QStringLiteral("targetCaseId")).toString());
    commandArguments.insert(QStringLiteral("targetGridId"), arguments.value(QStringLiteral("targetGridId")).toString());
    const bool move = arguments.value(QStringLiteral("move")).toBool();
    IDOSTransferObjectsCommand command(m_project, move ? QStringLiteral("move") : QStringLiteral("copy"), commandArguments);
    QString error;
    if (!command.validate(error))
    {
        return false;
    }
    if (validateOnly)
    {
        return true;
    }
    const QJsonObject response = m_project->commandManager()->execute(
        move ? QStringLiteral("objects.move") : QStringLiteral("objects.copy"), commandArguments);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        QMessageBox::warning(view, tr("Transfer Objects"), response.value(QStringLiteral("error")).toString());
        return false;
    }
    return true;
}

void IDOSMainWindow::onNodeImportDataset(IDOSTreeView* view, const QString& requestedText,
                                         const QString& requestedFileFilter, const QString& requestedProviderId,
                                         const QString& requestedDataTypeId)
{
    QPointer<IDOSProject> project(m_project);
    const QString title = requestedText;
    const QStringList files = QFileDialog::getOpenFileNames(view, title, QString(), requestedFileFilter);
    if (files.isEmpty() || project == nullptr || project != m_project || project->commandManager() == nullptr)
    {
        return;
    }
    QStringList failures;
    for (const QString& file : files)
    {
        if (project == nullptr || project != m_project)
        {
            return;
        }
        QJsonObject commandArguments;
        commandArguments.insert(QStringLiteral("filePath"), file);
        commandArguments.insert(QStringLiteral("providerId"), requestedProviderId);
        commandArguments.insert(QStringLiteral("dataTypeId"), requestedDataTypeId);
        IDOSCommandManager* manager = project->commandManager();
        if (!executeDataImport(manager, new IDOSImportDataCommand(project.data(), commandArguments), file))
        {
            failures.append(
                tr("Failed to import %1:\n%2").arg(file, manager->error()));
        }
    }
    if (!failures.isEmpty())
    {
        QMessageBox::warning(view, title, failures.join(QStringLiteral("\n\n")));
    }
}

void IDOSMainWindow::onNodeSelectSeismicInline(IDOSTreeView* view, const QString& requestedDataId,
                                               IDOSRenderView* targetView)
{
    QPointer<IDOSSeismicVolumeObject> volume(qobject_cast<IDOSSeismicVolumeObject*>(m_project->object(requestedDataId)));
    QPointer<IDOSRenderView> renderView(targetView);
    if (volume == nullptr || renderView == nullptr)
    {
        return;
    }
    IDOSRenderObject* object = nullptr;
    IDOSSeismicVolumeRenderProvider* provider = nullptr;
    for (IDOSRenderObject* candidate : renderView->scene()->objects())
    {
        IDOSSeismicVolumeRenderProvider* candidateProvider =
            dynamic_cast<IDOSSeismicVolumeRenderProvider*>(candidate->provider());
        if (candidateProvider != nullptr && candidateProvider->volume() == volume)
        {
            object = candidate;
            provider = candidateProvider;
            break;
        }
    }
    if (object == nullptr || provider == nullptr)
    {
        return;
    }
    const QString renderId = object->id();
    QStringList options;
    for (int number : provider->inlineNumbers())
    {
        options.append(QString::number(number));
    }
    if (options.isEmpty())
    {
        return;
    }
    bool accepted = false;
    const QString selected =
        QInputDialog::getItem(view, tr("Select Inline Section"), tr("Inline number:"), options,
                              options.indexOf(QString::number(provider->inlineNumber())), false, &accepted);
    if (!accepted || volume == nullptr || renderView == nullptr || m_project == nullptr ||
        m_project->object(volume->objectId()) != volume)
    {
        return;
    }
    object = renderView->scene()->object(renderId);
    provider = object != nullptr ? dynamic_cast<IDOSSeismicVolumeRenderProvider*>(object->provider()) : nullptr;
    bool valid = false;
    const int number = selected.toInt(&valid);
    if (valid && provider != nullptr && provider->volume() == volume && provider->setInlineNumber(number))
    {
        renderView->scene()->notifyChanged(renderId);
        renderView->resetCamera();
    }
}

void IDOSMainWindow::onNodeImportFluidRockTables(IDOSTreeView* view, const QString& requestedCategory)
{
    QPointer<IDOSProject> project(m_project);
    const QString category = requestedCategory;
    const bool fluid = category == QStringLiteral("fluid");
    const QString title = fluid ? tr("Import Fluid Tables") : tr("Import Rock Physics Functions");
    const QString filter = fluid
                               ? tr("ECLIPSE Fluid Tables (*.TXT *.txt *.DAT *.dat *.PRN *.prn);;All Files (*)")
                               : tr("ECLIPSE Rock Physics Tables (*.TXT *.txt *.DAT *.dat *.PRN *.prn);;All Files (*)");
    const QString file = QFileDialog::getOpenFileName(view, title, QString(), filter);
    if (file.isEmpty() || project == nullptr || project != m_project || project->commandManager() == nullptr)
    {
        return;
    }
    QJsonObject commandArguments;
    commandArguments.insert(QStringLiteral("filePath"), file);
    commandArguments.insert(QStringLiteral("category"), category);
    IDOSCommandManager* manager = project->commandManager();
    if (!executeDataImport(manager, new IDOSImportFluidRockTablesCommand(project.data(), commandArguments), file))
    {
        QMessageBox::warning(view, title, manager->error());
    }
}

void IDOSMainWindow::onNodeAddFluidRockTableToCase(IDOSTreeView* view, const QString& requestedDataId)
{
    QPointer<IDOSProject> project(m_project);
    const QString tableId = requestedDataId;
    if (qobject_cast<IDOSFluidRockTableObject*>(project->object(tableId)) == nullptr)
    {
        return;
    }
    QStringList names;
    QStringList ids;
    for (IDOSDataObject* data : project->objects())
    {
        if (qobject_cast<IDOSSimulationCaseObject*>(data) != nullptr)
        {
            names.append(data->name());
            ids.append(data->objectId());
        }
    }
    if (names.isEmpty())
    {
        QMessageBox::information(view, tr("Add Table to Simulation Case"),
                                 tr("Create or import a simulation case first."));
        return;
    }
    bool accepted = false;
    const QString name = QInputDialog::getItem(view, tr("Add Table to Simulation Case"), tr("Simulation case:"), names,
                                               0, false, &accepted);
    if (!accepted || project == nullptr || project != m_project || project->commandManager() == nullptr)
    {
        return;
    }
    const int index = names.indexOf(name);
    if (index < 0)
    {
        return;
    }
    QJsonObject commandArguments;
    commandArguments.insert(QStringLiteral("targetCaseId"), ids.at(index));
    commandArguments.insert(QStringLiteral("tableId"), tableId);
    const QJsonObject response =
        project->commandManager()->execute(QStringLiteral("case.fluidRock.attach"), commandArguments);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        QMessageBox::warning(view, tr("Add Table to Simulation Case"),
                             response.value(QStringLiteral("error")).toString());
    }
}

void IDOSMainWindow::dataImportWellPart(IDOSTreeView* view, const QString& id, const QString& providerPrefix)
{
    if (view == nullptr || m_project == nullptr || qobject_cast<IDOSWellObject*>(m_project->object(id)) == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(m_project);
    const QString importTitle = providerPrefix == QStringLiteral("idos.well.path")     ? tr("Import Well Trajectory")
                                : providerPrefix == QStringLiteral("idos.well.header") ? tr("Import Well Header")
                                : providerPrefix == QStringLiteral("idos.well.las")    ? tr("Import Well Curves")
                                : providerPrefix == QStringLiteral("idos.well.tops")   ? tr("Import Well Markers")
                                : providerPrefix == QStringLiteral("idos.well.completion")
                                    ? tr("Import Well Completions")
                                    : tr("Import Well Checkshots");
    const QString file = QFileDialog::getOpenFileName(view, importTitle, QString(),
                                                      IDOSDataProviderRegistry::instance().fileFilters(providerPrefix));
    if (file.isEmpty() || project.isNull() || project != m_project)
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("targetWellId"), id);
    IDOSCommandManager* manager = project->commandManager();
    if (manager == nullptr || !executeDataImport(manager, new IDOSImportWellDataCommand(project.data(), arguments), file))
    {
        if (manager != nullptr)
        {
            QMessageBox::warning(view, importTitle, manager->error());
        }
    }
}

void IDOSMainWindow::onDataImportWellHeader(IDOSTreeView* view, const QString& dataId)
{
    dataImportWellPart(view, dataId, QStringLiteral("idos.well.header"));
}

void IDOSMainWindow::onDataImportWellTrajectory(IDOSTreeView* view, const QString& dataId)
{
    dataImportWellPart(view, dataId, QStringLiteral("idos.well.path"));
}

void IDOSMainWindow::onDataImportWellCurves(IDOSTreeView* view, const QString& dataId)
{
    dataImportWellPart(view, dataId, QStringLiteral("idos.well.las"));
}

void IDOSMainWindow::onDataImportWellMarkers(IDOSTreeView* view, const QString& dataId)
{
    dataImportWellPart(view, dataId, QStringLiteral("idos.well.tops"));
}

void IDOSMainWindow::onDataImportWellCompletions(IDOSTreeView* view, const QString& dataId)
{
    dataImportWellPart(view, dataId, QStringLiteral("idos.well.completion"));
}

void IDOSMainWindow::onDataImportWellCheckshots(IDOSTreeView* view, const QString& dataId)
{
    dataImportWellPart(view, dataId, QStringLiteral("idos.well.checkshot"));
}
