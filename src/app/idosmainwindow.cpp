#include <memory>

#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QDialog>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QTreeView>

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
#include "idospropertywidget.h"
#include "idosrenderserver.h"
#include "idosrenderview.h"
#include "idosruntimeinfowidget.h"
#include "idossimulationcasetreeprovider.h"
#include "idostreepartnode.h"
#include "idostreeproviderregistry.h"
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
    , m_project(nullptr)
    , m_commandManager(nullptr)
    , m_dataTreeModel(new IDOSDataTreeModel(this))
    , m_caseTreeModel(new IDOSCaseTreeModel(this))
    , m_dataTreeView(nullptr)
    , m_caseTreeView(nullptr)
    , m_treeProviderRegistry(new IDOSTreeProviderRegistry())
    , m_renderServer(new IDOSRenderServer(this))
    , m_renderView(nullptr)
    , m_propertyWidget(nullptr)
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
    m_renderServer->addProvider(new IDOSGridRenderObjectProvider());
    m_renderServer->addProvider(new IDOSWellRenderObjectProvider());
}

void IDOSMainWindow::initMainWindow()
{
    setWindowTitle(tr("IDOS"));
    setWindowIcon(QIcon(":/images/app-logo.svg"));
}

void IDOSMainWindow::initDockManager()
{
    m_dockManager = new ads::CDockManager(this);
    m_dockManager->setStyleSheet(QString());
    setCentralWidget(m_dockManager);
}

void IDOSMainWindow::initRenderView()
{
    m_renderView = new IDOSRenderView(this);
    m_renderDock = new ads::CDockWidget(m_dockManager, tr("3D View"));
    m_renderDock->setObjectName(QStringLiteral("central3DDock"));
    m_renderDock->setIcon(QIcon(QStringLiteral(":/images/render-view.svg")));
    m_renderDock->setWidget(m_renderView);
    m_renderDock->setFeatures(ads::CDockWidget::NoDockWidgetFeatures);
    m_dockManager->setCentralWidget(m_renderDock);
    m_renderServer->addView(QStringLiteral("main3d"), m_renderView);
    connect(m_renderView, &IDOSRenderView::objectActivated,
            this, &IDOSMainWindow::onRenderObjectActivated);
}

void IDOSMainWindow::initDockWidgets()
{
    m_dataTreeView = new IDOSDataTreeView(this);
    m_dataTreeView->setModel(m_dataTreeModel);
    connect(m_dataTreeView, &QTreeView::doubleClicked, this, &IDOSMainWindow::onDataTreeItemActivated);
    connect(m_dataTreeView->selectionModel(), &QItemSelectionModel::currentChanged,this, &IDOSMainWindow::onDataTreeCurrentChanged);
    connect(m_dataTreeModel, &IDOSDataTreeModel::itemCheckedChanged, m_renderServer, &IDOSRenderServer::onItemCheckedChanged);

    IDOSDataTreeMenuProvider* provider = new IDOSDataTreeMenuProvider(m_dataTreeView);
    m_dataTreeView->setMenuProvider(provider);
    ads::CDockWidget* dataDock = new ads::CDockWidget(m_dockManager, tr("Data"));
    dataDock->setObjectName(QStringLiteral("dataTreeDock"));
    dataDock->setIcon(QIcon(QStringLiteral(":/images/gui-data-tree.svg")));
    dataDock->setWidget(m_dataTreeView);
    dataDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    ads::CDockAreaWidget* dataArea = m_dockManager->addDockWidget(ads::LeftDockWidgetArea, dataDock);

    m_propertyWidget = new IDOSPropertyWidget(this);
    ads::CDockWidget* propertyDock = new ads::CDockWidget(m_dockManager, tr("Properties"));
    propertyDock->setObjectName(QStringLiteral("propertyDock"));
    propertyDock->setWidget(m_propertyWidget, ads::CDockWidget::ForceNoScrollArea);
    propertyDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidget(ads::RightDockWidgetArea, propertyDock);

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
    connect(m_caseTreeModel, &IDOSCaseTreeModel::itemCheckedChanged, m_renderServer, &IDOSRenderServer::onItemCheckedChanged);
    connect(m_renderServer, &IDOSRenderServer::titleChanged, m_renderDock, &ads::CDockWidget::setWindowTitle);
    IDOSCaseTreeMenuProvider* caseMenu = new IDOSCaseTreeMenuProvider(m_caseTreeView);
    m_caseTreeView->setMenuProvider(caseMenu);
    ads::CDockWidget* caseDock = new ads::CDockWidget(m_dockManager, tr("Case"));
    caseDock->setObjectName(QStringLiteral("caseTreeDock"));
    caseDock->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    caseDock->setWidget(m_caseTreeView);
    caseDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidget(ads::BottomDockWidgetArea, caseDock, dataArea);
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
    frontViewAction->setData(static_cast<int>(IDOSRenderView::ViewPreset::Front));
    viewPresetActionGroup->addAction(frontViewAction);
    QAction* backViewAction = viewPresetsMenu->addAction(tr("Back"));
    backViewAction->setCheckable(true);
    backViewAction->setData(static_cast<int>(IDOSRenderView::ViewPreset::Back));
    viewPresetActionGroup->addAction(backViewAction);
    QAction* leftViewAction = viewPresetsMenu->addAction(tr("Left"));
    leftViewAction->setCheckable(true);
    leftViewAction->setData(static_cast<int>(IDOSRenderView::ViewPreset::Left));
    viewPresetActionGroup->addAction(leftViewAction);
    QAction* rightViewAction = viewPresetsMenu->addAction(tr("Right"));
    rightViewAction->setCheckable(true);
    rightViewAction->setData(static_cast<int>(IDOSRenderView::ViewPreset::Right));
    viewPresetActionGroup->addAction(rightViewAction);
    QAction* topViewAction = viewPresetsMenu->addAction(tr("Top"));
    topViewAction->setCheckable(true);
    topViewAction->setData(static_cast<int>(IDOSRenderView::ViewPreset::Top));
    viewPresetActionGroup->addAction(topViewAction);
    QAction* bottomViewAction = viewPresetsMenu->addAction(tr("Bottom"));
    bottomViewAction->setCheckable(true);
    bottomViewAction->setData(static_cast<int>(IDOSRenderView::ViewPreset::Bottom));
    viewPresetActionGroup->addAction(bottomViewAction);
    QAction* isometricViewAction = viewPresetsMenu->addAction(tr("Isometric"));
    isometricViewAction->setCheckable(true);
    isometricViewAction->setData(static_cast<int>(IDOSRenderView::ViewPreset::Isometric));
    viewPresetActionGroup->addAction(isometricViewAction);
    m_actionViewPresets->setMenu(viewPresetsMenu);
    viewPanel->addSmallAction(m_actionViewPresets, QToolButton::InstantPopup);

    m_actionResetView = new QAction(
        QIcon(QStringLiteral(":/images/render-fit.svg")), tr("Reset View"), this);
    m_actionResetView->setObjectName(QStringLiteral("resetViewAction"));
    viewPanel->addSmallAction(m_actionResetView);

    QAction* fitAllAction = new QAction(
        QIcon(QStringLiteral(":/images/render-fit.svg")), tr("Fit All"), this);
    fitAllAction->setObjectName(QStringLiteral("fitAllAction"));
    fitAllAction->setEnabled(false);
    viewPanel->addSmallAction(fitAllAction);
    QAction* backgroundColorAction = new QAction(
        QIcon(QStringLiteral(":/images/render-scene.svg")), tr("Background Color"), this);
    backgroundColorAction->setObjectName(QStringLiteral("backgroundColorAction"));
    backgroundColorAction->setEnabled(false);
    viewPanel->addSmallAction(backgroundColorAction);

    SARibbonPanel* windowPanel = projectPage->addPanel(tr("Window"));
    QAction* new3DWindowAction = new QAction(
        QIcon(QStringLiteral(":/images/render-view.svg")), tr("New 3D Window"), this);
    new3DWindowAction->setObjectName(QStringLiteral("new3DWindowAction"));
    new3DWindowAction->setEnabled(false);
    windowPanel->addSmallAction(new3DWindowAction);
    QAction* new2DWindowAction = new QAction(
        QIcon(QStringLiteral(":/images/gui-case-views.svg")), tr("New 2D Window"), this);
    new2DWindowAction->setObjectName(QStringLiteral("new2DWindowAction"));
    new2DWindowAction->setEnabled(false);
    windowPanel->addSmallAction(new2DWindowAction);
    QAction* windowLayoutAction = new QAction(
        QIcon(QStringLiteral(":/images/gui-data-tree.svg")), tr("Window Layout"), this);
    windowLayoutAction->setObjectName(QStringLiteral("windowLayoutAction"));
    QMenu* windowLayoutMenu = new QMenu(this);
    QAction* singleWindowAction = windowLayoutMenu->addAction(
        QIcon(QStringLiteral(":/images/render-view.svg")), tr("Single Window"));
    singleWindowAction->setObjectName(QStringLiteral("singleWindowAction"));
    singleWindowAction->setEnabled(false);
    QAction* sideBySideWindowsAction = windowLayoutMenu->addAction(
        QIcon(QStringLiteral(":/images/render-section.svg")), tr("Side by Side"));
    sideBySideWindowsAction->setObjectName(QStringLiteral("sideBySideWindowsAction"));
    sideBySideWindowsAction->setEnabled(false);
    QAction* stackWindowsAction = windowLayoutMenu->addAction(
        QIcon(QStringLiteral(":/images/render-section-position.svg")), tr("Stack Windows"));
    stackWindowsAction->setObjectName(QStringLiteral("stackWindowsAction"));
    stackWindowsAction->setEnabled(false);
    windowLayoutMenu->addSeparator();
    QMenu* gridLayoutMenu = windowLayoutMenu->addMenu(
        QIcon(QStringLiteral(":/images/gui-data-tree.svg")), tr("Grid Layout"));
    gridLayoutMenu->setObjectName(QStringLiteral("gridLayoutMenu"));
    QAction* gridLayout1x2Action = gridLayoutMenu->addAction(tr("1 × 2"));
    gridLayout1x2Action->setObjectName(QStringLiteral("gridLayout1x2Action"));
    gridLayout1x2Action->setEnabled(false);
    QAction* gridLayout2x1Action = gridLayoutMenu->addAction(tr("2 × 1"));
    gridLayout2x1Action->setObjectName(QStringLiteral("gridLayout2x1Action"));
    gridLayout2x1Action->setEnabled(false);
    QAction* gridLayout2x2Action = gridLayoutMenu->addAction(tr("2 × 2"));
    gridLayout2x2Action->setObjectName(QStringLiteral("gridLayout2x2Action"));
    gridLayout2x2Action->setEnabled(false);
    QAction* gridLayout2x3Action = gridLayoutMenu->addAction(tr("2 × 3"));
    gridLayout2x3Action->setObjectName(QStringLiteral("gridLayout2x3Action"));
    gridLayout2x3Action->setEnabled(false);
    QAction* gridLayout3x2Action = gridLayoutMenu->addAction(tr("3 × 2"));
    gridLayout3x2Action->setObjectName(QStringLiteral("gridLayout3x2Action"));
    gridLayout3x2Action->setEnabled(false);
    QAction* gridLayout3x3Action = gridLayoutMenu->addAction(tr("3 × 3"));
    gridLayout3x3Action->setObjectName(QStringLiteral("gridLayout3x3Action"));
    gridLayout3x3Action->setEnabled(false);
    windowLayoutAction->setMenu(windowLayoutMenu);
    windowPanel->addSmallAction(windowLayoutAction, QToolButton::InstantPopup);

    SARibbonPanel* extensionPanel = projectPage->addPanel(tr("Extensions"));
    QAction* aiAssistantAction = new QAction(
        QIcon(QStringLiteral(":/images/assistant-chat.svg")), tr("AI Assistant"), this);
    aiAssistantAction->setObjectName(QStringLiteral("aiAssistantAction"));
    aiAssistantAction->setEnabled(false);
    extensionPanel->addSmallAction(aiAssistantAction);
    extensionPanel->addSmallAction(m_actionPluginManager);
    QAction* pythonConsoleAction = new QAction(
        QIcon(QStringLiteral(":/images/python-console.svg")), tr("Python Console"), this);
    pythonConsoleAction->setObjectName(QStringLiteral("pythonConsoleAction"));
    pythonConsoleAction->setEnabled(false);
    extensionPanel->addSmallAction(pythonConsoleAction);

    SARibbonPanel* outputPanel = projectPage->addPanel(tr("Output"));
    QAction* captureScreenshotAction = new QAction(
        QIcon(QStringLiteral(":/images/render-view.svg")), tr("Capture Screenshot"), this);
    captureScreenshotAction->setObjectName(QStringLiteral("captureScreenshotAction"));
    captureScreenshotAction->setEnabled(false);
    outputPanel->addSmallAction(captureScreenshotAction);
    QAction* exportImageAction = new QAction(
        QIcon(QStringLiteral(":/images/app-export.svg")), tr("Export Image"), this);
    exportImageAction->setObjectName(QStringLiteral("exportImageAction"));
    exportImageAction->setEnabled(false);
    outputPanel->addSmallAction(exportImageAction);

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
    connect(m_renderServer, &IDOSRenderServer::currentViewChanged,
            this, &IDOSMainWindow::onCurrentViewChanged);
    onCurrentViewChanged(m_renderServer->activeViewId());
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
    m_commandManager = project != nullptr ? project->commandManager() : nullptr;
    m_renderServer->setProject(project);
    m_assistantWidget->setProject(project);
    m_propertyWidget->clear();
    m_renderServer->setHighlightedObjectId(QString());
    m_dataTreeModel->setProject(project);
    m_caseTreeModel->setProject(project);
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

    IDOSRenderView* renderView = m_renderServer->activeView();
    if (renderView == nullptr)
    {
        return;
    }

    const int presetValue = action->data().toInt();
    const IDOSRenderView::ViewPreset preset = static_cast<IDOSRenderView::ViewPreset>(presetValue);
    renderView->setViewPreset(preset);
}

void IDOSMainWindow::onCurrentViewChanged(const QString& viewId)
{
    const bool hasActiveView = !viewId.isEmpty();
    m_actionViewPresets->setEnabled(hasActiveView);
    m_actionResetView->setEnabled(hasActiveView);
}

void IDOSMainWindow::onResetViewTriggered()
{
    if (m_renderServer == nullptr)
    {
        return;
    }

    IDOSRenderView* renderView = m_renderServer->activeView();
    if (renderView != nullptr)
    {
        renderView->resetCamera();
    }
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

    if (m_project == nullptr || !current.isValid())
    {
        m_propertyWidget->clear();
        m_renderServer->setHighlightedObjectId(QString());
        return;
    }

    IDOSTreeNode* node = m_dataTreeModel->nodeFromIndex(current);
    IDOSObjectTreeNode* objectNode = dynamic_cast<IDOSObjectTreeNode*>(node);
    if (objectNode != nullptr)
    {
        IDOSWell* well = qobject_cast<IDOSWell*>(m_project->objectById(objectNode->objectId()));
        if (well != nullptr)
        {
            m_propertyWidget->setWell(well);
            m_propertyWidget->setWellPart(QString(), QString());
            m_renderServer->setHighlightedObjectId(well->objectId());
            return;
        }
    }

    IDOSTreePartNode* partNode = dynamic_cast<IDOSTreePartNode*>(node);
    if (partNode != nullptr)
    {
        IDOSWell* well = qobject_cast<IDOSWell*>(m_project->objectById(partNode->ownerObjectId()));
        if (well != nullptr)
        {
            m_propertyWidget->setWell(well);
            m_propertyWidget->setWellPart(partNode->partKey().toString(), partNode->itemKey());
            m_renderServer->setHighlightedObjectId(well->objectId());
            return;
        }
    }

    m_propertyWidget->clear();
    m_renderServer->setHighlightedObjectId(QString());
}

void IDOSMainWindow::onRenderObjectActivated(const QString& objectId)
{
    const QModelIndex index = m_dataTreeModel->indexFromObjectId(objectId);
    if (!index.isValid())
    {
        return;
    }

    m_dataTreeView->setCurrentIndex(index);
    m_dataTreeView->expand(index.parent());
    m_dataTreeView->scrollTo(index);
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
    m_dataTreeModel->setProject(nullptr);
    m_caseTreeModel->setProject(nullptr);
    m_renderView->clear();
    m_propertyWidget->clear();
    m_renderServer->setHighlightedObjectId(QString());
    m_renderServer->setProject(nullptr);
    m_assistantWidget->setProject(nullptr);
    onCommandStateChanged();
}
