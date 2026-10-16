#include <QScopedValueRollback>
#include <QUuid>
#include <QVariantMap>

#include "idosdataobject.h"
#include "idosmainwindow.h"
#include "idosproject.h"
#include "idosrenderscene.h"
#include "idosrenderserver.h"
#include "idosrenderview.h"
#include "providers/grid/idosgridrenderprovider.h"
#include "providers/idostreeitemprovider.h"
#include "tree/idostreeitem.h"
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

void IDOSAppTreeAccess::setDataTreeModel(IDOSTreeModel* model)
{
    if (m_dataTreeModel == model || (model != nullptr && model == m_caseTreeModel))
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
}

void IDOSAppTreeAccess::setCaseTreeModel(IDOSTreeModel* model)
{
    if (m_caseTreeModel == model || (model != nullptr && model == m_dataTreeModel))
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
}

void IDOSAppTreeAccess::connectModel(IDOSTreeModel* model)
{
    if (model == nullptr)
    {
        return;
    }
    connect(model, &IDOSTreeModel::transferRequested, this, &IDOSAppTreeAccess::onTransferRequested, Qt::DirectConnection);
    connect(model, &IDOSTreeModel::actionTriggered, this, &IDOSAppTreeAccess::onActionTriggered);
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
        (model != m_dataTreeModel && model != m_caseTreeModel) ||
        (state != Qt::Unchecked && state != Qt::Checked))
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
            m_targetView->scene()->endUpdate();
            if (state == Qt::Checked)
            {
                m_targetView->resetCamera();
            }
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
        : data != nullptr ? data->objectId() : QString();
    const bool available = m_renderServer != nullptr && targetView() != nullptr && !renderId.isEmpty() &&
        !m_renderServer->objectIds(targetView(), renderId).isEmpty();
    const QModelIndex index = model->indexFromItem(item);
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
    return m_targetView != nullptr ? m_targetView.data()
        : m_renderServer != nullptr ? m_renderServer->currentView() : nullptr;
}

bool IDOSAppTreeAccess::isCheckable(IDOSDataObject* data) const
{
    if (m_renderServer == nullptr || targetView() == nullptr || data == nullptr)
    {
        return false;
    }
    return m_renderServer->isCheckable(IDOSGridRenderProvider::supportsData(data)
        ? IDOSGridRenderProvider::renderDataId(data) : data->objectId());
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

void IDOSAppTreeAccess::onActionTriggered(const QString& operation, const QStringList& itemIds,
                                         const QVariantMap& arguments)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    if (m_project == nullptr || model == nullptr || window == nullptr ||
        window->project() != m_project || (model != m_dataTreeModel && model != m_caseTreeModel))
    {
        return;
    }
    for (const QString& id : itemIds)
    {
        if (model->item(id) == nullptr)
        {
            return;
        }
    }
    window->executeTreeAction(model, operation, arguments, targetView());
}

void IDOSAppTreeAccess::onTransferRequested(const QVariantMap& arguments, bool validateOnly, bool& accepted)
{
    IDOSTreeModel* model = qobject_cast<IDOSTreeModel*>(sender());
    IDOSMainWindow* window = qobject_cast<IDOSMainWindow*>(parent());
    accepted = false;
    if (m_project == nullptr || model == nullptr || window == nullptr ||
        window->project() != m_project || (model != m_dataTreeModel && model != m_caseTreeModel))
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
