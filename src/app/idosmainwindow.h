#ifndef IDOS_MAIN_WINDOW_H
#define IDOS_MAIN_WINDOW_H

#include "idos_app.h"

#include <SARibbonMainWindow.h>

class QAction;
class QCloseEvent;
class QModelIndex;
class IDOSProject;
class IDOSDataTreeModel;
class IDOSCaseTreeModel;
class IDOSDataTreeView;
class IDOSCaseTreeView;
class IDOSTreeProviderRegistry;
class IDOSRenderServer;
class IDOSRenderView;
class IDOSWell;
class IDOSWellLogTrackView;
class IDOSPropertyWidget;

namespace ads
{
class CDockManager;
class CDockWidget;
} // namespace ads

/**
 * @brief 主窗口：SARibbon 主窗口 + Qt Advanced Docking System 停靠面板。
 *
 * CDockManager 作为 central widget，数据树 / 工况树通过 CDockManager::addDockWidget
 * 挂载到左侧，中央区域通过 CDockManager::setCentralWidget 留给 3D 视图。
 */
class APP_EXPORT IDOSMainWindow : public SARibbonMainWindow
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

  public slots:
    void onNewProject();

  private slots:
    void onProjectDestroyed();
    void onDataTreeItemActivated(const QModelIndex& index);
    void onDataTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onRenderObjectActivated(const QString& objectId);

  protected:
    void closeEvent(QCloseEvent* event) override;

  private:
    bool confirmDiscardProject();
    IDOSWellLogTrackView* findOrCreateWellLogTrackView(IDOSWell* well);

    QAction* m_actionNewProject;
    QAction* m_actionOpenProject;
    QAction* m_actionSaveProject;
    QAction* m_actionSaveProjectAs;
    QAction* m_actionProjectSettings;

    IDOSProject* m_project;
    IDOSDataTreeModel* m_dataTreeModel;
    IDOSCaseTreeModel* m_caseTreeModel;
    IDOSDataTreeView* m_dataTreeView;
    IDOSCaseTreeView* m_caseTreeView;
    IDOSTreeProviderRegistry* m_treeProviderRegistry;
    IDOSRenderServer* m_renderServer;
    IDOSRenderView* m_renderView;
    IDOSPropertyWidget* m_propertyWidget;

    ads::CDockManager* m_dockManager;
    ads::CDockWidget* m_renderDock;
};

#endif // IDOS_MAIN_WINDOW_H
