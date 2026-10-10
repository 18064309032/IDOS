#ifndef IDOS_MAIN_WINDOW_H
#define IDOS_MAIN_WINDOW_H

#include <QMap>
#include <QPointer>
#include <QString>

#include <SARibbonMainWindow.h>

#include "idos_app.h"

class IDOSProjectMetadata;
class QAction;
class QCloseEvent;
class QModelIndex;
class QTreeView;
class IDOSCommandManager;
class IDOSProject;
class IDOSDataTreeModel;
class IDOSCaseTreeModel;
class IDOSDataTreeView;
class IDOSCaseTreeView;
class IDOSTreeProviderRegistry;
class IDOSRenderServer;
class IDOSWell;
class IDOSWellLogTrackView;
class IDOSAssistantWidget;
class IDOSDebugInfoWidget;
class IDOSRuntimeInfoWidget;
class IDOSPluginRegistry;
class IDOSPluginManagerWidget;
class IDOSAppInterface;

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
    bool createProject(const IDOSProjectMetadata& metadata,
                       bool confirmDiscard);

    // ===== 面板访问器 =====

    IDOSDataTreeView* dataTreeView() const;
    IDOSCaseTreeView* caseTreeView() const;
    IDOSDataTreeModel* dataTreeModel() const;
    IDOSCaseTreeModel* caseTreeModel() const;

  public slots:
    void onNewProject();

  private slots:
    void onProjectDestroyed();
    void onUndoTriggered();
    void onRedoTriggered();
    void onCommandStateChanged();
    void onDataTreeItemActivated(const QModelIndex& index);
    void onDataTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onCaseTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onRenderObjectActivated(const QString& viewId, const QString& objectId);
    void onPluginManagerTriggered();
    void onFocusedDockWidgetChanged(ads::CDockWidget* oldDock, ads::CDockWidget* currentDock);
    void onCurrentViewChanged(const QString& viewId);
    void onActiveViewContextObjectChanged(const QString& viewId, const QString& objectId);
    void onViewPresetTriggered(QAction* action);
    void onResetViewTriggered();
    void onFitAllTriggered();
    void onBackgroundColorTriggered();
    void onCaptureScreenshotTriggered();
    void onExportImageTriggered();
    void onAssistantTriggered();
    void onNew3DWindowTriggered();
    void onNew2DWindowTriggered();
    void onWindowLayoutTriggered(QAction* action);
    void onRenderViewDockClosed();
    void onOrientationMarkerToggled(bool checked);
    void onLegendToggled(bool checked);
    void onViewDecorationsChanged(bool orientationMarkerVisible,
                                  bool legendAvailable,
                                  bool legendVisible);

  protected:
    void closeEvent(QCloseEvent* event) override;

  private:
    void initProviders();
    void initMainWindow();
    void initDockManager();
    void initRenderView();
    void initRibbonAction();
    void initDockWidgets();
    void initPluginManager();
    void createRenderView(bool parallelProjection);
    void applyWindowLayout(const QString& layout);
    void updateTreeSelection(const QString& viewId);
    void setViewTreeContext(const QString& viewId, const QString& objectId, const QString& caseId);
    void selectTreeIndex(QTreeView* treeView, const QModelIndex& index);
    QModelIndex caseTreeIndexForContext(const QString& objectId, const QString& caseId) const;
    QModelIndex findCaseReferenceIndex(const QModelIndex& parentIndex, const QString& objectId) const;
    QString objectIdFromCaseTreeIndex(const QModelIndex& index) const;
    QString caseIdFromCaseTreeIndex(const QModelIndex& index) const;
    bool confirmDiscardProject();
    IDOSWellLogTrackView* findOrCreateWellLogTrackView(IDOSWell* well);

    QAction* m_actionNewProject;
    QAction* m_actionOpenProject;
    QAction* m_actionSaveProject;
    QAction* m_actionSaveProjectAs;
    QAction* m_actionProjectSettings;
    QAction* m_actionUndo;
    QAction* m_actionRedo;
    QAction* m_actionPluginManager;
    QAction* m_actionViewPresets;
    QAction* m_actionResetView;
    QAction* m_actionFitAll;
    QAction* m_actionBackgroundColor;
    QAction* m_actionOrientationMarker;
    QAction* m_actionLegend;
    QAction* m_actionCaptureScreenshot;
    QAction* m_actionExportImage;
    QAction* m_actionAssistant;
    QAction* m_actionNew3DWindow;
    QAction* m_actionNew2DWindow;
    QAction* m_actionWindowLayout;

    IDOSProject* m_project;
    QPointer<IDOSCommandManager> m_commandManager;
    IDOSDataTreeModel* m_dataTreeModel;
    IDOSCaseTreeModel* m_caseTreeModel;
    IDOSDataTreeView* m_dataTreeView;
    IDOSCaseTreeView* m_caseTreeView;
    IDOSTreeProviderRegistry* m_treeProviderRegistry;
    IDOSRenderServer* m_renderServer;
    IDOSRuntimeInfoWidget* m_runtimeInfoWidget;
    IDOSPluginRegistry* m_pluginRegistry;
    IDOSPluginManagerWidget* m_pluginManagerWidget;
    IDOSAppInterface* m_appInterface;
    IDOSDebugInfoWidget* m_debugInfoWidget;
    IDOSAssistantWidget* m_assistantWidget;

    ads::CDockManager* m_dockManager;
    ads::CDockWidget* m_renderDock;
    ads::CDockWidget* m_outputDock;
    ads::CDockWidget* m_debugDock;
    ads::CDockWidget* m_assistantDock;
    QMap<QString, QPointer<ads::CDockWidget>> m_viewDocks;
    QMap<QString, QString> m_viewCaseIds;
    int m_nextViewIndex;
    QString m_currentWindowLayout;
};

#endif // IDOS_MAIN_WINDOW_H
