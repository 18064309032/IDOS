#include <QMap>

#include "idosdataobject.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderregistry.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "providers/idosrendermetadata.h"
#include "providers/idosrenderprovider.h"

#include "idosrenderserver.h"

IDOSRenderServer::IDOSRenderServer(IDOSRenderRegistry* registry, QObject* parent)
    :
    QObject(parent)
    , m_registry(registry)
{
    if (registry != nullptr)
    {
        connect(registry, &IDOSRenderRegistry::metadataAdded, this, &IDOSRenderServer::onMetadataAdded);
        connect(registry, &IDOSRenderRegistry::metadataAboutToBeRemoved, this,
                &IDOSRenderServer::onMetadataAboutToBeRemoved);
    }
}

IDOSRenderServer::~IDOSRenderServer()
{
    const QList<IDOSRenderView*> registered = m_views;
    for (IDOSRenderView* view : registered)
    {
        removeView(view);
    }
}

IDOSProject* IDOSRenderServer::project() const
{
    return m_project.data();
}

void IDOSRenderServer::setProject(IDOSProject* project)
{
    if (m_project == project)
    {
        return;
    }
    if (m_project != nullptr)
    {
        disconnect(m_project.data(), nullptr, this, nullptr);
    }
    m_project = project;
    if (project != nullptr)
    {
        connect(project, &QObject::destroyed, this, &IDOSRenderServer::onProjectDestroyed);
        connect(project, &IDOSProject::objectRemoved, this, &IDOSRenderServer::onObjectRemoved);
        connect(project, &IDOSProject::objectsRemoved, this, &IDOSRenderServer::onObjectsRemoved);
        connect(project, &IDOSProject::objectAdded, this, &IDOSRenderServer::onObjectAdded);
        connect(project, &IDOSProject::objectsAdded, this, &IDOSRenderServer::onObjectsAdded);
        connect(project, &IDOSProject::objectDataChanged, this, &IDOSRenderServer::onObjectChanged);
        connect(project, &IDOSProject::objectsDataChanged, this, &IDOSRenderServer::onObjectsChanged);
        connect(project, &IDOSProject::objectAboutToBeRemoved, this, &IDOSRenderServer::onObjectAboutToBeRemoved);
    }
    reload();
}

void IDOSRenderServer::reload()
{
    for (IDOSRenderView* view : m_views)
    {
        view->scene()->beginUpdate();
        view->scene()->clear();
        populate(view);
        view->scene()->endUpdate();
    }
}

void IDOSRenderServer::addView(IDOSRenderView* view)
{
    if (view == nullptr || m_views.contains(view))
    {
        return;
    }
    m_views.append(view);
    connect(view, &QObject::destroyed, this, &IDOSRenderServer::onViewDestroyed);
    {
        connect(view->scene(), &IDOSRenderScene::objectAdded, this,
                &IDOSRenderServer::onSceneChanged, Qt::UniqueConnection);
        connect(view->scene(), &IDOSRenderScene::objectRemoved, this,
                &IDOSRenderServer::onSceneChanged, Qt::UniqueConnection);
        connect(view->scene(), &IDOSRenderScene::objectChanged, this,
                &IDOSRenderServer::onSceneChanged, Qt::UniqueConnection);
    }
    populate(view);
    if (m_currentView.isNull())
    {
        setCurrentView(view);
    }
}

void IDOSRenderServer::removeView(IDOSRenderView* view)
{
    if (!m_views.removeOne(view))
    {
        return;
    }
    disconnect(view, nullptr, this, nullptr);
    disconnect(view->scene(), nullptr, this, nullptr);
    if (m_currentView == view)
    {
        setCurrentView(m_views.isEmpty() ? nullptr : m_views.first());
    }
    view->scene()->clear();
}

QList<IDOSRenderView*> IDOSRenderServer::views() const
{
    return m_views;
}

IDOSRenderView* IDOSRenderServer::currentView() const
{
    return m_currentView.data();
}

bool IDOSRenderServer::setCurrentView(IDOSRenderView* view)
{
    if (view != nullptr && !m_views.contains(view))
    {
        return false;
    }
    if (m_currentView == view)
    {
        return true;
    }
    if (m_currentView != nullptr)
    {
        m_currentView->setActive(false);
    }
    m_currentView = view;
    if (view != nullptr)
    {
        view->setActive(true);
    }
    emit currentViewChanged(view);
    emit stateChanged();
    return true;
}

bool IDOSRenderServer::isCheckable(const QString& dataId) const
{
    IDOSDataObject* data = m_project != nullptr ? m_project->object(dataId) : nullptr;
    return data != nullptr && m_registry != nullptr && m_registry->metadata(data->typeId()) != nullptr;
}

Qt::CheckState IDOSRenderServer::checkState(const QString& dataId, IDOSRenderView* view) const
{
    bool checked = false;
    bool unchecked = false;
    for (const QString& id : objectIds(view, dataId))
    {
        IDOSRenderObject* object = view->scene()->object(id);
        checked = checked || object->visible();
        unchecked = unchecked || !object->visible();
    }
    return checked ? (unchecked ? Qt::PartiallyChecked : Qt::Checked) : Qt::Unchecked;
}

void IDOSRenderServer::setChecked(const QString& dataId, Qt::CheckState state, IDOSRenderView* view)
{
    if (view == nullptr || !m_views.contains(view) || !isCheckable(dataId) || state == Qt::PartiallyChecked)
    {
        return;
    }
    if (state == Qt::Checked && objectIds(view, dataId).isEmpty())
    {
        update(view, m_project->object(dataId), false, true);
    }
    for (const QString& id : objectIds(view, dataId))
    {
        IDOSRenderObject* object = view->scene()->object(id);
        object->setVisible(state == Qt::Checked);
        view->scene()->notifyChanged(id);
    }
}

void IDOSRenderServer::onSceneChanged()
{
    emit stateChanged();
}

QList<QString> IDOSRenderServer::objectIds(IDOSRenderView* view, const QString& dataId) const
{
    QList<QString> result;
    if (view == nullptr || !m_views.contains(view))
    {
        return result;
    }
    const QList<IDOSRenderObject*> objects = view->scene()->objects();
    for (IDOSRenderObject* object : objects)
    {
        IDOSDataObject* source = object->provider()->dataSource();
        if (source != nullptr && source->objectId() == dataId)
        {
            result.append(object->id());
        }
    }
    return result;
}

IDOSDataObject* IDOSRenderServer::dataSource(IDOSRenderView* view, const QString& objectId) const
{
    IDOSRenderObject* object = view != nullptr && m_views.contains(view) ? view->scene()->object(objectId) : nullptr;
    return object != nullptr ? object->provider()->dataSource() : nullptr;
}

void IDOSRenderServer::populate(IDOSRenderView* view)
{
    if (m_project == nullptr)
    {
        return;
    }
    QStringList ids;
    view->scene()->beginUpdate();
    const QList<IDOSDataObject*> objects = m_project->objects();
    for (IDOSDataObject* data : objects)
    {
        update(view, data, true);
        ids.append(data->objectId());
    }
    emit objectsChanged(view, ids, true);
    view->scene()->endUpdate();
    view->resetCamera();
}

bool IDOSRenderServer::update(IDOSRenderView* view, IDOSDataObject* data, bool dataChanged, bool create)
{
    if (m_registry == nullptr)
    {
        return false;
    }
    IDOSRenderMetadata* metadata = m_registry->metadata(data->typeId());
    if (metadata == nullptr)
    {
        return false;
    }
    const QList<QString> ids = objectIds(view, data->objectId());
    if (ids.isEmpty())
    {
        if (!create)
        {
            return false;
        }
        IDOSRenderProvider* provider = metadata->create(data);
        if (provider == nullptr)
        {
            return false;
        }
        IDOSRenderObject* object = new IDOSRenderObject(provider);
        object->setName(data->name());
        object->setVisible(false);
        bindDataReferences(object->provider());
        object->update();
        if (!view->scene()->add(object))
        {
            delete object;
            return false;
        }
        return object->visible();
    }
    for (const QString& id : ids)
    {
        IDOSRenderObject* object = view->scene()->object(id);
        object->setName(data->name());
        if (dataChanged)
        {
            bindDataReferences(object->provider());
            object->update();
        }
        view->scene()->notifyChanged(id);
    }
    return false;
}

void IDOSRenderServer::update(const QStringList& ids, bool dataChanged)
{
    if (m_project == nullptr)
    {
        return;
    }
    for (IDOSRenderView* view : m_views)
    {
        bool created = false;
        view->scene()->beginUpdate();
        for (const QString& id : ids)
        {
            IDOSDataObject* data = m_project->object(id);
            if (data != nullptr)
            {
                created = update(view, data, dataChanged) || created;
            }
        }
        if (dataChanged)
        {
            for (IDOSRenderObject* object : view->scene()->objects())
            {
                IDOSDataObject* source = object->provider()->dataSource();
                if (source != nullptr && !ids.contains(source->objectId()) && object->provider()->dependsOn(ids))
                {
                    bindDataReferences(object->provider());
            object->update();
                    view->scene()->notifyChanged(object->id());
                }
            }
        }
        emit objectsChanged(view, ids, dataChanged);
        view->scene()->endUpdate();
        if (created)
        {
            view->resetCamera();
        }
    }
}

void IDOSRenderServer::onProjectDestroyed()
{
    m_project = nullptr;
    reload();
}

void IDOSRenderServer::onViewDestroyed(QObject* object)
{
    const bool wasCurrent = m_currentView.isNull() || m_currentView.data() == object;
    for (int index = m_views.size() - 1; index >= 0; --index)
    {
        if (m_views.at(index) == object)
        {
            m_views.removeAt(index);
        }
    }
    if (wasCurrent)
    {
        m_currentView = nullptr;
        if (m_views.isEmpty())
        {
            emit currentViewChanged(nullptr);
        }
        else
        {
            setCurrentView(m_views.first());
        }
    }
}

void IDOSRenderServer::onObjectAboutToBeRemoved(const QString& id)
{
    emit dataAboutToBeRemoved(id);
    for (IDOSRenderView* view : m_views)
    {
        view->scene()->beginUpdate();
        const QList<QString> ids = objectIds(view, id);
        for (const QString& objectId : ids)
        {
            view->scene()->remove(objectId);
        }
        for (IDOSRenderObject* object : view->scene()->objects())
        {
            if (object->provider()->removeDataReference(id))
            {
                object->update();
                view->scene()->notifyChanged(object->id());
            }
        }
        view->scene()->endUpdate();
    }
}

void IDOSRenderServer::onObjectAdded(const QString& id)
{
    update(QStringList{id}, true);
}

void IDOSRenderServer::onObjectsAdded(const QStringList& ids)
{
    update(ids, true);
}

void IDOSRenderServer::onObjectChanged(const QString& id)
{
    update(QStringList{id}, true);
}

void IDOSRenderServer::onObjectsChanged(const QStringList& ids)
{
    update(ids, true);
}

void IDOSRenderServer::onMetadataAdded(const QString& typeId)
{
    if (m_project == nullptr)
    {
        return;
    }
    QStringList ids;
    const QList<IDOSDataObject*> objects = m_project->objects();
    for (IDOSDataObject* data : objects)
    {
        if (data->typeId() == typeId)
        {
            ids.append(data->objectId());
        }
    }
    update(ids, true);
}

void IDOSRenderServer::onMetadataAboutToBeRemoved(const QString& typeId)
{
    for (IDOSRenderView* view : m_views)
    {
        const QList<IDOSRenderObject*> objects = view->scene()->objects();
        view->scene()->beginUpdate();
        for (IDOSRenderObject* object : objects)
        {
            IDOSDataObject* data = object->provider()->dataSource();
            if (data != nullptr && data->typeId() == typeId)
            {
                view->scene()->remove(object->id());
            }
        }
        view->scene()->endUpdate();
    }
}

void IDOSRenderServer::onObjectRemoved(const QString& id)
{
    update(QStringList{id}, false);
}

void IDOSRenderServer::onObjectsRemoved(const QStringList& ids)
{
    update(ids, false);
}

void IDOSRenderServer::bindDataReferences(IDOSRenderProvider* provider)
{
    if (m_project == nullptr || provider == nullptr)
    {
        return;
    }
    for (const QString& id : provider->dependencyIds())
    {
        IDOSDataObject* data = m_project->object(id);
        if (data != nullptr)
        {
            provider->setDataReference(data);
        }
    }
}

QVariantMap IDOSRenderServer::presentationState() const
{
    QVariantMap state;
    QVariantList views;
    for (IDOSRenderView* view : m_views)
    {
        QVariantMap entry;
        entry.insert(QStringLiteral("id"), view->property("viewId").toString());
        entry.insert(QStringLiteral("view"), view->presentationState());
        for (IDOSRenderView* source : m_views)
        {
            if (!source->isCameraLinked() && view->isCameraLinkedTo(source))
            {
                entry.insert(QStringLiteral("linkedView"), source->property("viewId").toString());
                break;
            }
        }
        QMap<QString, QVariant> objects;
        QStringList selection;
        for (IDOSRenderObject* object : view->scene()->objects())
        {
            IDOSDataObject* data = object->provider()->dataSource();
            if (data == nullptr)
            {
                continue;
            }
            QVariantMap item;
            item.insert(QStringLiteral("dataId"), data->objectId());
            item.insert(QStringLiteral("typeId"), data->typeId());
            item.insert(QStringLiteral("provider"), object->provider()->presentationState());
            objects.insert(data->objectId(), item);
            if (view->selection().contains(object->id()))
            {
                selection.append(data->objectId());
            }
        }
        selection.sort();
        entry.insert(QStringLiteral("objects"), objects.values());
        entry.insert(QStringLiteral("selection"), selection);
        views.append(entry);
    }
    state.insert(QStringLiteral("views"), views);
    state.insert(QStringLiteral("currentView"), m_currentView != nullptr ? m_currentView->property("viewId").toString() : QString());
    return state;
}

void IDOSRenderServer::restorePresentationState(const QVariantMap& state)
{
    const QVariantList entries = state.value(QStringLiteral("views")).toList();
    for (IDOSRenderView* view : m_views)
    {
        view->unlinkCamera();
    }
    for (const QVariant& value : entries)
    {
        const QVariantMap entry = value.toMap();
        for (IDOSRenderView* view : m_views)
        {
            if (view->property("viewId").toString() != entry.value(QStringLiteral("id")).toString())
            {
                continue;
            }
            view->scene()->beginUpdate();
            const QVariantList objects = entry.value(QStringLiteral("objects")).toList();
            for (const QVariant& item : objects)
            {
                const QVariantMap saved = item.toMap();
                const QString dataId = saved.value(QStringLiteral("dataId")).toString();
                IDOSDataObject* data = m_project != nullptr ? m_project->object(dataId) : nullptr;
                if (data == nullptr || data->typeId() != saved.value(QStringLiteral("typeId")).toString())
                {
                    continue;
                }
                update(view, data, false, true);
                for (const QString& id : objectIds(view, dataId))
                {
                    IDOSRenderObject* object = view->scene()->object(id);
                    const QVariantMap previous = object->provider()->presentationState();
                    if (!object->provider()->restorePresentationState(saved.value(QStringLiteral("provider")).toMap()))
                    {
                        object->provider()->restorePresentationState(previous);
                    }
                    bindDataReferences(object->provider());
                    object->update();
                    view->scene()->notifyChanged(id);
                }
            }
            view->scene()->endUpdate();
            QList<QString> selected;
            for (const QString& dataId : entry.value(QStringLiteral("selection")).toStringList())
            {
                selected.append(objectIds(view, dataId));
            }
            view->setSelection(selected);
            view->restorePresentationState(entry.value(QStringLiteral("view")).toMap());
        }
    }
    for (const QVariant& value : entries)
    {
        const QVariantMap entry = value.toMap();
        for (IDOSRenderView* view : m_views)
        {
            if (view->property("viewId").toString() != entry.value(QStringLiteral("id")).toString())
            {
                continue;
            }
            for (IDOSRenderView* source : m_views)
            {
                if (source->property("viewId").toString() == entry.value(QStringLiteral("linkedView")).toString() &&
                    !source->isCameraLinked())
                {
                    view->linkCameraTo(source);
                    break;
                }
            }
        }
    }
    for (IDOSRenderView* view : m_views)
    {
        if (view->property("viewId").toString() == state.value(QStringLiteral("currentView")).toString())
        {
            setCurrentView(view);
            break;
        }
    }
}
