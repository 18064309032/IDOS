#include "idosmainwindow.h"
#include "idosdataobjecthandling.h"
#include "idosgridrenderobjectprovider.h"
#include "idoswellrenderobjectprovider.h"
#include "idoswelllogtrackview.h"
#include "idospropertywidget.h"
#include "idosassistantwidget.h"
#include "idosrenderserver.h"
#include "idosdatatreemodel.h"
#include "idosdatatreemenuprovider.h"
#include "idoscasetreemenuprovider.h"
#include "idosdatatreeview.h"
#include "idosgriddatatreeprovider.h"
#include "idoswelldatatreeprovider.h"
#include "idoscasetreemodel.h"
#include "idoscasetreeview.h"
#include "idossimulationcasetreeprovider.h"
#include "idosobjecttreenode.h"
#include "idostreepartnode.h"
#include "idostreeproviderregistry.h"
#include "idosproject.h"
#include "idosprojectmetadata.h"
#include "command/idoscommandmanager.h"
#include "idosnewprojectdialog.h"
#include "idosrenderview.h"
#include "idoswell.h"
#include "idosapplication.h"
#include "idosdebuginfowidget.h"
#include "idosruntimeinfowidget.h"
#include "log/idoslogger.h"
#include "command/idoscommandmetadata.h"
#include "command/idoscommandregistry.h"
#include "command/idoscreatecasecommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosrenameobjectcommand.h"
#include "command/idosimportcommands.h"
#include <memory>
#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonPanel.h>
#include <SARibbonSystemButtonBar.h>
#include <SARibbonQuickAccessBar.h>
#include <QAction>
#include <QCloseEvent>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QIcon>

#include <DockManager.h>
#include <DockWidget.h>

IDOSMainWindow::IDOSMainWindow(QWidget* parent)
    : SARibbonMainWindow(parent)
    , m_actionNewProject(nullptr)
    , m_actionOpenProject(nullptr)
    , m_actionSaveProject(nullptr)
    , m_actionSaveProjectAs(nullptr)
    , m_actionProjectSettings(nullptr)
    , m_actionUndo(nullptr)
    , m_actionRedo(nullptr)
    , m_project(nullptr)
    , m_dataTreeModel(new IDOSDataTreeModel(this))
    , m_caseTreeModel(new IDOSCaseTreeModel(this))
    , m_dataTreeView(nullptr)
    , m_caseTreeView(nullptr)
    , m_treeProviderRegistry(new IDOSTreeProviderRegistry())
    , m_renderServer(new IDOSRenderServer(this))
    , m_renderView(nullptr)
    , m_propertyWidget(nullptr)
    , m_runtimeInfoWidget(nullptr)
    , m_debugInfoWidget(nullptr)
    , m_assistantWidget(nullptr)
    , m_dockManager(nullptr)
    , m_renderDock(nullptr)
    , m_outputDock(nullptr)
    , m_debugDock(nullptr)
    , m_assistantDock(nullptr)
{
    m_treeProviderRegistry->registerDataProvider(new IDOSWellDataTreeProvider());
    m_treeProviderRegistry->registerDataProvider(new IDOSGridDataTreeProvider());
    m_treeProviderRegistry->registerCaseProvider(new IDOSSimulationCaseTreeProvider());
    m_dataTreeModel->setTreeProviderRegistry(m_treeProviderRegistry);
    m_caseTreeModel->setTreeProviderRegistry(m_treeProviderRegistry);
    m_renderServer->addProvider(new IDOSGridRenderObjectProvider());
    m_renderServer->addProvider(new IDOSWellRenderObjectProvider());

    setWindowTitle(tr("IDOS"));

    m_actionNewProject = new QAction(QIcon(QStringLiteral(":/images/app-project-new.svg")), tr("New Project"), this);
    m_actionNewProject->setObjectName(QStringLiteral("newProjectAction"));
    m_actionNewProject->setShortcut(QKeySequence::New);
    addAction(m_actionNewProject);
    connect(m_actionNewProject, &QAction::triggered, this, &IDOSMainWindow::onNewProject);
    SARibbonCategory* projectPage = ribbonBar()->addCategoryPage(tr("Project"));
    projectPage->addPanel(tr("Project Management"))->addLargeAction(m_actionNewProject);

    m_actionUndo = new QAction(QIcon(QStringLiteral(":/images/app-undo.svg")),
                               tr("Undo"),
                               this);
    m_actionUndo->setObjectName(QStringLiteral("undoAction"));
    m_actionUndo->setShortcut(QKeySequence::Undo);
    m_actionUndo->setEnabled(false);
    addAction(m_actionUndo);
    connect(m_actionUndo, &QAction::triggered, this, &IDOSMainWindow::onUndoTriggered);

    m_actionRedo = new QAction(QIcon(QStringLiteral(":/images/app-redo.svg")),
                               tr("Redo"),
                               this);
    m_actionRedo->setObjectName(QStringLiteral("redoAction"));
    m_actionRedo->setShortcut(QKeySequence::Redo);
    m_actionRedo->setEnabled(false);
    addAction(m_actionRedo);
    connect(m_actionRedo, &QAction::triggered, this, &IDOSMainWindow::onRedoTriggered);
    SARibbonQuickAccessBar* quickAccessBar = ribbonBar()->quickAccessBar();
    quickAccessBar->addAction(m_actionUndo);
    quickAccessBar->addAction(m_actionRedo);

    m_dockManager = new ads::CDockManager(this);
    m_dockManager->setStyleSheet(QString());
    setCentralWidget(m_dockManager);

    m_renderView = new IDOSRenderView(this);
    m_renderDock = new ads::CDockWidget(m_dockManager, tr("3D View"));
    m_renderDock->setObjectName(QStringLiteral("central3DDock"));
    m_renderDock->setIcon(QIcon(QStringLiteral(":/images/render-view.svg")));
    m_renderDock->setWidget(m_renderView);
    m_renderDock->setFeatures(ads::CDockWidget::NoDockWidgetFeatures);
    m_dockManager->setCentralWidget(m_renderDock);
    m_renderServer->addView(QStringLiteral("main3d"), m_renderView);
    m_renderServer->setActiveView(QStringLiteral("main3d"));
    connect(m_renderView, &IDOSRenderView::objectActivated,
            this, &IDOSMainWindow::onRenderObjectActivated);

    // ---- 数据树 ----
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

    IDOS_MESSAGE(tr("Application started."), IDOSLogLevel::Info);
}

IDOSMainWindow::~IDOSMainWindow()
{
    m_dataTreeModel->setProject(nullptr);
    m_caseTreeModel->setProject(nullptr);
    if (m_project)
    {
        disconnect(m_project, nullptr, this, nullptr);
    }
    delete m_treeProviderRegistry;
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

    if (m_project)
    {
        disconnect(m_project, nullptr, this, nullptr);
    }
    m_project = project;
    m_renderServer->setProject(project);
    m_assistantWidget->setProject(project);
    m_propertyWidget->clear();
    m_renderServer->setHighlightedObjectId(QString());
    m_dataTreeModel->setProject(project);
    m_caseTreeModel->setProject(project);
    if (project)
    {
        connect(project, &QObject::destroyed, this, &IDOSMainWindow::onProjectDestroyed);
        connect(project->commandManager(), &IDOSCommandManager::stateChanged,
                this, &IDOSMainWindow::onCommandStateChanged);
    }

    onCommandStateChanged();
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
        return false;
    }

    IDOSProject* project = new IDOSProject(this);
    if (!project->setMetadata(metadata))
    {
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
    if (m_project != nullptr)
    {
        m_project->commandManager()->undo();
    }
}

void IDOSMainWindow::onRedoTriggered()
{
    if (m_project != nullptr)
    {
        m_project->commandManager()->redo();
    }
}

void IDOSMainWindow::onCommandStateChanged()
{
    IDOSCommandManager* commandManager =
        m_project != nullptr ? m_project->commandManager() : nullptr;

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
    if (!confirmDiscardProject())
    {
        event->ignore();
        return;
    }
    SARibbonMainWindow::closeEvent(event);
}

void IDOSMainWindow::onProjectDestroyed()
{
    m_dataTreeModel->setProject(nullptr);
    m_caseTreeModel->setProject(nullptr);
    m_renderView->clear();
    m_propertyWidget->clear();
    m_renderServer->setHighlightedObjectId(QString());
    m_renderServer->setProject(nullptr);
    m_assistantWidget->setProject(nullptr);
    m_project = nullptr;
    onCommandStateChanged();
}
