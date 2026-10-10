#include <memory>

#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QCloseEvent>
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
#include <QPushButton>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QStringList>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>

#include <DockManager.h>
#include <DockWidget.h>
#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonPanel.h>
#include <SARibbonQuickAccessBar.h>
#include <SARibbonSystemButtonBar.h>

#include "command/idoscommandmanager.h"
#include "command/idoscommandmetadata.h"
#include "command/idoscommandregistry.h"
#include "command/idoscreatecasecommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosimportcommands.h"
#include "command/idosrenameobjectcommand.h"
#include "idosappinterface.h"
#include "idosapplication.h"
#include "idosassistantwidget.h"
#include "idoscaseobject.h"
#include "idoscasetreemenuprovider.h"
#include "idoscasetreemodel.h"
#include "idoscasetreeview.h"
#include "idosdataobjecthandling.h"
#include "idosdatatreemenuprovider.h"
#include "idosdatatreemodel.h"
#include "idosdatatreeview.h"
#include "idosdebuginfowidget.h"
#include "idosgriddatatreeprovider.h"
#include "idosgridrenderobjectprovider.h"
#include "idosnewprojectdialog.h"
#include "idosobjecttreenode.h"
#include "idosproject.h"
#include "idosprojectmetadata.h"
#include "plugin/idospluginmanagerwidget.h"
#include "plugin/idospluginregistry.h"
#include "idosrenderserver.h"
#include "idosrendertypes.h"
#include "idosruntimeinfowidget.h"
#include "idossimulationcasetreeprovider.h"
#include "idostreepartnode.h"
#include "idostreeproviderregistry.h"
#include "idostreereferencenode.h"
#include "idoswell.h"
#include "idoswelldatatreeprovider.h"
#include "idoswelllogtrackview.h"
#include "idoswellrenderobjectprovider.h"
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
    , m_actionNew3DWindow(nullptr)
    , m_actionNew2DWindow(nullptr)
    , m_actionWindowLayout(nullptr)
    , m_project(nullptr)
    , m_commandManager(nullptr)
    , m_dataTreeModel(new IDOSDataTreeModel(this))
    , m_caseTreeModel(new IDOSCaseTreeModel(this))
    , m_dataTreeView(nullptr)
    , m_caseTreeView(nullptr)
    , m_treeProviderRegistry(new IDOSTreeProviderRegistry())
    , m_renderServer(new IDOSRenderServer(this))
    , m_runtimeInfoWidget(nullptr)
    , m_pluginRegistry(nullptr)
    , m_pluginManagerWidget(nullptr)
    , m_appInterface(new IDOSAppInterface(this))
    , m_debugInfoWidget(nullptr)
    , m_assistantWidget(nullptr)
    , m_dockManager(nullptr)
    , m_renderDock(nullptr)
    , m_outputDock(nullptr)
    , m_debugDock(nullptr)
    , m_assistantDock(nullptr)
    , m_viewCaseIds()
    , m_nextViewIndex(1)
    , m_currentWindowLayout(QStringLiteral("single"))
{
    initProviders();
    initMainWindow();
    initDockManager();
    initRenderView();
    initRibbonAction();

    initDockWidgets();
    IDOS_MESSAGE(tr("Application started."), IDOSLogLevel::Info);
    initPluginManager();
}

void IDOSMainWindow::initProviders()
{
    m_treeProviderRegistry->registerDataProvider(new IDOSWellDataTreeProvider());
    m_treeProviderRegistry->registerDataProvider(new IDOSGridDataTreeProvider());
    m_treeProviderRegistry->registerCaseProvider(new IDOSSimulationCaseTreeProvider());
    m_dataTreeModel->setTreeProviderRegistry(m_treeProviderRegistry);
    m_caseTreeModel->setTreeProviderRegistry(m_treeProviderRegistry);
    m_renderServer->registerMetadata(new IDOSGridRenderObjectProvider());
    m_renderServer->registerMetadata(new IDOSWellRenderObjectProvider());
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
    QWidget* renderViewWidget = m_renderServer->createView(QStringLiteral("main3d"), false);
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
    m_dataTreeView = new IDOSDataTreeView(this);
    m_dataTreeView->setModel(m_dataTreeModel);
    connect(m_dataTreeView, &QTreeView::doubleClicked, this, &IDOSMainWindow::onDataTreeItemActivated);
    connect(m_dataTreeView->selectionModel(), &QItemSelectionModel::currentChanged,this, &IDOSMainWindow::onDataTreeCurrentChanged);

    IDOSDataTreeMenuProvider* provider = new IDOSDataTreeMenuProvider(m_dataTreeView);
    m_dataTreeView->setMenuProvider(provider);
    ads::CDockWidget* dataDock = new ads::CDockWidget(m_dockManager, tr("Data"));
    dataDock->setObjectName(QStringLiteral("dataTreeDock"));
    dataDock->setIcon(QIcon(QStringLiteral(":/images/gui-data-tree.svg")));
    dataDock->setWidget(m_dataTreeView);
    dataDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    ads::CDockAreaWidget* dataArea = m_dockManager->addDockWidget(ads::LeftDockWidgetArea, dataDock);

    m_runtimeInfoWidget = new IDOSRuntimeInfoWidget(this);
    m_outputDock = new ads::CDockWidget(m_dockManager, tr("Output"));
    m_outputDock->setObjectName(QStringLiteral("outputDock"));
    m_outputDock->setWidget(m_runtimeInfoWidget, ads::CDockWidget::ForceNoScrollArea);
    m_outputDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    ads::CDockAreaWidget* outputArea = m_dockManager->addDockWidget(ads::BottomDockWidgetArea, m_outputDock, m_renderDock->dockAreaWidget());

    m_debugInfoWidget = new IDOSDebugInfoWidget(this);
    m_debugDock = new ads::CDockWidget(m_dockManager, tr("Debug"));
    m_debugDock->setObjectName(QStringLiteral("debugDock"));
    m_debugDock->setWidget(m_debugInfoWidget, ads::CDockWidget::ForceNoScrollArea);
    m_debugDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidgetTabToArea(m_debugDock, outputArea);

    m_assistantWidget = new IDOSAssistantWidget(this);
    IDOSCommandRegistry* commandRegistry = m_assistantWidget->commandRegistry();
    if (commandRegistry != nullptr)
    {
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateWellCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteGridCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeletePropertyCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteWellCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSRenameObjectCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportCaseCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportGridCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportPropertyCommandMetadata()));
        commandRegistry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportWellDataCommandMetadata()));
    }
    m_assistantDock = new ads::CDockWidget(m_dockManager, tr("Assistant"));
    m_assistantDock->setObjectName(QStringLiteral("assistantDock"));
    m_assistantDock->setWidget(m_assistantWidget, ads::CDockWidget::ForceNoScrollArea);
    m_assistantDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidgetTabToArea(m_assistantDock, outputArea);

    // ---- 工况树 ----
    m_caseTreeView = new IDOSCaseTreeView(this);
    m_caseTreeView->setModel(m_caseTreeModel);
    connect(m_caseTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &IDOSMainWindow::onCaseTreeCurrentChanged);
    connect(m_renderServer, &IDOSRenderServer::titleChanged, m_renderDock, &ads::CDockWidget::setWindowTitle);
    IDOSCaseTreeMenuProvider* caseMenu = new IDOSCaseTreeMenuProvider(m_caseTreeView);
    m_caseTreeView->setMenuProvider(caseMenu);
    ads::CDockWidget* caseDock = new ads::CDockWidget(m_dockManager, tr("Case"));
    caseDock->setObjectName(QStringLiteral("caseTreeDock"));
    caseDock->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    caseDock->setWidget(m_caseTreeView);
    caseDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidget(ads::BottomDockWidgetArea, caseDock, dataArea);

    updateTreeSelection(m_renderServer->activeViewId());
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
    frontViewAction->setData(static_cast<int>(IDOSOrientation::Front));
    viewPresetActionGroup->addAction(frontViewAction);
    QAction* backViewAction = viewPresetsMenu->addAction(tr("Back"));
    backViewAction->setCheckable(true);
    backViewAction->setData(static_cast<int>(IDOSOrientation::Back));
    viewPresetActionGroup->addAction(backViewAction);
    QAction* leftViewAction = viewPresetsMenu->addAction(tr("Left"));
    leftViewAction->setCheckable(true);
    leftViewAction->setData(static_cast<int>(IDOSOrientation::Left));
    viewPresetActionGroup->addAction(leftViewAction);
    QAction* rightViewAction = viewPresetsMenu->addAction(tr("Right"));
    rightViewAction->setCheckable(true);
    rightViewAction->setData(static_cast<int>(IDOSOrientation::Right));
    viewPresetActionGroup->addAction(rightViewAction);
    QAction* topViewAction = viewPresetsMenu->addAction(tr("Top"));
    topViewAction->setCheckable(true);
    topViewAction->setData(static_cast<int>(IDOSOrientation::Top));
    viewPresetActionGroup->addAction(topViewAction);
    QAction* bottomViewAction = viewPresetsMenu->addAction(tr("Bottom"));
    bottomViewAction->setCheckable(true);
    bottomViewAction->setData(static_cast<int>(IDOSOrientation::Bottom));
    viewPresetActionGroup->addAction(bottomViewAction);
    QAction* isometricViewAction = viewPresetsMenu->addAction(tr("Isometric"));
    isometricViewAction->setCheckable(true);
    isometricViewAction->setData(static_cast<int>(IDOSOrientation::Isometric));
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

    SARibbonPanel* extensionPanel = projectPage->addPanel(tr("Extensions"));
    m_actionAssistant = new QAction(
        QIcon(QStringLiteral(":/images/assistant-chat.svg")), tr("AI Assistant"), this);
    m_actionAssistant->setObjectName(QStringLiteral("aiAssistantAction"));
    extensionPanel->addSmallAction(m_actionAssistant);
    extensionPanel->addSmallAction(m_actionPluginManager);
    QAction* pythonConsoleAction = new QAction(
        QIcon(QStringLiteral(":/images/python-console.svg")), tr("Python Console"), this);
    pythonConsoleAction->setObjectName(QStringLiteral("pythonConsoleAction"));
    pythonConsoleAction->setEnabled(false);
    extensionPanel->addSmallAction(pythonConsoleAction);

    SARibbonPanel* outputPanel = projectPage->addPanel(tr("Output"));
    m_actionCaptureScreenshot = new QAction(
        QIcon(QStringLiteral(":/images/render-view.svg")), tr("Capture Screenshot"), this);
    m_actionCaptureScreenshot->setObjectName(QStringLiteral("captureScreenshotAction"));
    outputPanel->addSmallAction(m_actionCaptureScreenshot);
    m_actionExportImage = new QAction(
        QIcon(QStringLiteral(":/images/app-export.svg")), tr("Export Image"), this);
    m_actionExportImage->setObjectName(QStringLiteral("exportImageAction"));
    outputPanel->addSmallAction(m_actionExportImage);

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
    connect(m_actionNew3DWindow, &QAction::triggered,
            this, &IDOSMainWindow::onNew3DWindowTriggered);
    connect(m_actionNew2DWindow, &QAction::triggered,
            this, &IDOSMainWindow::onNew2DWindowTriggered);
    connect(windowLayoutMenu, &QMenu::triggered,
            this, &IDOSMainWindow::onWindowLayoutTriggered);
    connect(gridLayoutMenu, &QMenu::triggered,
            this, &IDOSMainWindow::onWindowLayoutTriggered);
    connect(m_renderServer, &IDOSRenderServer::currentViewChanged,
            this, &IDOSMainWindow::onCurrentViewChanged);
    connect(m_renderServer, &IDOSRenderServer::activeViewDecorationsChanged,
            this, &IDOSMainWindow::onViewDecorationsChanged);
    connect(m_renderServer, &IDOSRenderServer::renderViewObjectActivated,
            this, &IDOSMainWindow::onRenderObjectActivated);
    connect(m_renderServer, &IDOSRenderServer::activeViewContextObjectChanged,
            this, &IDOSMainWindow::onActiveViewContextObjectChanged);
    onCurrentViewChanged(m_renderServer->activeViewId());
    onViewDecorationsChanged(m_renderServer->activeViewOrientationMarkerVisible(),
                             m_renderServer->activeViewLegendAvailable(),
                             m_renderServer->activeViewLegendVisible());
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
    m_dataTreeModel->setProject(nullptr);
    m_caseTreeModel->setProject(nullptr);
    if (m_project)
    {
        disconnect(m_project, nullptr, this, nullptr);
    }
    disconnect(m_commandManager.data(), nullptr, this, nullptr);
    delete m_treeProviderRegistry;
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
    disconnect(m_commandManager.data(), nullptr, this, nullptr);
    m_project = project;
    m_viewCaseIds.clear();
    m_commandManager = project != nullptr ? project->commandManager() : nullptr;
    m_renderServer->setProject(project);
    m_assistantWidget->setProject(project);
    m_dataTreeModel->setProject(project);
    m_caseTreeModel->setProject(project);
    updateTreeSelection(m_renderServer->activeViewId());
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
}

IDOSDataTreeModel* IDOSMainWindow::dataTreeModel() const
{
    return m_dataTreeModel;
}

IDOSCaseTreeModel* IDOSMainWindow::caseTreeModel() const
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
    if (action == nullptr || m_renderServer == nullptr)
    {
        return;
    }

    if (!m_renderServer->hasActiveView())
    {
        return;
    }

    const int presetValue = action->data().toInt();
    const IDOSOrientation orientation = static_cast<IDOSOrientation>(presetValue);
    m_renderServer->setActiveViewOrientation(orientation);
}

void IDOSMainWindow::onCurrentViewChanged(const QString& viewId)
{
    const bool hasActiveView = m_renderServer->hasActiveView();
    m_actionViewPresets->setEnabled(hasActiveView);
    m_actionResetView->setEnabled(hasActiveView);
    m_actionFitAll->setEnabled(hasActiveView);
    m_actionBackgroundColor->setEnabled(hasActiveView);
    m_actionCaptureScreenshot->setEnabled(hasActiveView);
    m_actionExportImage->setEnabled(hasActiveView);
    updateTreeSelection(viewId);
}

void IDOSMainWindow::onActiveViewContextObjectChanged(const QString& viewId,
                                                       const QString& objectId)
{
    Q_UNUSED(objectId)
    if (viewId == m_renderServer->activeViewId())
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
            m_renderServer->setActiveView(viewIterator.key());
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
    m_actionOrientationMarker->setEnabled(m_renderServer->hasActiveView());
    m_actionOrientationMarker->setChecked(orientationMarkerVisible);
    m_actionLegend->setEnabled(legendAvailable);
    m_actionLegend->setChecked(legendVisible);
}

void IDOSMainWindow::onOrientationMarkerToggled(bool checked)
{
    m_renderServer->setActiveViewOrientationMarkerVisible(checked);
}

void IDOSMainWindow::onLegendToggled(bool checked)
{
    m_renderServer->setActiveViewLegendVisible(checked);
}

void IDOSMainWindow::onResetViewTriggered()
{
    if (m_renderServer == nullptr)
    {
        return;
    }

    if (m_renderServer->hasActiveView())
    {
        m_renderServer->setActiveViewOrientation(m_renderServer->activeViewParallelProjection()
                                                     ? IDOSOrientation::Top
                                                     : IDOSOrientation::Isometric);
    }
}

void IDOSMainWindow::onFitAllTriggered()
{
    if (m_renderServer->hasActiveView())
    {
        m_renderServer->resetActiveViewCamera();
    }
}

void IDOSMainWindow::onBackgroundColorTriggered()
{
    if (!m_renderServer->hasActiveView())
    {
        return;
    }

    const QColor selectedColor = QColorDialog::getColor(
        m_renderServer->activeViewBackgroundColor(), this, tr("Background Color"));
    if (selectedColor.isValid())
    {
        m_renderServer->setActiveViewBackgroundColor(selectedColor);
    }
}

void IDOSMainWindow::onCaptureScreenshotTriggered()
{
    if (!m_renderServer->hasActiveView())
    {
        return;
    }

    const QImage screenshot = m_renderServer->captureActiveViewImage();
    if (screenshot.isNull())
    {
        return;
    }

    QApplication::clipboard()->setImage(screenshot);
    m_renderServer->flashActiveViewScreenshot();
    statusBar()->showMessage(tr("Screenshot copied to clipboard."), 3000);
}

void IDOSMainWindow::onExportImageTriggered()
{
    if (!m_renderServer->hasActiveView())
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

    const QImage image = m_renderServer->captureActiveViewImage();
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
                m_renderServer->removeView(viewId);
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

void IDOSMainWindow::createRenderView(bool parallelProjection)
{
    if (m_dockManager == nullptr || m_renderServer == nullptr)
    {
        return;
    }

    const int viewIndex = m_nextViewIndex++;
    const QString viewId = QStringLiteral("view_%1").arg(viewIndex, 4, 10, QLatin1Char('0'));
    QWidget* renderViewWidget = m_renderServer->createView(viewId, parallelProjection);
    if (renderViewWidget == nullptr)
    {
        return;
    }
    const QString viewTitle = parallelProjection
                                  ? tr("2D View") + QStringLiteral(" %1").arg(viewIndex)
                                  : tr("3D View") + QStringLiteral(" %1").arg(viewIndex);
    ads::CDockWidget* dock = new ads::CDockWidget(m_dockManager, viewTitle);
    dock->setObjectName(QStringLiteral("renderViewDock_%1").arg(viewIndex));
    dock->setIcon(QIcon(parallelProjection
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
    m_renderServer->setActiveView(viewId);

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

    QString objectId;
    if (m_project != nullptr && current.isValid())
    {
        IDOSTreeNode* node = m_dataTreeModel->nodeFromIndex(current);
        IDOSObjectTreeNode* objectNode = dynamic_cast<IDOSObjectTreeNode*>(node);
        if (objectNode != nullptr)
        {
            objectId = objectNode->objectId();
        }
        else
        {
            IDOSTreePartNode* partNode = dynamic_cast<IDOSTreePartNode*>(node);
            if (partNode != nullptr)
            {
                objectId = partNode->ownerObjectId();
            }
        }
    }

    const QString viewId = m_renderServer->activeViewId();
    if (!viewId.isEmpty())
    {
        const QModelIndex caseIndex = objectId.isEmpty()
                                          ? QModelIndex()
                                          : m_caseTreeModel->indexFromReferencedObjectId(objectId);
        const QString caseId = caseIdFromCaseTreeIndex(caseIndex);
        setViewTreeContext(viewId, objectId, caseId);
        updateTreeSelection(viewId);
    }
}

void IDOSMainWindow::onCaseTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    const QString viewId = m_renderServer->activeViewId();
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
    m_renderServer->setActiveView(viewId);
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

    const QString objectId = viewId == m_renderServer->activeViewId()
                                 ? m_renderServer->activeViewContextObjectId()
                                 : QString();
    const QString caseId = m_viewCaseIds.value(viewId);
    const QModelIndex dataIndex = objectId.isEmpty()
                                      ? QModelIndex()
                                      : m_dataTreeModel->indexFromObjectId(objectId);
    const QModelIndex caseIndex = caseTreeIndexForContext(objectId, caseId);
    selectTreeIndex(m_dataTreeView, dataIndex);
    selectTreeIndex(m_caseTreeView, caseIndex);
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

    if (viewId == m_renderServer->activeViewId())
    {
        m_renderServer->setActiveViewContextObjectId(objectId);
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
        const QModelIndex caseIndex = m_caseTreeModel->indexFromObjectId(caseId);
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
        return m_caseTreeModel->indexFromReferencedObjectId(objectId);
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
        IDOSTreeReferenceNode* referenceNode =
            dynamic_cast<IDOSTreeReferenceNode*>(m_caseTreeModel->nodeFromIndex(childIndex));
        if (referenceNode != nullptr && referenceNode->itemRef().objectId() == objectId)
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
    if (!index.isValid())
    {
        return QString();
    }

    IDOSTreeNode* node = m_caseTreeModel->nodeFromIndex(index);
    IDOSTreeReferenceNode* referenceNode = dynamic_cast<IDOSTreeReferenceNode*>(node);
    if (referenceNode != nullptr)
    {
        return referenceNode->itemRef().objectId();
    }

    IDOSTreePartNode* partNode = dynamic_cast<IDOSTreePartNode*>(node);
    if (partNode != nullptr)
    {
        return partNode->ownerObjectId();
    }

    IDOSObjectTreeNode* objectNode = dynamic_cast<IDOSObjectTreeNode*>(node);
    if (objectNode != nullptr && m_project != nullptr &&
        qobject_cast<IDOSCaseObject*>(m_project->objectById(objectNode->objectId())) == nullptr)
    {
        return objectNode->objectId();
    }
    return QString();
}

QString IDOSMainWindow::caseIdFromCaseTreeIndex(const QModelIndex& index) const
{
    QModelIndex currentIndex = index;
    while (currentIndex.isValid())
    {
        IDOSTreeNode* node = m_caseTreeModel->nodeFromIndex(currentIndex);
        IDOSObjectTreeNode* objectNode = dynamic_cast<IDOSObjectTreeNode*>(node);
        if (objectNode != nullptr && m_project != nullptr &&
            qobject_cast<IDOSCaseObject*>(m_project->objectById(objectNode->objectId())) != nullptr)
        {
            return objectNode->objectId();
        }
        currentIndex = currentIndex.parent();
    }
    return QString();
}

void IDOSMainWindow::onDataTreeItemActivated(const QModelIndex& index)
{
    if (m_project == nullptr)
    {
        return;
    }

    IDOSTreePartNode* partNode = dynamic_cast<IDOSTreePartNode*>(m_dataTreeModel->nodeFromIndex(index));
    if (partNode == nullptr || partNode->partKey().toString() != QStringLiteral("idos.well.logs"))
    {
        return;
    }

    IDOSWell* well = qobject_cast<IDOSWell*>(m_project->objectById(partNode->ownerObjectId()));
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
    m_dataTreeModel->setProject(nullptr);
    m_caseTreeModel->setProject(nullptr);
    m_renderServer->setProject(nullptr);
    m_assistantWidget->setProject(nullptr);
    updateTreeSelection(m_renderServer->activeViewId());
    onCommandStateChanged();
}
