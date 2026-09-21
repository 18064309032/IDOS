#ifndef IDOS_MAIN_WINDOW_H
#define IDOS_MAIN_WINDOW_H

#include "idos_gui.h"
#include <QMainWindow>

class QAction;
class IDOSProject;
class IDOSTreeView;
class IDOSTreeModel;
class IDOSInputTreeModel;
class IDOSModelTreeModel;
class IDOSInputTreeView;
class IDOSModelTreeView;

/**
 * @brief 主窗口：输入树 + 模型树双停靠面板 + 中央视图占位。
 *
 * 模型/视图成对拆分：IDOSInputTreeModel+IDOSInputTreeView（输入面板）、
 * IDOSModelTreeModel+IDOSModelTreeView（模型面板），机制层共享于
 * gui/tree/。中央区域预留给 3D 视图。
 *
 * 对外访问约定：视图/模型访问器返回最终类型；跨面板联动优先走
 * 领域信号而非互摸视图。
 *
 * @note 计划切换为 SARibbon 风格（SARibbonMainWindow），待第三方库
 * 二进制补回 thirdparty/SARibbon 后启用。
 */
class GUI_EXPORT IDOSMainWindow : public QMainWindow
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
    /** 创建一个树停靠面板（含右键菜单 Provider 装配）。objectName 为稳定
     *  标识（不随翻译变化），title 为显示标题（tr 翻译）。返回视图。 */
    IDOSTreeView* createTreeDock(const QString& objectName, const QString& title,
                                 IDOSTreeModel* model, IDOSTreeView* view,
                                 Qt::DockWidgetArea area);

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
};

#endif // IDOS_MAIN_WINDOW_H
