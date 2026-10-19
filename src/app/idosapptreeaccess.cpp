#include <QScopedValueRollback>
#include <QUuid>
#include <QVariantMap>

#include "grid/idosgridrenderprovider.h"
#include "idosdataobject.h"
#include "idosmainwindow.h"
#include "idosproject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "render/idosrenderserver.h"
#include "tree/idoscasetreeitemmodel.h"
#include "tree/idosdatatreeitemmodel.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeitemprovider.h"
#include "tree/idostreemodel.h"

#include "idosapptreeaccess.h"

IDOSAppTreeAccess::IDOSAppTreeAccess(IDOSProject* project, QObject* parent)
    :
    IDOSTreeAccess(parent)
    , m_project(project)
    , m_applying(false)
{
    if (project != nullptr)
    {
        connect(project, &QObject::destroyed, this, &IDOSAppTreeAccess::onProjectDestroyed);
        connect(project, &IDOSProject::objectAdded, this,
                static_cast<void (IDOSAppTreeAccess::*)(const QString&)>(&IDOSAppTreeAccess::onProjectChanged));
        connect(project, &IDOSProject::objectDataChanged, this,
                static_cast<void (IDOSAppTreeAccess::*)(const QString&)>(&IDOSAppTreeAccess::onProjectChanged));
        connect(project, &IDOSProject::objectRemoved, this,
                static_cast<void (IDOSAppTreeAccess::*)(const QString&)>(&IDOSAppTreeAccess::onProjectChanged));
        connect(project, &IDOSProject::objectsAdded, this,
                static_cast<void (IDOSAppTreeAccess::*)(const QStringList&)>(&IDOSAppTreeAccess::onProjectChanged));
        connect(project, &IDOSProject::objectsDataChanged, this,
                static_cast<void (IDOSAppTreeAccess::*)(const QStringList&)>(&IDOSAppTreeAccess::onProjectChanged));
        connect(project, &IDOSProject::objectsRemoved, this,
                static_cast<void (IDOSAppTreeAccess::*)(const QStringList&)>(&IDOSAppTreeAccess::onProjectChanged));
    }
}

IDOSAppTreeAccess::~IDOSAppTreeAccess()
{
    m_applying = true;
    if (m_project != nullptr)
    {
        disconnect(m_project.data(), nullptr, this, nullptr);
    }
    if (m_renderServer != nullptr)
    {
        disconnect(m_renderServer.data(), nullptr, this, nullptr);
    }
    if (m_dataTreeModel != nullptr)
    {
        disconnect(m_dataTreeModel.data(), nullptr, this, nullptr);
        m_dataTreeModel->clear();
        m_dataTreeModel->setSessionId(QString());
    }
    if (m_caseTreeModel != nullptr)
    {
        disconnect(m_caseTreeModel.data(), nullptr, this, nullptr);
        m_caseTreeModel->clear();
        m_caseTreeModel->setSessionId(QString());
    }
}

void IDOSAppTreeAccess::setDataModel(IDOSDataTreeItemModel* model)
{
    if (m_dataTreeModel == model)
    {
        return;
    }
    if (m_dataTreeModel != nullptr)
    {
        disconnect(m_dataTreeModel.data(), nullptr, this, nullptr);
        m_dataTreeModel->clear();
        m_dataTreeModel->setSessionId(QString());
    }
    m_dataTreeModel = model;
    connectModel(model);
    if (model != nullptr)
    {
        connect(model, &IDOSDataTreeItemModel::importDatasetRequested, this,
                &IDOSAppTreeAccess::onImportDatasetRequested);
        connect(model, &IDOSDataTreeItemModel::selectSeismicInlineRequested, this,
                &IDOSAppTreeAccess::onSelectSeismicInlineRequested);
        connect(model, &IDOSDataTreeItemModel::importFluidRockTablesRequested, this,
                &IDOSAppTreeAccess::onImportFluidRockTablesRequested);
        connect(model, &IDOSDataTreeItemModel::addFluidRockTableToCaseRequested, this,
                &IDOSAppTreeAccess::onAddFluidRockTableToCaseRequested);
        connect(model, &IDOSDataTreeItemModel::showGridSettingsRequested, this,
                &IDOSAppTreeAccess::onShowGridSettingsRequested);
        connect(model, &IDOSDataTreeItemModel::displayGridPropertyRequested, this,
                &IDOSAppTreeAccess::onDisplayGridPropertyRequested);
        connect(model, &IDOSDataTreeItemModel::showWellSettingsRequested, this,
                &IDOSAppTreeAccess::onShowWellSettingsRequested);
        connect(model, &IDOSDataTreeItemModel::removeRequested, this, &IDOSAppTreeAccess::onRemoveRequested);
        connect(model, &IDOSDataTreeItemModel::addToCaseRequested, this, &IDOSAppTreeAccess::onAddToCaseRequested);
        connect(model, &IDOSDataTreeItemModel::renameRequested, this,
                &IDOSAppTreeAccess::onRenameRequested);
        connect(model, &IDOSDataTreeItemModel::importGridPropertyRequested, this,
                &IDOSAppTreeAccess::onImportGridPropertyRequested);
        connect(model, &IDOSDataTreeItemModel::importGridDynamicPropertiesRequested, this,
                &IDOSAppTreeAccess::onImportGridDynamicPropertiesRequested);
        connect(model, &IDOSDataTreeItemModel::deleteGridPropertyRequested, this,
                &IDOSAppTreeAccess::onDeleteGridPropertyRequested);
        connect(model, &IDOSDataTreeItemModel::deleteGridRequested, this, &IDOSAppTreeAccess::onDeleteGridRequested);
        connect(model, &IDOSDataTreeItemModel::deleteGridFilterRequested, this,
                &IDOSAppTreeAccess::onDeleteGridFilterRequested);
        connect(model, &IDOSDataTreeItemModel::renameGridFilterRequested, this,
                &IDOSAppTreeAccess::onRenameGridFilterRequested);
        connect(model, &IDOSDataTreeItemModel::viewWellDataRequested, this,
                &IDOSAppTreeAccess::onViewWellDataRequested);
        connect(model, &IDOSDataTreeItemModel::newWellRequested, this, &IDOSAppTreeAccess::onNewWellRequested);
        connect(model, &IDOSDataTreeItemModel::importWellRequested, this, &IDOSAppTreeAccess::onImportWellRequested);
        connect(model, &IDOSDataTreeItemModel::importWellCheckshotsRequested, this,
                &IDOSAppTreeAccess::onImportWellCheckshotsRequested);
        connect(model, &IDOSDataTreeItemModel::importWellCompletionsRequested, this,
                &IDOSAppTreeAccess::onImportWellCompletionsRequested);
        connect(model, &IDOSDataTreeItemModel::importWellMarkersRequested, this,
                &IDOSAppTreeAccess::onImportWellMarkersRequested);
        connect(model, &IDOSDataTreeItemModel::importWellCurvesRequested, this,
                &IDOSAppTreeAccess::onImportWellCurvesRequested);
        connect(model, &IDOSDataTreeItemModel::importWellTrajectoryRequested, this,
                &IDOSAppTreeAccess::onImportWellTrajectoryRequested);
        connect(model, &IDOSDataTreeItemModel::importWellHeaderRequested, this,
                &IDOSAppTreeAccess::onImportWellHeaderRequested);
        connect(model, &IDOSDataTreeItemModel::deleteWellRequested, this, &IDOSAppTreeAccess::onDeleteWellRequested);
    }
}

void IDOSAppTreeAccess::setCaseModel(IDOSCaseTreeItemModel* model)
{
    if (m_caseTreeModel == model)
    {
        return;
    }
    if (m_caseTreeModel != nullptr)
    {
        disconnect(m_caseTreeModel.data(), nullptr, this, nullptr);
        m_caseTreeModel->clear();
        m_caseTreeModel->setSessionId(QString());
    }
    m_caseTreeModel = model;
    connectModel(model);
    if (model != nullptr)
    {
        connect(model, &IDOSCaseTreeItemModel::showGridSettingsRequested, this,
                &IDOSAppTreeAccess::onShowGridSettingsRequested);
        connect(model, &IDOSCaseTreeItemModel::displayGridPropertyRequested, this,
                &IDOSAppTreeAccess::onDisplayGridPropertyRequested);
        connect(model, &IDOSCaseTreeItemModel::showWellSettingsRequested, this,
                &IDOSAppTreeAccess::onShowWellSettingsRequested);
        connect(model, &IDOSCaseTreeItemModel::renameRequested, this,
                &IDOSAppTreeAccess::onRenameRequested);
        connect(model, &IDOSCaseTreeItemModel::importGridPropertyRequested, this,
                &IDOSAppTreeAccess::onImportGridPropertyRequested);
        connect(model, &IDOSCaseTreeItemModel::importGridDynamicPropertiesRequested, this,
                &IDOSAppTreeAccess::onImportGridDynamicPropertiesRequested);
        connect(model, &IDOSCaseTreeItemModel::deleteGridPropertyRequested, this,
                &IDOSAppTreeAccess::onDeleteGridPropertyRequested);
        connect(model, &IDOSCaseTreeItemModel::deleteGridRequested, this, &IDOSAppTreeAccess::onDeleteGridRequested);
        connect(model, &IDOSCaseTreeItemModel::deleteGridFilterRequested, this,
                &IDOSAppTreeAccess::onDeleteGridFilterRequested);
        connect(model, &IDOSCaseTreeItemModel::renameGridFilterRequested, this,
                &IDOSAppTreeAccess::onRenameGridFilterRequested);
        connect(model, &IDOSCaseTreeItemModel::viewWellDataRequested, this,
                &IDOSAppTreeAccess::onViewWellDataRequested);
        connect(model, &IDOSCaseTreeItemModel::importWellRequested, this, &IDOSAppTreeAccess::onImportWellRequested);
        connect(model, &IDOSCaseTreeItemModel::importWellCheckshotsRequested, this,
                &IDOSAppTreeAccess::onImportWellCheckshotsRequested);
        connect(model, &IDOSCaseTreeItemModel::importWellCompletionsRequested, this,
                &IDOSAppTreeAccess::onImportWellCompletionsRequested);
        connect(model, &IDOSCaseTreeItemModel::importWellMarkersRequested, this,
                &IDOSAppTreeAccess::onImportWellMarkersRequested);
        connect(model, &IDOSCaseTreeItemModel::importWellCurvesRequested, this,
                &IDOSAppTreeAccess::onImportWellCurvesRequested);
        connect(model, &IDOSCaseTreeItemModel::importWellTrajectoryRequested, this,
                &IDOSAppTreeAccess::onImportWellTrajectoryRequested);
        connect(model, &IDOSCaseTreeItemModel::importWellHeaderRequested, this,
                &IDOSAppTreeAccess::onImportWellHeaderRequested);
        connect(model, &IDOSCaseTreeItemModel::deleteWellRequested, this, &IDOSAppTreeAccess::onDeleteWellRequested);
        connect(model, &IDOSCaseTreeItemModel::generateInputRequested, this,
                &IDOSAppTreeAccess::onGenerateInputRequested);
        connect(model, &IDOSCaseTreeItemModel::viewPropertyCurvesRequested, this,
                &IDOSAppTreeAccess::onViewPropertyCurvesRequested);
        connect(model, &IDOSCaseTreeItemModel::importSummaryResultsRequested, this,
                &IDOSAppTreeAccess::onImportSummaryResultsRequested);
        connect(model, &IDOSCaseTreeItemModel::viewSummaryResultsRequested, this,
                &IDOSAppTreeAccess::onViewSummaryResultsRequested);
        connect(model, &IDOSCaseTreeItemModel::renameCaseRequested, this, &IDOSAppTreeAccess::onRenameCaseRequested);
        connect(model, &IDOSCaseTreeItemModel::newCaseRequested, this, &IDOSAppTreeAccess::onNewCaseRequested);
        connect(model, &IDOSCaseTreeItemModel::importCaseRequested, this, &IDOSAppTreeAccess::onImportCaseRequested);
        connect(model, &IDOSCaseTreeItemModel::importGridRequested, this, &IDOSAppTreeAccess::onImportGridRequested);
        connect(model, &IDOSCaseTreeItemModel::deleteCaseRequested, this, &IDOSAppTreeAccess::onDeleteCaseRequested);
    }
}

void IDOSAppTreeAccess::connectModel(IDOSTreeModel* model)
{
    if (model == nullptr)
    {
        return;
    }
    connect(model, &IDOSTreeModel::transferRequested, this, &IDOSAppTreeAccess::onTransferRequested,
            Qt::DirectConnection);
    connect(model, &IDOSTreeModel::checkStateChanged, this, &IDOSAppTreeAccess::onCheckStateChanged);
    connect(model, &IDOSTreeModel::registryChanged, this, &IDOSAppTreeAccess::onRegistryChanged);
    connect(model, &QAbstractItemModel::modelReset, this, &IDOSAppTreeAccess::onRegistryChanged);
    connect(model, &QAbstractItemModel::rowsInserted, this, &IDOSAppTreeAccess::onRenderStateChanged);
    initializeModel(model);
}

void IDOSAppTreeAccess::initializeModel(IDOSTreeModel* model)
{
    if (model == nullptr)
    {
        return;
    }
    QScopedValueRollback<bool> guard(m_applying, true);
    model->clear();
    model->setSessionId(QString());
    if (m_project != nullptr)
    {
        QString session = m_project->property("idosTreeTransferSession").toString();
        if (session.isEmpty())
        {
            session = QUuid::createUuid().toString(QUuid::WithoutBraces);
            m_project->setProperty("idosTreeTransferSession", session);
        }
        model->setSessionId(session);
        model->refresh();
        for (IDOSDataObject* data : m_project->objects())
        {
            model->addObject(data);
        }
    }
    updateCheckStates(model, model->root());
    model->refreshCheckStates();
}

void IDOSAppTreeAccess::setRenderServer(IDOSRenderServer* server)
{
    if (m_renderServer == server)
    {
        return;
    }
    if (m_renderServer != nullptr)
    {
        disconnect(m_renderServer.data(), nullptr, this, nullptr);
    }
    m_renderServer = server;
    if (server != nullptr)
    {
        connect(server, &IDOSRenderServer::stateChanged, this, &IDOSAppTreeAccess::onRenderStateChanged);
        connect(server, &IDOSRenderServer::currentViewChanged, this, &IDOSAppTreeAccess::onRenderStateChanged);
        connect(server, &QObject::destroyed, this, &IDOSAppTreeAccess::onRenderStateChanged);
    }
    onRenderStateChanged();
}

void IDOSAppTreeAccess::onProjectChanged(const QString& id)
{
    onProjectChanged(QStringList(id));
}

void IDOSAppTreeAccess::onProjectChanged(const QStringList& ids)
{
    QScopedValueRollback<bool> guard(m_applying, true);
    updateModel(m_dataTreeModel.data(), ids);
    updateModel(m_caseTreeModel.data(), ids);
    if (m_dataTreeModel != nullptr)
    {
        updateCheckStates(m_dataTreeModel.data(), m_dataTreeModel->root());
        m_dataTreeModel->refreshCheckStates();
    }
    if (m_caseTreeModel != nullptr)
    {
        updateCheckStates(m_caseTreeModel.data(), m_caseTreeModel->root());
        m_caseTreeModel->refreshCheckStates();
    }
}

void IDOSAppTreeAccess::onProjectDestroyed()
{
    m_project.clear();
    if (m_dataTreeModel != nullptr)
    {
        m_dataTreeModel->clear();
        m_dataTreeModel->setSessionId(QString());
    }
    if (m_caseTreeModel != nullptr)
    {
        m_caseTreeModel->clear();
        m_caseTreeModel->setSessionId(QString());
    }
}

void IDOSAppTreeAccess::onRegistryChanged()
{
    if (m_applying)
    {
        return;
    }
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    if (model == nullptr || m_project == nullptr)
    {
        return;
    }
    QStringList ids;
    for (IDOSDataObject* data : m_project->objects())
    {
        ids.append(data->objectId());
    }
    updateModel(model, ids);
    onRenderStateChanged();
}

void IDOSAppTreeAccess::onCheckStateChanged(IDOSTreeItem* item, Qt::CheckState state)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    if (m_applying || item == nullptr || model == nullptr || item->model() != model ||
        (model != m_dataTreeModel && model != m_caseTreeModel) || (state != Qt::Unchecked && state != Qt::Checked))
    {
        return;
    }
    m_targetView = targetView();
    if (m_targetView != nullptr)
    {
        m_applying = true;
        m_targetView->scene()->beginUpdate();
        applyChecked(item, state);
        if (m_targetView != nullptr)
        {
            m_targetView->endSceneUpdate(state == Qt::Checked);
        }
        m_applying = false;
        m_targetView.clear();
    }
    onRenderStateChanged();
}

void IDOSAppTreeAccess::applyChecked(IDOSTreeItem* item, Qt::CheckState state)
{
    if (item->provider()->isGroup())
    {
        for (IDOSTreeItem* child : item->children())
        {
            if (child->provider()->isCheckable())
            {
                applyChecked(child, state);
            }
        }
        return;
    }
    if (m_project != nullptr)
    {
        applyDataChecked(m_project->object(item->provider()->dataId()), state);
    }
}

void IDOSAppTreeAccess::onRenderStateChanged()
{
    if (m_applying)
    {
        return;
    }
    QScopedValueRollback<bool> guard(m_applying, true);
    if (m_dataTreeModel != nullptr)
    {
        updateCheckStates(m_dataTreeModel.data(), m_dataTreeModel->root());
        m_dataTreeModel->refreshCheckStates();
    }
    if (m_caseTreeModel != nullptr)
    {
        updateCheckStates(m_caseTreeModel.data(), m_caseTreeModel->root());
        m_caseTreeModel->refreshCheckStates();
    }
}

void IDOSAppTreeAccess::updateCheckStates(IDOSTreeModel* model, IDOSTreeItem* item)
{
    for (IDOSTreeItem* child : item->children())
    {
        updateCheckStates(model, child);
    }
    IDOSDataObject* data = m_project != nullptr ? m_project->object(item->provider()->dataId()) : nullptr;
    const QString renderId = IDOSGridRenderProvider::supportsData(data) ? IDOSGridRenderProvider::renderDataId(data)
                             : data != nullptr                          ? data->objectId()
                                                                        :
                                                                        QString();
    const bool available = m_renderServer != nullptr && targetView() != nullptr && !renderId.isEmpty() &&
                           !m_renderServer->objectIds(targetView(), renderId).isEmpty();
    const QModelIndex index = model->index(item);
    if (index.isValid() && model->data(index, IDOSTreeModel::RenderAvailableRole).toBool() != available)
    {
        model->setData(index, available, IDOSTreeModel::RenderAvailableRole);
    }
    if (!item->provider()->isGroup())
    {
        model->setCheckState(item, checkState(data), isCheckable(data));
    }
}

IDOSRenderView* IDOSAppTreeAccess::targetView() const
{
    return m_targetView != nullptr     ? m_targetView.data()
           : m_renderServer != nullptr ? m_renderServer->currentView()
                                       : nullptr;
}

bool IDOSAppTreeAccess::isCheckable(IDOSDataObject* data) const
{
    if (m_renderServer == nullptr || targetView() == nullptr || data == nullptr)
    {
        return false;
    }
    return m_renderServer->isCheckable(
        IDOSGridRenderProvider::supportsData(data) ? IDOSGridRenderProvider::renderDataId(data) : data->objectId());
}

void IDOSAppTreeAccess::updateModel(IDOSTreeModel* model, const QStringList& ids)
{
    if (model == nullptr || m_project == nullptr)
    {
        return;
    }
    QStringList affected;
    QList<IDOSTreeItem*> pending;
    pending.append(model->root());
    while (!pending.isEmpty())
    {
        IDOSTreeItem* item = pending.takeLast();
        IDOSTreeItemProvider* provider = item->provider();
        if (provider != nullptr && item->provider()->key() == QStringLiteral("object:") + item->provider()->dataId())
        {
            if (provider->dependsOn(ids))
            {
                affected.append(item->provider()->dataId());
            }
        }
        else
        {
            for (IDOSTreeItem* child : item->children())
            {
                pending.append(child);
            }
        }
    }
    for (const QString& id : ids)
    {
        if (!affected.contains(id))
        {
            affected.append(id);
        }
    }
    for (const QString& id : affected)
    {
        IDOSDataObject* data = m_project->object(id);
        if (data == nullptr)
        {
            model->removeItem(model->find(QStringLiteral("object:") + id));
        }
        else
        {
            model->addObject(data);
        }
    }
}

void IDOSAppTreeAccess::onTransferRequested(const QVariantMap& arguments, bool validateOnly, bool& accepted)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    accepted = false;
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    accepted = window->transferTreeItems(model, arguments, validateOnly);
}

void IDOSAppTreeAccess::applyDataChecked(IDOSDataObject* data, Qt::CheckState state)
{
    if (!isCheckable(data))
    {
        return;
    }
    if (IDOSGridRenderProvider::supportsData(data))
    {
        for (const QString& id : IDOSGridRenderProvider::requiredDataIds(data, state))
        {
            m_renderServer->setChecked(id, Qt::Checked, targetView());
        }
        IDOSGridRenderProvider::setDataChecked(data, state, targetView(), m_project->objects());
    }
    else
    {
        m_renderServer->setChecked(data->objectId(), state, targetView());
    }
}

Qt::CheckState IDOSAppTreeAccess::checkState(IDOSDataObject* data) const
{
    if (!isCheckable(data))
    {
        return Qt::Unchecked;
    }
    return IDOSGridRenderProvider::supportsData(data) ? IDOSGridRenderProvider::dataCheckState(data, targetView())
                                                      : m_renderServer->checkState(data->objectId(), targetView());
}

void IDOSAppTreeAccess::onImportDatasetRequested(const QString& text, const QString& fileFilter,
                                                 const QString& providerId, const QString& dataTypeId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onNodeImportDataset(view, text, fileFilter, providerId, dataTypeId);
}

void IDOSAppTreeAccess::onSelectSeismicInlineRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onNodeSelectSeismicInline(view, dataId, targetView());
}

void IDOSAppTreeAccess::onImportFluidRockTablesRequested(const QString& category)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onNodeImportFluidRockTables(view, category);
}

void IDOSAppTreeAccess::onAddFluidRockTableToCaseRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onNodeAddFluidRockTableToCase(view, dataId);
}

void IDOSAppTreeAccess::onShowGridSettingsRequested(const QString& gridId, const QString& filterId,
                                                    const QString& propertyId, bool openFilter)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataGridSettingsTriggered(view, gridId, filterId, propertyId, openFilter, targetView());
}

void IDOSAppTreeAccess::onDisplayGridPropertyRequested(const QString& gridId, const QString& propertyId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataGridDisplayTriggered(view, gridId, propertyId, targetView());
}

void IDOSAppTreeAccess::onShowWellSettingsRequested(const QString& wellId, bool trajectory)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataWellSettingsTriggered(view, wellId, trajectory, targetView());
}

void IDOSAppTreeAccess::onRemoveRequested(const QStringList& objectIds)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataDeleteDatasets(view, objectIds);
}

void IDOSAppTreeAccess::onAddToCaseRequested(const QJsonArray& items)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataAddToCase(view, items);
}

void IDOSAppTreeAccess::onRenameRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    if (model == m_caseTreeModel)
    {
        window->onCaseRenameObject(view, dataId);
    }
    else
    {
        window->onDataRenameObject(view, dataId);
    }
}

void IDOSAppTreeAccess::onImportGridPropertyRequested(const QString& dataId, const QString& caseId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataGridImportProperty(view, dataId, caseId);
}

void IDOSAppTreeAccess::onImportGridDynamicPropertiesRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataGridImportDynamicProperties(view, dataId);
}

void IDOSAppTreeAccess::onDeleteGridPropertyRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataGridDeleteProperty(view, dataId);
}

void IDOSAppTreeAccess::onDeleteGridRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataGridDelete(view, dataId);
}

void IDOSAppTreeAccess::onDeleteGridFilterRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataGridDeleteFilter(view, dataId);
}

void IDOSAppTreeAccess::onRenameGridFilterRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataGridRenameFilter(view, dataId);
}

void IDOSAppTreeAccess::onViewWellDataRequested(const QString& dataId, const QString& part, const QString& itemKey)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataViewWellData(view, dataId, part, itemKey);
}

void IDOSAppTreeAccess::onNewWellRequested()
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataNewWell(view);
}

void IDOSAppTreeAccess::onImportWellRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataImportWell(view, dataId);
}

void IDOSAppTreeAccess::onDeleteWellRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataDeleteWell(view, dataId);
}

void IDOSAppTreeAccess::onGenerateInputRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseGenerateInput(view, dataId);
}

void IDOSAppTreeAccess::onViewPropertyCurvesRequested(const QString& caseId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseViewPropertyCurves(view, caseId);
}

void IDOSAppTreeAccess::onImportSummaryResultsRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseImportSummaryResults(view, dataId);
}

void IDOSAppTreeAccess::onViewSummaryResultsRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseViewSummaryResults(view, dataId);
}

void IDOSAppTreeAccess::onRenameCaseRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseRenameCase(view, dataId);
}

void IDOSAppTreeAccess::onNewCaseRequested()
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseNewCase(view);
}

void IDOSAppTreeAccess::onImportCaseRequested()
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseImportCase(view);
}

void IDOSAppTreeAccess::onImportGridRequested(const QString& caseId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseImportGrid(view, caseId);
}

void IDOSAppTreeAccess::onDeleteCaseRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onCaseDeleteCase(view, dataId);
}

void IDOSAppTreeAccess::onImportWellHeaderRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataImportWellHeader(view, dataId);
}

void IDOSAppTreeAccess::onImportWellTrajectoryRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataImportWellTrajectory(view, dataId);
}

void IDOSAppTreeAccess::onImportWellCurvesRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataImportWellCurves(view, dataId);
}

void IDOSAppTreeAccess::onImportWellMarkersRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataImportWellMarkers(view, dataId);
}

void IDOSAppTreeAccess::onImportWellCompletionsRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataImportWellCompletions(view, dataId);
}

void IDOSAppTreeAccess::onImportWellCheckshotsRequested(const QString& dataId)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr || window->project() != m_project ||
        (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    IDOSTreeView* view =
        model == m_dataTreeModel ? static_cast<IDOSTreeView*>(window->dataTreeView()) : window->caseTreeView();
    window->onDataImportWellCheckshots(view, dataId);
}
