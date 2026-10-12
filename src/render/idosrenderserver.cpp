#include "idosdataobject.h"
#include "idosproject.h"
#include "idosrendermetadata.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderregistry.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"

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

IDOSRenderRegistry* IDOSRenderServer::registry() const
{
    return m_registry.data();
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
    for (IDOSRenderView* view : m_views)
    {
        view->setProject(project);
    }
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
    view->setRegistry(m_registry);
    view->setProject(m_project);
    connect(view, &QObject::destroyed, this, &IDOSRenderServer::onViewDestroyed);
    populate(view);
}

void IDOSRenderServer::removeView(IDOSRenderView* view)
{
    if (!m_views.removeOne(view))
    {
        return;
    }
    disconnect(view, nullptr, this, nullptr);
    view->scene()->clear();
    view->setProject(nullptr);
    view->setRegistry(nullptr);
}

QList<IDOSRenderView*> IDOSRenderServer::views() const
{
    return m_views;
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

bool IDOSRenderServer::update(IDOSRenderView* view, IDOSDataObject* data, bool dataChanged)
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
        IDOSRenderProvider* provider = metadata->create(data);
        if (provider == nullptr)
        {
            return false;
        }
        IDOSRenderObject* object = new IDOSRenderObject(provider);
        object->setName(data->name());
        object->setVisible(false);
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
    for (int index = m_views.size() - 1; index >= 0; --index)
    {
        if (m_views.at(index) == object)
        {
            m_views.removeAt(index);
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
