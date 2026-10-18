#include <QMap>
#include <QSet>

#include "idosdataobject.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderprovidermetadata.h"
#include "idosrenderproviderregistry.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"

#include "idosrenderserver.h"

IDOSRenderServer::IDOSRenderServer(IDOSRenderProviderRegistry* registry, QObject* parent)
    :
    QObject(parent)
    , m_registry(registry)
{
    if (registry != nullptr)
    {
        connect(registry, &IDOSRenderProviderRegistry::metadataRegistered, this, &IDOSRenderServer::onMetadataRegistered);
        connect(registry, &IDOSRenderProviderRegistry::metadataUnregistering, this,
                &IDOSRenderServer::onMetadataUnregistering);
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
    m_removingIds.clear();
    m_removedStates.clear();
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
                &IDOSRenderServer::onRenderObjectRemoved, Qt::UniqueConnection);
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
    m_removedStates.remove(view);
    if (m_currentView == view)
    {
        setCurrentView(m_views.isEmpty() ? nullptr : m_views.first());
    }
    view->scene()->clear();
    disconnect(view->scene(), nullptr, this, nullptr);
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
        checked = checked || object->isVisible();
        unchecked = unchecked || !object->isVisible();
    }
    return checked ? (unchecked ? Qt::PartiallyChecked : Qt::Checked) : Qt::Unchecked;
}

void IDOSRenderServer::setChecked(const QString& dataId, Qt::CheckState state, IDOSRenderView* view)
{
    if (view == nullptr || !m_views.contains(view) || !isCheckable(dataId) ||
        (state != Qt::Checked && state != Qt::Unchecked))
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
        IDOSDataObject* source = dataObject(view, object->id());
        if (source != nullptr && source->objectId() == dataId)
        {
            result.append(object->id());
        }
    }
    return result;
}

IDOSDataObject* IDOSRenderServer::dataObject(IDOSRenderView* view, const QString& objectId) const
{
    if (m_project == nullptr || view == nullptr || !m_views.contains(view) || !view->scene()->contains(objectId))
    {
        return nullptr;
    }
    return m_project->object(m_sources.value(objectId));
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
    if (m_registry == nullptr || data == nullptr || view == nullptr || !m_views.contains(view))
    {
        return false;
    }
    IDOSRenderProviderMetadata* metadata = m_registry->metadata(data->typeId());
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
        IDOSRenderProvider* provider = metadata->create(data, references(metadata->dependencies(data, QVariantMap())));
        if (provider == nullptr)
        {
            return false;
        }
        IDOSRenderObject* object = new IDOSRenderObject(provider);
        object->setName(data->name());
        object->setVisible(false);
        m_sources.insert(object->id(), data->objectId());
        object->update();
        if (!view->scene()->add(object))
        {
            m_sources.remove(object->id());
            delete object;
            return false;
        }
        return true;
    }
    for (const QString& id : ids)
    {
        IDOSRenderObject* object = view->scene()->object(id);
        object->setName(data->name());
        if (dataChanged)
        {
            if (!refreshProvider(object, data, object->provider()->saveState()))
            {
                view->scene()->remove(id);
                continue;
            }
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
            restoreRemovedObjects(view);
            for (IDOSRenderObject* object : view->scene()->objects())
            {
                IDOSDataObject* source = dataObject(view, object->id());
                if (source == nullptr || ids.contains(source->objectId()))
                {
                    continue;
                }
                const QStringList dependencies = metadataDependencies(object, source);
                bool affected = false;
                for (const QString& id : ids)
                {
                    affected = affected || dependencies.contains(id);
                }
                if (affected)
                {
                    if (refreshProvider(object, source, object->provider()->saveState()))
                    {
                        view->scene()->notifyChanged(object->id());
                    }
                    else
                    {
                        view->scene()->remove(object->id());
                    }
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
    m_removingIds.clear();
    m_removedStates.clear();
    reload();
}

void IDOSRenderServer::onViewDestroyed(QObject* object)
{
    const bool wasCurrent = m_currentView.isNull() || m_currentView.data() == object;
    for (int index = m_views.size() - 1; index >= 0; --index)
    {
        if (m_views.at(index) == object)
        {
            m_removedStates.remove(m_views.at(index));
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
    m_removingIds.append(id);
    emit dataAboutToBeRemoved(id);
    for (IDOSRenderView* view : m_views)
    {
        view->scene()->beginUpdate();
        const QList<IDOSRenderObject*> objects = view->scene()->objects();
        for (IDOSRenderObject* object : objects)
        {
            if (m_sources.value(object->id()) == id)
            {
                QVariantMap saved;
                IDOSDataObject* source = dataObject(view, object->id());
                saved.insert(QStringLiteral("typeId"), source != nullptr ? source->typeId() : QString());
                saved.insert(QStringLiteral("provider"), object->provider()->saveState());
                saved.insert(QStringLiteral("selected"), view->selection().contains(object->id()));
                m_removedStates[view].insert(id, saved);
                view->scene()->remove(object->id());
                continue;
            }
            IDOSDataObject* data = dataObject(view, object->id());
            if (metadataDependencies(object, data).contains(id))
            {
                const QVariantMap previous = object->provider()->saveState();
                if (refreshProvider(object, data, object->provider()->saveState()))
                {
                    view->scene()->notifyChanged(object->id());
                }
                else
                {
                    QVariantMap saved;
                    saved.insert(QStringLiteral("typeId"), data->typeId());
                    saved.insert(QStringLiteral("provider"), previous);
                    saved.insert(QStringLiteral("selected"), view->selection().contains(object->id()));
                    m_removedStates[view].insert(data->objectId(), saved);
                    view->scene()->remove(object->id());
                }
            }
        }
        view->scene()->endUpdate();
    }
}

void IDOSRenderServer::restoreRemovedObjects(IDOSRenderView* view)
{
    if (m_project == nullptr || !m_removedStates.contains(view))
    {
        return;
    }
    const QStringList dataIds = m_removedStates.value(view).keys();
    for (const QString& dataId : dataIds)
    {
        IDOSDataObject* data = m_project->object(dataId);
        const QVariantMap saved = m_removedStates.value(view).value(dataId);
        if (data == nullptr || m_removingIds.contains(dataId) ||
            data->typeId() != saved.value(QStringLiteral("typeId")).toString())
        {
            continue;
        }
        const QVariantMap state = saved.value(QStringLiteral("provider")).toMap();
        IDOSRenderProviderMetadata* metadata = m_registry != nullptr ? m_registry->metadata(data->typeId()) : nullptr;
        if (metadata == nullptr)
        {
            continue;
        }
        bool available = true;
        for (const QString& dependency : metadata->dependencies(data, state))
        {
            available = available && m_project->object(dependency) != nullptr && !m_removingIds.contains(dependency);
        }
        if (!available)
        {
            continue;
        }
        update(view, data, false, true);
        const QList<QString> ids = objectIds(view, dataId);
        bool restored = !ids.isEmpty();
        for (const QString& id : ids)
        {
            IDOSRenderObject* object = view->scene()->object(id);
            if (!refreshProvider(object, data, state))
            {
                restored = false;
                view->scene()->remove(id);
                continue;
            }
            if (saved.value(QStringLiteral("selected")).toBool())
            {
                QList<QString> selected = view->selection();
                selected.append(id);
                view->setSelection(selected);
            }
            view->scene()->notifyChanged(id);
        }
        if (restored)
        {
            m_removedStates[view].remove(dataId);
        }
    }
}

void IDOSRenderServer::onObjectAdded(const QString& id)
{
    m_removingIds.removeAll(id);
    update(QStringList{id}, true);
}

void IDOSRenderServer::onObjectsAdded(const QStringList& ids)
{
    for (const QString& id : ids)
    {
        m_removingIds.removeAll(id);
    }
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

void IDOSRenderServer::onMetadataRegistered(const QString& typeId)
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

void IDOSRenderServer::onMetadataUnregistering(const QString& typeId)
{
    for (IDOSRenderView* view : m_views)
    {
        const QList<IDOSRenderObject*> objects = view->scene()->objects();
        view->scene()->beginUpdate();
        for (IDOSRenderObject* object : objects)
        {
            IDOSDataObject* data = dataObject(view, object->id());
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
    m_removingIds.removeAll(id);
    update(QStringList{id}, false);
}

void IDOSRenderServer::onObjectsRemoved(const QStringList& ids)
{
    for (const QString& id : ids)
    {
        m_removingIds.removeAll(id);
    }
    update(ids, false);
}

QList<IDOSDataObject*> IDOSRenderServer::references(const QStringList& ids) const
{
    QList<IDOSDataObject*> result;
    if (m_project == nullptr)
    {
        return result;
    }
    for (const QString& id : ids)
    {
        IDOSDataObject* data = m_project->object(id);
        if (data != nullptr && !m_removingIds.contains(id))
        {
            result.append(data);
        }
    }
    return result;
}

QStringList IDOSRenderServer::metadataDependencies(IDOSRenderObject* object, IDOSDataObject* data) const
{
    IDOSRenderProviderMetadata* metadata = m_registry != nullptr && data != nullptr
        ? m_registry->metadata(data->typeId()) : nullptr;
    return metadata != nullptr ? metadata->dependencies(data, object->provider()->saveState()) : QStringList();
}

bool IDOSRenderServer::refreshProvider(IDOSRenderObject* object, IDOSDataObject* data, const QVariantMap& state)
{
    IDOSRenderProviderMetadata* metadata = m_registry != nullptr && data != nullptr
        ? m_registry->metadata(data->typeId()) : nullptr;
    if (metadata == nullptr || object == nullptr || object->provider() == nullptr)
    {
        return false;
    }
    IDOSRenderProvider* provider = object->provider();
    const QVariantMap previous = provider->saveState();
    if (state != previous && !provider->restoreState(state))
    {
        provider->restoreState(previous);
        return false;
    }
    const QStringList dependencies = metadata->dependencies(data, state);
    if (!metadata->update(provider, references(dependencies)))
    {
        return false;
    }
    return true;
}

void IDOSRenderServer::onRenderObjectRemoved(const QString& id)
{
    m_sources.remove(id);
    emit stateChanged();
}

QVariantMap IDOSRenderServer::saveState() const
{
    QVariantMap state;
    QVariantList views;
    for (IDOSRenderView* view : m_views)
    {
        QVariantMap entry;
        entry.insert(QStringLiteral("id"), view->property("viewId").toString());
        entry.insert(QStringLiteral("view"), view->saveState());
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
            IDOSDataObject* data = dataObject(view, object->id());
            if (data == nullptr)
            {
                continue;
            }
            QVariantMap item;
            item.insert(QStringLiteral("dataId"), data->objectId());
            item.insert(QStringLiteral("typeId"), data->typeId());
            item.insert(QStringLiteral("provider"), object->provider()->saveState());
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

bool IDOSRenderServer::restoreState(const QVariantMap& state)
{
    if (state.isEmpty())
    {
        return true;
    }
    if (m_project == nullptr || m_registry == nullptr ||
        state.value(QStringLiteral("views")).type() != QVariant::List)
    {
        return false;
    }
    QHash<IDOSRenderView*, QList<IDOSRenderObject*>> prepared;
    QHash<QString, QString> sources;
    QHash<IDOSRenderView*, QVariantMap> savedViews;
    bool valid = true;
    for (const QVariant& value : state.value(QStringLiteral("views")).toList())
    {
        const QVariantMap entry = value.toMap();
        IDOSRenderView* target = nullptr;
        for (IDOSRenderView* view : m_views)
        {
            if (view->property("viewId").toString() == entry.value(QStringLiteral("id")).toString())
            {
                target = view;
                break;
            }
        }
        if (target == nullptr || savedViews.contains(target) ||
            entry.value(QStringLiteral("objects")).type() != QVariant::List)
        {
            valid = false;
            break;
        }
        savedViews.insert(target, entry);
        QSet<QString> dataIds;
        for (const QVariant& item : entry.value(QStringLiteral("objects")).toList())
        {
            const QVariantMap saved = item.toMap();
            const QString dataId = saved.value(QStringLiteral("dataId")).toString();
            IDOSDataObject* data = m_project->object(dataId);
            IDOSRenderProviderMetadata* metadata = data != nullptr ? m_registry->metadata(data->typeId()) : nullptr;
            if (data == nullptr || metadata == nullptr || dataIds.contains(dataId) ||
                data->typeId() != saved.value(QStringLiteral("typeId")).toString() ||
                saved.value(QStringLiteral("provider")).type() != QVariant::Map)
            {
                valid = false;
                break;
            }
            dataIds.insert(dataId);
            const QVariantMap providerState = saved.value(QStringLiteral("provider")).toMap();
            IDOSRenderProvider* provider = metadata->create(data, references(metadata->dependencies(data, providerState)));
            if (provider == nullptr)
            {
                valid = false;
                break;
            }
            IDOSRenderObject* object = new IDOSRenderObject(provider);
            if (!refreshProvider(object, data, providerState))
            {
                delete object;
                valid = false;
                break;
            }
            object->setName(data->name());
            sources.insert(object->id(), dataId);
            prepared[target].append(object);
        }
        if (!valid)
        {
            break;
        }
    }
    if (!valid)
    {
        for (const QList<IDOSRenderObject*>& objects : prepared)
        {
            qDeleteAll(objects);
        }
        return false;
    }
    QHash<IDOSRenderView*, QVariantMap> previous;
    QHash<IDOSRenderView*, IDOSRenderView*> previousLinks;
    for (IDOSRenderView* view : m_views)
    {
        previous.insert(view, view->saveState());
        for (IDOSRenderView* source : m_views)
        {
            if (view->isCameraLinkedTo(source))
            {
                previousLinks.insert(view, source);
                break;
            }
        }
    }
    for (IDOSRenderView* view : m_views)
    {
        if (savedViews.contains(view) &&
            !view->restoreState(savedViews.value(view).value(QStringLiteral("view")).toMap()))
        {
            for (IDOSRenderView* restored : m_views)
            {
                restored->restoreState(previous.value(restored));
            }
            for (IDOSRenderView* restored : previousLinks.keys())
            {
                restored->linkCameraTo(previousLinks.value(restored));
            }
            for (const QList<IDOSRenderObject*>& objects : prepared)
            {
                qDeleteAll(objects);
            }
            return false;
        }
    }
    for (IDOSRenderView* view : m_views)
    {
        view->unlinkCamera();
        view->scene()->beginUpdate();
        view->scene()->clear();
        m_removedStates.remove(view);
        QList<QString> selected;
        const QStringList selection = savedViews.value(view).value(QStringLiteral("selection")).toStringList();
        for (IDOSRenderObject* object : prepared.value(view))
        {
            const QString dataId = sources.value(object->id());
            m_sources.insert(object->id(), dataId);
            view->scene()->add(object);
            if (selection.contains(dataId))
            {
                selected.append(object->id());
            }
        }
        view->setSelection(selected);
        view->scene()->endUpdate();
    }
    for (IDOSRenderView* view : m_views)
    {
        const QString linkedId = savedViews.value(view).value(QStringLiteral("linkedView")).toString();
        if (!linkedId.isEmpty())
        {
            for (IDOSRenderView* source : m_views)
            {
                if (source != view && source->property("viewId").toString() == linkedId && !source->isCameraLinked())
                {
                    view->linkCameraTo(source);
                    break;
                }
            }
        }
        if (view->property("viewId").toString() == state.value(QStringLiteral("currentView")).toString())
        {
            setCurrentView(view);
        }
    }
    emit stateChanged();
    return true;
}
