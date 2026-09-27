#include "idosmainwindow.h"
#include "idosdataobjecthandling.h"
#include "idosgridrenderobjectprovider.h"
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
#include "idostreeproviderregistry.h"
#include "idosproject.h"
#include "idosnewprojectdialog.h"
#include "idosrenderview.h"
#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonPanel.h>
#include <QAction>
#include <QCloseEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeView>
#include <QIcon>

#include <DockManager.h>
#include <DockWidget.h>

IDOSMainWindow::IDOSMainWindow(QWidget* parent)
    : SARibbonMainWindow(parent)
    , m_actionNewProject(nullptr)
    , m_actionOpenProject(nullptr)
    , m_actionSaveProject(nullptr)
    , m_actionImportWell(nullptr)
    , m_actionImportGrid(nullptr)
    , m_project(nullptr)
    , m_dataTreeModel(new IDOSDataTreeModel(this))
    , m_caseTreeModel(new IDOSCaseTreeModel(this))
    , m_dataTreeView(nullptr)
    , m_caseTreeView(nullptr)
    , m_treeProviderRegistry(new IDOSTreeProviderRegistry())
    , m_renderServer(new IDOSRenderServer(this))
    , m_renderView(nullptr)
    , m_dockManager(nullptr)
    , m_renderDock(nullptr)
{
    m_treeProviderRegistry->registerDataProvider(new IDOSWellDataTreeProvider());
    m_treeProviderRegistry->registerDataProvider(new IDOSGridDataTreeProvider());
    m_treeProviderRegistry->registerCaseProvider(new IDOSSimulationCaseTreeProvider());
    m_dataTreeModel->setTreeProviderRegistry(m_treeProviderRegistry);
    m_caseTreeModel->setTreeProviderRegistry(m_treeProviderRegistry);
    m_renderServer->addProvider(new IDOSGridRenderObjectProvider());

    setWindowTitle(tr("IDOS"));
    resize(1280, 800);

    m_actionNewProject = new QAction(QIcon(QStringLiteral(":/images/app-project-new.svg")), tr("New Project"), this);
    m_actionNewProject->setObjectName(QStringLiteral("newProjectAction"));
    m_actionNewProject->setShortcut(QKeySequence::New);
    addAction(m_actionNewProject);
    connect(m_actionNewProject, &QAction::triggered, this, &IDOSMainWindow::onNewProject);
    SARibbonCategory* projectPage = ribbonBar()->addCategoryPage(tr("Project"));
    projectPage->addPanel(tr("Project Management"))->addLargeAction(m_actionNewProject);

    // CDockManager 作为 central widget，ADS 接管整个中央区域的停靠管理
    m_dockManager = new ads::CDockManager(this);
    // 清空 ADS 内部 stylesheet，避免覆盖 SARibbon 主题
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

    // ---- 数据树 ----
    m_dataTreeView = new IDOSDataTreeView(this);
    m_dataTreeView->setModel(m_dataTreeModel);
    IDOSDataTreeMenuProvider* provider = new IDOSDataTreeMenuProvider(m_dataTreeView);
    m_dataTreeView->setMenuProvider(provider);
    ads::CDockWidget* dataDock = new ads::CDockWidget(m_dockManager, tr("Data"));
    dataDock->setObjectName(QStringLiteral("dataTreeDock"));
    dataDock->setIcon(QIcon(QStringLiteral(":/images/gui-data-tree.svg")));
    dataDock->setWidget(m_dataTreeView);
    dataDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    ads::CDockAreaWidget* dataArea = m_dockManager->addDockWidget(ads::LeftDockWidgetArea, dataDock);

    // ---- 工况树 ----
    m_caseTreeView = new IDOSCaseTreeView(this);
    m_caseTreeView->setModel(m_caseTreeModel);
    connect(m_caseTreeModel, &IDOSCaseTreeModel::itemCheckedChanged, m_renderServer,
            &IDOSRenderServer::onItemCheckedChanged);
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
    m_dataTreeModel->setProject(project);
    m_caseTreeModel->setProject(project);
    if (project)
    {
        connect(project, &QObject::destroyed, this, &IDOSMainWindow::onProjectDestroyed);
    }
}

IDOSDataTreeModel* IDOSMainWindow::dataTreeModel() const
{
    return m_dataTreeModel;
}
IDOSCaseTreeModel* IDOSMainWindow::caseTreeModel() const
{
    return m_caseTreeModel;
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
    if (!confirmDiscardProject())
    {
        return;
    }
    IDOSProject* project = new IDOSProject(this);
    // 当前阶段只创建空容器并初始化树，不应用对话框中的工程信息。
    IDOSProject* previous = m_project;
    setProject(project);
    m_dataTreeView->expandToDepth(0);
    if (previous && previous->parent() == this)
    {
        previous->deleteLater();
    }
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
    m_renderServer->setProject(nullptr);
    m_project = nullptr;
}

void IDOSMainWindow::onNewWell()
{
    if (IDOSDataObjectHandling::newWell(m_project, this))
    {
        m_dataTreeView->expandToDepth(1);
    }
}

void IDOSMainWindow::onImportWellData()
{
    if (IDOSDataObjectHandling::importWellData(m_project, this) > 0)
    {
        m_dataTreeView->expandToDepth(1);
    }
}



