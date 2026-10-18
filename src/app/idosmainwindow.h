#ifndef IDOS_MAIN_WINDOW_H
#define IDOS_MAIN_WINDOW_H

#include <QJsonArray>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <SARibbonMainWindow.h>

#include "idos_app.h"
#include "tree/idoscasetreeitemmodel.h"
#include "tree/idoscasetreeview.h"
#include "tree/idosdatatreeitemmodel.h"
#include "tree/idosdatatreeview.h"

class IDOSGridFilterWidget;
class IDOSGridSettingWidget;
class IDOSWellDisplayWidget;
class IDOSRenderObject;
class IDOSCaseObject;
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
class IDOSAppTreeAccess;
class IDOSTreeModel;
class IDOSTreeView;
class IDOSDataTreeMenuProvider;
class IDOSTreeItemProviderRegistry;
class IDOSCaseTreeMenuProvider;
class IDOSRenderProviderRegistry;
class IDOSRenderServer;
class IDOSRenderView;
class IDOSWellObject;
class IDOSWellLogTrackView;
class IDOSAssistantWidget;
class IDOSDebugInfoWidget;
class IDOSRuntimeInfoWidget;
class IDOSPythonConsoleWidget;
class IDOSPluginRegistry;
class IDOSPluginManagerWidget;
class IDOSAppInterface;
class IDOSToolProviderRegistry;
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

    /** 切换工程并重新装配树 Access 与渲染服务。 */
    void setProject(IDOSProject* project);
    bool transferTreeItems(IDOSTreeModel* model, const QVariantMap& arguments, bool validateOnly);
    void onNodeImportDataset(IDOSTreeView* view, const QString& requestedText, const QString& requestedFileFilter,
                             const QString& requestedProviderId, const QString& requestedDataTypeId);
    void onNodeSelectSeismicInline(IDOSTreeView* view, const QString& requestedDataId, IDOSRenderView* targetView);
    void onNodeImportFluidRockTables(IDOSTreeView* view, const QString& requestedCategory);
    void onNodeAddFluidRockTableToCase(IDOSTreeView* view, const QString& requestedDataId);
    void onDataGridSettingsTriggered(IDOSTreeView* view, const QString& requestedGridId,
                                     const QString& requestedFilterId, const QString& requestedPropertyId,
                                     bool requestedOpenFilter, IDOSRenderView* targetView);
    void onDataGridDisplayTriggered(IDOSTreeView* view, const QString& requestedGridId,
                                    const QString& requestedPropertyId, IDOSRenderView* targetView);
    void onDataWellSettingsTriggered(IDOSTreeView* view, const QString& requestedWellId, bool requestedTrajectory,
                                     IDOSRenderView* targetView);
    void onDataDeleteDatasets(IDOSTreeView* sourceView, const QStringList& requestedObjectIds);
    void onDataAddToCase(IDOSTreeView* sourceView, const QJsonArray& requestedItems);
    void onDataRenameObject(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onDataGridImportProperty(IDOSTreeView* sourceView, const QString& requestedDataId,
                                  const QString& requestedCaseId);
    void onDataGridImportDynamicProperties(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onDataGridDeleteProperty(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onDataGridDelete(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onDataGridDeleteFilter(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onDataGridRenameFilter(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onDataViewWellData(IDOSTreeView* sourceView, const QString& requestedDataId, const QString& requestedPart,
                            const QString& requestedItemKey);
    void onDataNewWell(IDOSTreeView* sourceView);
    void onDataImportWell(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onDataDeleteWell(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onCaseGenerateInput(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onCaseViewPropertyCurves(IDOSTreeView* sourceView, const QString& requestedCaseId);
    void onCaseImportSummaryResults(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onCaseViewSummaryResults(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onCaseRenameCase(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onCaseNewCase(IDOSTreeView* sourceView);
    void onCaseImportCase(IDOSTreeView* sourceView);
    void onCaseImportGrid(IDOSTreeView* sourceView, const QString& requestedCaseId);
    void onCaseDeleteCase(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onCaseRenameObject(IDOSTreeView* sourceView, const QString& requestedDataId);
    void onDataImportWellHeader(IDOSTreeView* view, const QString& dataId);
    void onDataImportWellTrajectory(IDOSTreeView* view, const QString& dataId);
    void onDataImportWellCurves(IDOSTreeView* view, const QString& dataId);
    void onDataImportWellMarkers(IDOSTreeView* view, const QString& dataId);
    void onDataImportWellCompletions(IDOSTreeView* view, const QString& dataId);
    void onDataImportWellCheckshots(IDOSTreeView* view, const QString& dataId);
    bool openProject(const QString& path, bool confirmDiscard = true);
    bool saveProject(const QString& path = QString());
    bool createProject(const IDOSProjectMetadata& metadata, bool confirmDiscard);

    // ===== 面板访问器 =====

    IDOSDataTreeView* dataTreeView() const;
    IDOSCaseTreeView* caseTreeView() const;
    IDOSDataTreeItemModel* dataTreeModel() const;
    IDOSCaseTreeItemModel* caseTreeModel() const;
    IDOSProject* project() const;
    IDOSDataRegistry* dataRegistry() const;
    IDOSTreeItemProviderRegistry* treeRegistry() const;
    IDOSRenderProviderRegistry* renderRegistry() const;
    IDOSToolProviderRegistry* toolRegistry() const;
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
    void onRenderViewActivated(const QString& id);
    void onRenderViewDestroyed(QObject* object);
    void onRenderCurrentViewChanged(IDOSRenderView* view);
    void onRenderDataRemoving(const QString& id);
    void onRenderObjectsChanged(IDOSRenderView* view, const QStringList& ids, bool dataChanged);
    void onDataGridFilterRenameAccepted();
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
    void dataImportWellPart(IDOSTreeView* view, const QString& id, const QString& providerPrefix);
    void clearTreeDialogs();
    IDOSRenderObject* dataGridRenderObject(const QString& gridId, IDOSRenderView* view) const;
    int dataGridPropertyCount(const IDOSTreeView* view, const QString& gridId) const;
    bool dataDeleteGrid(IDOSTreeView* view, const QString& gridId);
    bool dataDeleteProperty(IDOSTreeView* view, const QString& propertyId);
    bool dataImportProperty(IDOSTreeView* view, const QString& targetGridId, const QString& targetCaseId);
    bool dataImportDynamicProperties(IDOSTreeView* view, const QString& targetGridId);
    void dataReportGridImportSuccess(IDOSTreeView* view, const QString& message);
    int dataImportWellData(IDOSProject* project, QWidget* parent);
    int dataImportWellData(IDOSProject* project, const QString& targetWellId, QWidget* parent);
    bool dataNewWell(IDOSProject* project, QWidget* parent);
    bool dataDeleteWell(IDOSProject* project, const QString& wellId, QWidget* parent);
    int caseGridPropertyCount(const IDOSTreeView* view, const QString& gridId) const;
    QString caseReferencedGridId(const IDOSCaseObject* caseObj);
    bool caseNameExists(const IDOSTreeView* view, const QString& name) const;
    bool caseNewCase(IDOSTreeView* view);
    bool caseImportCase(IDOSTreeView* view);
    bool caseDeleteCase(IDOSTreeView* view, const QString& caseId);
    bool caseImportGrid(IDOSTreeView* view, const QString& targetCaseId);
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
    IDOSWellLogTrackView* findOrCreateWellLogTrackView(IDOSWellObject* well);

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
    IDOSTreeItemProviderRegistry* m_treeRegistry;
    QPointer<IDOSGridSettingWidget> m_gridWidget;
    QPointer<IDOSGridFilterWidget> m_filterWidget;
    QPointer<IDOSWellDisplayWidget> m_wellWidget;
    IDOSAppTreeAccess* m_treeAccess;
    IDOSDataTreeItemModel* m_dataTreeModel;
    IDOSCaseTreeItemModel* m_caseTreeModel;
    IDOSDataTreeView* m_dataTreeView;
    IDOSDataTreeMenuProvider* m_dataTreeMenuProvider;
    IDOSCaseTreeView* m_caseTreeView;
    IDOSCaseTreeMenuProvider* m_caseTreeMenuProvider;
    IDOSRenderProviderRegistry* m_renderRegistry;
    IDOSToolProviderRegistry* m_toolRegistry;
    IDOSRenderServer* m_renderServer;
    QMap<QString, QPointer<IDOSRenderView>> m_views;
    QMap<QString, QString> m_viewContextIds;
    IDOSRuntimeInfoWidget* m_runtimeInfoWidget;
    IDOSPluginRegistry* m_pluginRegistry;
    IDOSPluginManagerWidget* m_pluginManagerWidget;
    IDOSAppInterface* m_appInterface;
    IDOSSimulation* m_simulation;
    IDOSDebugInfoWidget* m_debugInfoWidget;
    IDOSCommandRegistry* m_commandRegistry;
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
