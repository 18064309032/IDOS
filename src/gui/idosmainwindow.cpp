#include "idosmainwindow.h"
#include "input/idosdatatreemodel.h"
#include "input/idosdatatreeview.h"
#include "input/providers/idosgriddatatreeprovider.h"
#include "input/providers/idoswelldatatreeprovider.h"
#include "case/idoscasetreemodel.h"
#include "case/idoscasetreeview.h"
#include "case/providers/idossimulationcasetreeprovider.h"
#include "tree/idostreeproviderregistry.h"
#include "idosproject.h"

#include <DockManager.h>
#include <DockWidget.h>

#include <QLabel>

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
    , m_dockManager(nullptr)
{
    m_treeProviderRegistry->registerDataProvider(new IDOSWellDataTreeProvider());
    m_treeProviderRegistry->registerDataProvider(new IDOSGridDataTreeProvider());
    m_treeProviderRegistry->registerCaseProvider(new IDOSSimulationCaseTreeProvider());
    m_dataTreeModel->setTreeProviderRegistry(m_treeProviderRegistry);
    m_caseTreeModel->setTreeProviderRegistry(m_treeProviderRegistry);

    setWindowTitle(QStringLiteral("IDOS"));
    resize(1280, 800);

    // CDockManager 作为 central widget，ADS 接管整个中央区域的停靠管理
    m_dockManager = new ads::CDockManager(this);
    // 清空 ADS 内部 stylesheet，避免覆盖 SARibbon 主题
    m_dockManager->setStyleSheet(QString());
    setCentralWidget(m_dockManager);

    // 中央区域占位，渲染模块接入后替换为 3D 视图
    QLabel* placeholder = new QLabel(tr("3D View (renderer not yet connected)"), this);
    placeholder->setAlignment(Qt::AlignCenter);
    ads::CDockWidget* centralDock = new ads::CDockWidget(m_dockManager, tr("3D View"));
    centralDock->setObjectName(QStringLiteral("central3DDock"));
    centralDock->setWidget(placeholder);
    centralDock->setFeatures(ads::CDockWidget::NoDockWidgetFeatures);
    m_dockManager->setCentralWidget(centralDock);

    // ---- 数据树 ----
    m_dataTreeView = new IDOSDataTreeView(this);
    m_dataTreeView->setModel(m_dataTreeModel);
    ads::CDockWidget* dataDock = new ads::CDockWidget(m_dockManager, tr("Data"));
    dataDock->setObjectName(QStringLiteral("dataTreeDock"));
    dataDock->setWidget(m_dataTreeView);
    dataDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    ads::CDockAreaWidget* dataArea = m_dockManager->addDockWidget(ads::LeftDockWidgetArea, dataDock);

    // ---- 工况树 ----
    m_caseTreeView = new IDOSCaseTreeView(this);
    m_caseTreeView->setModel(m_caseTreeModel);
    ads::CDockWidget* caseDock = new ads::CDockWidget(m_dockManager, tr("Case"));
    caseDock->setObjectName(QStringLiteral("caseTreeDock"));
    caseDock->setWidget(m_caseTreeView);
    caseDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    m_dockManager->addDockWidget(ads::BottomDockWidgetArea, caseDock, dataArea);
}

IDOSMainWindow::~IDOSMainWindow()
{
    delete m_treeProviderRegistry;
}

IDOSDataTreeView* IDOSMainWindow::dataTreeView() const { return m_dataTreeView; }

IDOSCaseTreeView* IDOSMainWindow::caseTreeView() const { return m_caseTreeView; }

IDOSDataTreeModel* IDOSMainWindow::dataTreeModel() const { return m_dataTreeModel; }

IDOSCaseTreeModel* IDOSMainWindow::caseTreeModel() const { return m_caseTreeModel; }

void IDOSMainWindow::setProject(IDOSProject* project)
{
    if (m_project == project) return;

    m_project = project;
    m_dataTreeModel->setProject(project);
    m_caseTreeModel->setProject(project);
}
