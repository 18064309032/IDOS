#ifndef IDOS_MAIN_WINDOW_H
#define IDOS_MAIN_WINDOW_H

#include "idos_gui.h"

#include <SARibbonMainWindow.h>
#include <DockWidget.h>

class QAction;
class IDOSProject;
class IDOSDataTreeModel;
class IDOSCaseTreeModel;
class IDOSDataTreeView;
class IDOSCaseTreeView;
class IDOSTreeProviderRegistry;

namespace ads
{
    class CDockManager;
}

/**
 * @brief 主窗口：SARibbon 主窗口 + Qt Advanced Docking System 停靠面板。
 *
 * CDockManager 作为 central widget，数据树 / 工况树通过 CDockManager::addDockWidget
 * 挂载到左侧，中央区域通过 CDockManager::setCentralWidget 留给 3D 视图。
 */
class GUI_EXPORT IDOSMainWindow : public SARibbonMainWindow
{
    Q_OBJECT

public:
    explicit IDOSMainWindow(QWidget* parent = nullptr);
    ~IDOSMainWindow() override;

    /** 设置工程，同时喂给数据树与工况树两个模型实例。 */
    void setProject(IDOSProject* project);

    // ===== 面板访问器 =====

    IDOSDataTreeView* dataTreeView() const;
    IDOSCaseTreeView* caseTreeView() const;
    IDOSDataTreeModel* dataTreeModel() const;
    IDOSCaseTreeModel* caseTreeModel() const;

private:
    QAction* m_actionNewProject;      // 占位（SARibbon 接入后入 ribbon 面板）
    QAction* m_actionOpenProject;
    QAction* m_actionSaveProject;
    QAction* m_actionImportWell;
    QAction* m_actionImportGrid;

    IDOSProject* m_project;
    IDOSDataTreeModel* m_dataTreeModel;
    IDOSCaseTreeModel* m_caseTreeModel;
    IDOSDataTreeView* m_dataTreeView;
    IDOSCaseTreeView* m_caseTreeView;
    IDOSTreeProviderRegistry* m_treeProviderRegistry;

    ads::CDockManager* m_dockManager;
};

#endif // IDOS_MAIN_WINDOW_H
