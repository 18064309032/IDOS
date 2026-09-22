#include "idosmainwindow.h"
#include "inputtree/idosinputtreemodel.h"
#include "inputtree/idosinputtreeview.h"
#include "modeltree/idosmodeltreemodel.h"
#include "modeltree/idosmodeltreeview.h"
#include "tree/idostreemodelmenuprovider.h"
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
    , m_inputModel(new IDOSInputTreeModel(this))
    , m_modelTreeModel(new IDOSModelTreeModel(this))
    , m_inputView(nullptr)
    , m_modelView(nullptr)
    , m_dockManager(nullptr)
{
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

    // ---- 输入树 ----
    m_inputView = new IDOSInputTreeView(this);
    m_inputView->setModel(m_inputModel);

    IDOSTreeModelMenuProvider* inputMenuProvider = new IDOSTreeModelMenuProvider(this);
    inputMenuProvider->setModel(m_inputModel);
    inputMenuProvider->setTreeView(m_inputView);
    m_inputView->setMenuProvider(inputMenuProvider);

    ads::CDockWidget* inputDock = new ads::CDockWidget(m_dockManager, tr("Input"));
    inputDock->setObjectName(QStringLiteral("inputTreeDock"));
    inputDock->setWidget(m_inputView);
    inputDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    ads::CDockAreaWidget* inputArea = m_dockManager->addDockWidget(ads::LeftDockWidgetArea, inputDock);

    // ---- 模型树 ----
    m_modelView = new IDOSModelTreeView(this);
    m_modelView->setModel(m_modelTreeModel);

    IDOSTreeModelMenuProvider* modelMenuProvider = new IDOSTreeModelMenuProvider(this);
    modelMenuProvider->setModel(m_modelTreeModel);
    modelMenuProvider->setTreeView(m_modelView);
    m_modelView->setMenuProvider(modelMenuProvider);

    ads::CDockWidget* modelDock = new ads::CDockWidget(m_dockManager, tr("Model"));
    modelDock->setObjectName(QStringLiteral("modelTreeDock"));
    modelDock->setWidget(m_modelView);
    modelDock->setFeatures(ads::CDockWidget::DockWidgetMovable | ads::CDockWidget::DockWidgetFloatable);
    // 相对 inputArea 底部插入，左侧形成上下分割而非标签页
    m_dockManager->addDockWidget(ads::BottomDockWidgetArea, modelDock, inputArea);
}

IDOSMainWindow::~IDOSMainWindow() = default;

IDOSInputTreeView* IDOSMainWindow::inputTreeView() const { return m_inputView; }

IDOSModelTreeView* IDOSMainWindow::modelTreeView() const { return m_modelView; }

IDOSInputTreeModel* IDOSMainWindow::inputTreeModel() const { return m_inputModel; }

IDOSModelTreeModel* IDOSMainWindow::modelTreeModel() const { return m_modelTreeModel; }

void IDOSMainWindow::setProject(IDOSProject* project)
{
    if (m_project == project) return;

    m_project = project;
    m_inputModel->setProject(project);
    m_modelTreeModel->setProject(project);
}
