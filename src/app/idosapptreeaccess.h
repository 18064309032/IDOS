#ifndef IDOS_APP_TREE_ACCESS_H
#define IDOS_APP_TREE_ACCESS_H

#include <QPointer>
#include <QStringList>
#include <QVariantMap>
#include <QtCore/qnamespace.h>

#include "idos_app.h"
#include "tree/idostreeaccess.h"

class IDOSDataObject;
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
    void setDataTreeModel(IDOSTreeModel* model);
    void setCaseTreeModel(IDOSTreeModel* model);
    void setRenderServer(IDOSRenderServer* server);
private slots:
    void onProjectChanged(const QString& id);
    void onProjectChanged(const QStringList& ids);
    void onProjectDestroyed();
    void onRegistryChanged();
    void onTransferRequested(const QVariantMap& arguments, bool validateOnly, bool& accepted);
    void onActionTriggered(const QString& operation, const QStringList& itemIds,
                           const QVariantMap& arguments);
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
    QPointer<IDOSTreeModel> m_dataTreeModel;
    QPointer<IDOSTreeModel> m_caseTreeModel;
    QPointer<IDOSRenderServer> m_renderServer;
    QPointer<IDOSRenderView> m_targetView;
    bool m_applying;
};

#endif
