#include <QtAlgorithms>

#include "idosrenderobject.h"

#include "idosrenderscene.h"

IDOSRenderScene::IDOSRenderScene()
{
}

IDOSRenderScene::~IDOSRenderScene()
{
    clear();
}

void IDOSRenderScene::addObject(IDOSRenderObject* object)
{
    if (object == nullptr)
    {
        return;
    }
    if (m_objectById.contains(object->id()))
    {
        removeObject(object->id());
    }
    m_objects.append(object);
    m_objectById.insert(object->id(), object);
}

void IDOSRenderScene::removeObject(const QString& objectId)
{
    IDOSRenderObject* renderObject = m_objectById.value(objectId, nullptr);
    if (renderObject == nullptr)
    {
        return;
    }
    m_objectById.remove(objectId);
    m_objects.removeAll(renderObject);
    delete renderObject;
}

IDOSRenderObject* IDOSRenderScene::object(const QString& objectId) const
{
    return m_objectById.value(objectId, nullptr);
}

void IDOSRenderScene::setObject(IDOSRenderObject* object)
{
    clear();
    addObject(object);
}

void IDOSRenderScene::clear()
{
    qDeleteAll(m_objects);
    m_objects.clear();
    m_objectById.clear();
}

void IDOSRenderScene::setObjectVisible(const QString& objectId, bool visible)
{
    IDOSRenderObject* renderObject = object(objectId);
    if (renderObject == nullptr)
    {
        return;
    }
    renderObject->setVisible(visible);
}

const QList<IDOSRenderObject*>& IDOSRenderScene::objects() const
{
    return m_objects;
}
