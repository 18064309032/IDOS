#ifndef IDOS_APP_TREE_ACCESS_H
#define IDOS_APP_TREE_ACCESS_H

#include <QJsonArray>
#include <QPointer>
#include <QStringList>
#include <QVariantMap>
#include <QtCore/qnamespace.h>

#include "idos_app.h"
#include "tree/idostreeaccess.h"

class IDOSDataObject;
class IDOSDataTreeItemModel;
class IDOSCaseTreeItemModel;
class IDOSProject;
class IDOSRenderServer;
class IDOSRenderView;
class IDOSTreeItem;
class IDOSTreeModel;

class APP_EXPORT IDOSAppTreeAccess : public IDOSTreeAccess
{
    Q_OBJECT
  public:
    explicit IDOSAppTreeAccess(IDOSProject* project, QObject* parent = nullptr);
    ~IDOSAppTreeAccess() override;
    void setDataModel(IDOSDataTreeItemModel* model);
    void setCaseModel(IDOSCaseTreeItemModel* model);
    void setRenderServer(IDOSRenderServer* server);
  private slots:
    void onProjectChanged(const QString& id);
    void onProjectChanged(const QStringList& ids);
    void onProjectDestroyed();
    void onRegistryChanged();
    void onTransferRequested(const QVariantMap& arguments, bool validateOnly, bool& accepted);
    void onImportDatasetRequested(const QString& text, const QString& fileFilter, const QString& providerId,
                                  const QString& dataTypeId);
    void onSelectSeismicInlineRequested(const QString& dataId);
    void onImportFluidRockTablesRequested(const QString& category);
    void onAddFluidRockTableToCaseRequested(const QString& dataId);
    void onShowGridSettingsRequested(const QString& gridId, const QString& filterId, const QString& propertyId,
                                     bool openFilter);
    void onDisplayGridPropertyRequested(const QString& gridId, const QString& propertyId);
    void onShowWellSettingsRequested(const QString& wellId, bool trajectory);
    void onRemoveRequested(const QStringList& objectIds);
    void onAddToCaseRequested(const QJsonArray& items);
    void onRenameRequested(const QString& dataId);
    void onImportGridPropertyRequested(const QString& dataId, const QString& caseId);
    void onImportGridDynamicPropertiesRequested(const QString& dataId);
    void onDeleteGridPropertyRequested(const QString& dataId);
    void onDeleteGridRequested(const QString& dataId);
    void onDeleteGridFilterRequested(const QString& dataId);
    void onRenameGridFilterRequested(const QString& dataId);
    void onViewWellDataRequested(const QString& dataId, const QString& part, const QString& itemKey);
    void onNewWellRequested();
    void onImportWellRequested(const QString& dataId);
    void onDeleteWellRequested(const QString& dataId);
    void onGenerateInputRequested(const QString& dataId);
    void onViewPropertyCurvesRequested(const QString& caseId);
    void onImportSummaryResultsRequested(const QString& dataId);
    void onViewSummaryResultsRequested(const QString& dataId);
    void onRenameCaseRequested(const QString& dataId);
    void onNewCaseRequested();
    void onImportCaseRequested();
    void onImportGridRequested(const QString& caseId);
    void onDeleteCaseRequested(const QString& dataId);
    void onImportWellHeaderRequested(const QString& dataId);
    void onImportWellTrajectoryRequested(const QString& dataId);
    void onImportWellCurvesRequested(const QString& dataId);
    void onImportWellMarkersRequested(const QString& dataId);
    void onImportWellCompletionsRequested(const QString& dataId);
    void onImportWellCheckshotsRequested(const QString& dataId);
    void onCheckStateChanged(IDOSTreeItem* item, Qt::CheckState state);
    void onRenderStateChanged();

  private:
    void connectModel(IDOSTreeModel* model);
    void initializeModel(IDOSTreeModel* model);
    void updateModel(IDOSTreeModel* model, const QStringList& ids);
    void updateCheckStates(IDOSTreeModel* model, IDOSTreeItem* item);
    void applyChecked(IDOSTreeItem* item, Qt::CheckState state);
    void applyDataChecked(IDOSDataObject* data, Qt::CheckState state);
    Qt::CheckState checkState(IDOSDataObject* data) const;
    bool isCheckable(IDOSDataObject* data) const;
    IDOSRenderView* targetView() const;
    QPointer<IDOSProject> m_project;
    QPointer<IDOSDataTreeItemModel> m_dataTreeModel;
    QPointer<IDOSCaseTreeItemModel> m_caseTreeModel;
    QPointer<IDOSRenderServer> m_renderServer;
    QPointer<IDOSRenderView> m_targetView;
    bool m_applying;
};

#endif
