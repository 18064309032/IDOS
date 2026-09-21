#include "idosmainwindow.h"
#include "inputtree/idosinputtreemodel.h"
#include "inputtree/idosinputtreeview.h"
#include "modeltree/idosmodeltreemodel.h"
#include "modeltree/idosmodeltreeview.h"
#include "tree/idostreemodelmenuprovider.h"
#include "tree/idostreeview.h"
#include "idosproject.h"

#include <QDockWidget>
#include <QLabel>

IDOSMainWindow::IDOSMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_actionNewProject(nullptr)   // 占位：SARibbon 接入后创建并入 ribbon 面板
    , m_actionOpenProject(nullptr)
    , m_actionSaveProject(nullptr)
    , m_actionImportWell(nullptr)
    , m_actionImportGrid(nullptr)
    , m_project(nullptr)
    , m_inputModel(new IDOSInputTreeModel(this))
    , m_modelTreeModel(new IDOSModelTreeModel(this))
    , m_inputView(nullptr)
    , m_modelView(nullptr)
{
    setWindowTitle(QStringLiteral("IDOS"));
    resize(1280, 800);

    // 双树同源：成对的 model/view 子类各管一个面板，按类型注册表分流。
    // 布局参照 Petrel/tNavigator：两棵树上下排列停靠左侧，右侧留给 3D 视图
    m_inputView = new IDOSInputTreeView(this);
    createTreeDock(QStringLiteral("inputTreeDock"), tr("Input"),
                   m_inputModel, m_inputView, Qt::LeftDockWidgetArea);

    m_modelView = new IDOSModelTreeView(this);
    createTreeDock(QStringLiteral("modelTreeDock"), tr("Model"),
                   m_modelTreeModel, m_modelView, Qt::LeftDockWidgetArea);

    // 中央区域占位，渲染模块接入后替换为 3D 视图
    auto* placeholder = new QLabel(tr("3D View (renderer not yet connected)"), this);
    placeholder->setAlignment(Qt::AlignCenter);
    setCentralWidget(placeholder);
}

IDOSMainWindow::~IDOSMainWindow() = default;

IDOSTreeView* IDOSMainWindow::createTreeDock(const QString& objectName, const QString& title,
                                             IDOSTreeModel* model, IDOSTreeView* view,
                                             Qt::DockWidgetArea area)
{
    view->setModel(model);

    // 右键菜单：可见性切换等（Provider 由窗口持有）
    auto* menuProvider = new IDOSTreeModelMenuProvider(this);
    menuProvider->setModel(model);
    menuProvider->setTreeView(view);
    view->setMenuProvider(menuProvider);

    auto* dock = new QDockWidget(title, this);
    dock->setObjectName(objectName);   // 稳定标识（saveState/restoreState 用），不随翻译变化
    dock->setWidget(view);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(area, dock);

    return view;
}

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
