#include "idosrenderobject.h"

#include "idosrenderscene.h"

IDOSRenderScene::IDOSRenderScene(QObject* parent)
    :
    QObject(parent)
    , m_updateDepth(0)
    , m_pending(false)
{
}

IDOSRenderScene::~IDOSRenderScene()
{
    clear();
}

bool IDOSRenderScene::add(IDOSRenderObject* object)
{
    if (object == nullptr || contains(object->id()))
    {
        return false;
    }
    m_objects.insert(object->id(), object);
    emit objectAdded(object->id());
    notify();
    return true;
}

bool IDOSRenderScene::remove(const QString& id)
{
    IDOSRenderObject* value = object(id);
    if (value == nullptr)
    {
        return false;
    }
    emit objectAboutToBeRemoved(id);
    m_objects.remove(id);
    delete value;
    emit objectRemoved(id);
    notify();
    return true;
}

void IDOSRenderScene::clear()
{
    beginUpdate();
    const QList<QString> keys = ids();
    for (const QString& id : keys)
    {
        remove(id);
    }
    endUpdate();
}

IDOSRenderObject* IDOSRenderScene::object(const QString& id) const
{
    return m_objects.value(id, nullptr);
}

QList<IDOSRenderObject*> IDOSRenderScene::objects() const
{
    return m_objects.values();
}

QList<QString> IDOSRenderScene::ids() const
{
    return m_objects.keys();
}

bool IDOSRenderScene::contains(const QString& id) const
{
    return m_objects.contains(id);
}

void IDOSRenderScene::notifyChanged(const QString& id)
{
    if (contains(id))
    {
        emit objectChanged(id);
        notify();
    }
}

void IDOSRenderScene::beginUpdate()
{
    ++m_updateDepth;
}

void IDOSRenderScene::endUpdate()
{
    if (m_updateDepth == 0)
    {
        return;
    }
    --m_updateDepth;
    if (m_updateDepth == 0 && m_pending)
    {
        m_pending = false;
        emit changed();
    }
}

void IDOSRenderScene::notify()
{
    if (m_updateDepth > 0)
    {
        m_pending = true;
    }
    else
    {
        emit changed();
    }
}
