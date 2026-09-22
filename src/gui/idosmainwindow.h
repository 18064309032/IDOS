#ifndef IDOS_MAIN_WINDOW_H
#define IDOS_MAIN_WINDOW_H

#include "idos_gui.h"

#include <SARibbonMainWindow.h>
#include <DockWidget.h>

class QAction;
class IDOSProject;
class IDOSInputTreeModel;
class IDOSModelTreeModel;
class IDOSInputTreeView;
class IDOSModelTreeView;

namespace ads
{
    class CDockManager;
}

/**
 * @brief 主窗口：SARibbon 主窗口 + Qt Advanced Docking System 停靠面板。
 *
 * CDockManager 作为 central widget，输入树 / 模型树通过 CDockManager::addDockWidget
 * 挂载到左侧，中央区域通过 CDockManager::setCentralWidget 留给 3D 视图。
 *
 * 模型/视图成对拆分：IDOSInputTreeModel+IDOSInputTreeView（输入面板）、
 * IDOSModelTreeModel+IDOSModelTreeView（模型面板），机制层共享于 gui/tree/。
 *
 * 对外访问约定：视图/模型访问器返回最终类型；跨面板联动优先走
 * 领域信号而非互摸视图。
 */
class GUI_EXPORT IDOSMainWindow : public SARibbonMainWindow
{
    Q_OBJECT

public:
    explicit IDOSMainWindow(QWidget* parent = nullptr);
    ~IDOSMainWindow() override;

    /** 设置工程，同时喂给输入树与模型树两个模型实例。 */
    void setProject(IDOSProject* project);

    // ===== 面板访问器 =====

    IDOSInputTreeView* inputTreeView() const;
    IDOSModelTreeView* modelTreeView() const;
    IDOSInputTreeModel* inputTreeModel() const;
    IDOSModelTreeModel* modelTreeModel() const;

private:
    QAction* m_actionNewProject;      // 占位（SARibbon 接入后入 ribbon 面板）
    QAction* m_actionOpenProject;
    QAction* m_actionSaveProject;
    QAction* m_actionImportWell;
    QAction* m_actionImportGrid;

    IDOSProject* m_project;
    IDOSInputTreeModel* m_inputModel;
    IDOSModelTreeModel* m_modelTreeModel;
    IDOSInputTreeView* m_inputView;
    IDOSModelTreeView* m_modelView;

    ads::CDockManager* m_dockManager;
};

#endif // IDOS_MAIN_WINDOW_H
