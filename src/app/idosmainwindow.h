#ifndef IDOS_MAIN_WINDOW_H
#define IDOS_MAIN_WINDOW_H

#include <QList>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <SARibbonMainWindow.h>

#include "idos_app.h"

class IDOSProjectMetadata;
class IDOSCommandRegistry;
class IDOSDataRegistry;
class QAction;
class QCloseEvent;
class QLabel;
class QModelIndex;
class QTreeView;
class QStackedWidget;
class IDOSProjectStartWidget;
class IDOSCommandManager;
class IDOSProject;
class IDOSTreeModel;
class IDOSTreeView;
class IDOSTreeRegistry;
class IDOSTreeServer;
class IDOSTree;
class IDOSRenderRegistry;
class IDOSRenderServer;
class IDOSRenderView;
class IDOSWell;
class IDOSWellLogTrackView;
class IDOSAssistantWidget;
class IDOSDebugInfoWidget;
class IDOSRuntimeInfoWidget;
class IDOSPythonConsoleWidget;
class IDOSPluginRegistry;
class IDOSPluginManagerWidget;
class IDOSAppInterface;
class IDOSToolRegistry;
class IDOSSimulation;
class SARibbonPanel;

namespace ads
{
class CDockManager;
class CDockWidget;
} // namespace ads

/**
 * @brief 主窗口：SARibbon 主窗口 + Qt Advanced Docking System 停靠面板。
 *
 * CDockManager 作为 central widget，数据树 / 工况树通过 CDockManager::addDockWidget
 * 挂载到左侧；中央页面在启动页与工程渲染视图之间切换。
 */
class APP_EXPORT IDOSMainWindow : public SARibbonMainWindow
{
    Q_OBJECT

  public:
    explicit IDOSMainWindow(QWidget* parent = nullptr);
    ~IDOSMainWindow() override;

    /** 设置工程，同时喂给数据树与工况树两个模型实例。 */
    void setProject(IDOSProject* project);
    bool openProject(const QString& path, bool confirmDiscard = true);
    bool saveProject(const QString& path = QString());
    bool createProject(const IDOSProjectMetadata& metadata,
                       bool confirmDiscard);

    // ===== 面板访问器 =====

    IDOSTreeView* dataTreeView() const;
    IDOSTreeView* caseTreeView() const;
    IDOSTreeModel* dataTreeModel() const;
    IDOSTreeModel* caseTreeModel() const;
    IDOSProject* project() const;
    IDOSDataRegistry* dataRegistry() const;
    IDOSTreeRegistry* treeRegistry() const;
    IDOSRenderRegistry* renderRegistry() const;
    IDOSToolRegistry* toolRegistry() const;
    IDOSCommandRegistry* commandRegistry() const;
    IDOSRenderServer* renderServer() const;
    SARibbonPanel* homeRibbonPanel(const QString& groupId) const;

  signals:
    void projectChanged(IDOSProject* project);

  public slots:
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveProjectAs();
    void onCloseProject();

  private slots:
    void onProjectDestroyed();
    void onProjectModified();
    void onPresentationChanged();
    void onStartPageRecentProjectOpenRequested(const QString& path);
    void onStartPageRecentProjectRemoveRequested(const QString& path);
    void onRenderViewFocused();
    void onRenderViewDecorations();
    void onRenderViewActivated(const QString& id);
    void onRenderViewDestroyed(QObject* object);
    void onRenderCurrentViewChanged(IDOSRenderView* view);
    void onRenderDataRemoving(const QString& id);
    void onRenderObjectsChanged(IDOSRenderView* view, const QStringList& ids, bool dataChanged);
    void onUndoTriggered();
    void onRedoTriggered();
    void onCommandStateChanged();
    void onDataTreeItemActivated(const QModelIndex& index);
    void onDataTreeItemClicked(const QModelIndex& index);
    void onCaseTreeItemClicked(const QModelIndex& index);
    void onDataTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onCaseTreeCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onRenderObjectActivated(const QString& viewId, const QString& objectId);
    void onPluginManagerTriggered();
    void onFocusedDockWidgetChanged(ads::CDockWidget* oldDock, ads::CDockWidget* currentDock);
    void onCurrentViewChanged(const QString& viewId);
    void onActiveViewContextObjectChanged(const QString& viewId, const QString& objectId);
    void onCaptureScreenshotTriggered();
    void onExportImageTriggered();
    void onAssistantTriggered();
    void onPythonConsoleTriggered();
    void onNew3DWindowTriggered();
    void onNew2DWindowTriggered();
    void onWindowLayoutTriggered(QAction* action);
    void onRenderViewDockClosed();

  protected:
    void closeEvent(QCloseEvent* event) override;

  private:
    void collectProjectPresentation(bool force = false);
    void prepareProjectViews(const QVariantMap& state);
    void restoreProjectPresentation(const QVariantMap& state);
    void saveWindowPreferences();
    void restoreWindowPreferences(bool geometry);
    void rememberProject(const QString& path);
    void updateRecentProjects();
    void updateCentralPage();
    QString layoutSettingsKey() const;
    QList<IDOSWellLogTrackView*> logTrackViews() const;
    static QString windowSettingsPath();
    void initProviders();
    IDOSRenderView* activeView() const;
    QString activeViewId() const;
    IDOSRenderView* createView(const QString& id, bool twoDimensional);
    void removeView(const QString& id);
    void setActiveView(const QString& id);
    QString viewContextId() const;
    void setViewContextId(const QString& id);
    void initMainWindow();
    void initDockManager();
    void initRenderView();
    void initRibbonAction();
    void initDockWidgets();
    void initPluginManager();
    void createRenderView(bool twoDimensional);
    void applyWindowLayout(const QString& layout);
    void updateTreeSelection(const QString& viewId, QTreeView* source = nullptr);
    void setViewTreeContext(const QString& viewId, const QString& objectId, const QString& caseId);
    void updateStatusBarIndicators();
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
    QAction* m_actionCloseProject;
    QAction* m_actionProjectSettings;
    QAction* m_actionUndo;
    QAction* m_actionRedo;
    QAction* m_actionPluginManager;
    QAction* m_actionCaptureScreenshot;
    QAction* m_actionExportImage;
    QAction* m_actionAssistant;
    QAction* m_actionPythonConsole;
    QAction* m_actionNew3DWindow;
    QAction* m_actionNew2DWindow;
    QAction* m_actionWindowLayout;

    IDOSProject* m_project;
    IDOSDataRegistry* m_dataRegistry;
    QPointer<IDOSCommandManager> m_commandManager;
    IDOSTree* m_dataTree;
    IDOSTree* m_caseTree;
    IDOSTreeModel* m_dataTreeModel;
    IDOSTreeModel* m_caseTreeModel;
    IDOSTreeView* m_dataTreeView;
    IDOSTreeView* m_caseTreeView;
    IDOSTreeRegistry* m_treeRegistry;
    IDOSTreeServer* m_treeServer;
    IDOSRenderRegistry* m_renderRegistry;
    IDOSToolRegistry* m_toolRegistry;
    IDOSRenderServer* m_renderServer;
    QMap<QString, QPointer<IDOSRenderView>> m_views;
    QMap<QString, QString> m_viewContextIds;
    IDOSRuntimeInfoWidget* m_runtimeInfoWidget;
    IDOSPluginRegistry* m_pluginRegistry;
    IDOSPluginManagerWidget* m_pluginManagerWidget;
    IDOSAppInterface* m_appInterface;
    IDOSSimulation* m_simulation;
    IDOSDebugInfoWidget* m_debugInfoWidget;
    IDOSAssistantWidget* m_assistantWidget;
    IDOSPythonConsoleWidget* m_pythonConsoleWidget;
    QLabel* m_projectStatusLabel;
    QLabel* m_projectStatusDot;
    QLabel* m_viewStatusLabel;
    QLabel* m_objectCountStatusLabel;

    ads::CDockManager* m_dockManager;
    ads::CDockWidget* m_renderDock;
    QStackedWidget* m_centralPageStack;
    IDOSProjectStartWidget* m_startPage;
    QWidget* m_renderPage;
    ads::CDockWidget* m_outputDock;
    ads::CDockWidget* m_debugDock;
    ads::CDockWidget* m_pythonConsoleDock;
    ads::CDockWidget* m_assistantDock;
    QMap<QString, QPointer<ads::CDockWidget>> m_viewDocks;
    QMap<QString, QString> m_viewCaseIds;
    int m_nextViewIndex;
    QString m_currentWindowLayout;
    bool m_syncingTreeSelection;
    bool m_restoringPresentation;
    QVariantMap m_defaultViewState;
};

#endif // IDOS_MAIN_WINDOW_H
