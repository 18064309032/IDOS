#include "idosproject.h"

IDOSProject::IDOSProject(QObject* parent)
    : QObject(parent)
{
}

IDOSProject::~IDOSProject() = default;

// ===== 增删查 =====

void IDOSProject::addObject(IDOSDataObject* object)
{
    if (object == nullptr) return;

    const QString id = object->objectId();

    // 已存在同 id，先移除旧的
    auto* existing = m_objects.value(id, nullptr);
    if (existing != nullptr && existing != object)
    {
        disconnectObject(existing);
        m_objects.remove(id);
        existing->deleteLater();
    }

    object->setParent(this);
    m_objects.insert(id, object);
    connectObject(object);

    emit objectAdded(id);
}

bool IDOSProject::removeObject(const QString& objectId)
{
    IDOSDataObject* object = m_objects.value(objectId, nullptr);
    if (object == nullptr) return false;

    disconnectObject(object);
    m_objects.remove(objectId);
    object->deleteLater();

    emit objectRemoved(objectId);
    return true;
}

IDOSDataObject* IDOSProject::objectById(const QString& objectId) const
{
    return m_objects.value(objectId, nullptr);
}

IDOSDataObject* IDOSProject::objectByName(const QString& name) const
{
    for (IDOSDataObject* object : m_objects)
    {
        if (object->name() == name) return object;
    }
    return nullptr;
}

QList<IDOSDataObject*> IDOSProject::objects() const
{
    return m_objects.values();
}

// ===== 内部：信号转发 =====

void IDOSProject::connectObject(IDOSDataObject* object)
{
    connect(object, &IDOSDataObject::visibilityChanged,
            this, &IDOSProject::onObjectVisibilityChanged);
    connect(object, &IDOSDataObject::nameChanged,
            this, &IDOSProject::onObjectNameChanged);
    connect(object, &IDOSDataObject::dataChanged,
            this, &IDOSProject::onObjectDataChanged);
}

void IDOSProject::disconnectObject(IDOSDataObject* object)
{
    object->disconnect(this);
}

void IDOSProject::onObjectDataChanged()
{
    auto* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        emit objectChanged(obj->objectId());
    }
}

void IDOSProject::onObjectVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);
    auto* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        emit objectChanged(obj->objectId());
    }
}

void IDOSProject::onObjectNameChanged(const QString& name)
{
    Q_UNUSED(name);
    auto* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        emit objectChanged(obj->objectId());
    }
}
