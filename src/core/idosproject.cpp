#include <QUndoStack>

#include "command/idoscommandmanager.h"

#include "idosproject.h"

IDOSProject::IDOSProject(QObject* parent)
    : QObject(parent)
    , m_undoStack(new QUndoStack(this))
    , m_commandManager(new IDOSCommandManager(this, this))
    , m_updateDepth(0)
{
}

IDOSProject::~IDOSProject()
{
}

const IDOSProjectMetadata& IDOSProject::metadata() const
{
    return m_metadata;
}

QUndoStack* IDOSProject::undoStack() const
{
    return m_undoStack;
}

IDOSCommandManager* IDOSProject::commandManager() const
{
    return m_commandManager;
}

bool IDOSProject::setMetadata(const IDOSProjectMetadata& metadata)
{
    if (metadata.name().trimmed().isEmpty())
    {
        return false;
    }
    if (metadata.coordinateType() == IDOSProjectMetadata::CoordinateType::Custom && metadata.coordinateReference().trimmed().isEmpty())
    {
        return false;
    }
    m_metadata = metadata;
    m_metadata.setName(metadata.name().trimmed());
    m_metadata.setFieldBlock(metadata.fieldBlock().trimmed());
    m_metadata.setDescription(metadata.description().trimmed());
    m_metadata.setCoordinateReference(metadata.coordinateType() == IDOSProjectMetadata::CoordinateType::Custom
                                      ? metadata.coordinateReference().trimmed()
                                      : QString());
    m_metadata.setVerticalDatum(metadata.verticalDatum().trimmed());
    emit metadataChanged();
    return true;
}

// ===== 增删查 =====

void IDOSProject::addObject(IDOSDataObject* object)
{
    if (object == nullptr)
    {
        return;
    }

    const QString id = object->objectId();

    // 已存在同 id，先移除旧的
    IDOSDataObject* existing = m_objects.value(id, nullptr);
    if (existing != nullptr && existing != object)
    {
        disconnectObject(existing);
        m_objects.remove(id);
        existing->deleteLater();
    }

    object->setParent(this);
    m_objects.insert(id, object);
    connectObject(object);

    notifyAdded(id);
}

IDOSDataObject* IDOSProject::takeObject(const QString& objectId)
{
    IDOSDataObject* object = m_objects.value(objectId, nullptr);
    if (object == nullptr)
    {
        return nullptr;
    }

    disconnectObject(object);
    m_objects.remove(objectId);
    object->setParent(nullptr);

    notifyRemoved(objectId);
    return object;
}

bool IDOSProject::removeObject(const QString& objectId)
{
    IDOSDataObject* object = takeObject(objectId);
    if (object == nullptr)
    {
        return false;
    }

    object->deleteLater();
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
        if (object->name() == name)
        {
            return object;
        }
    }
    return nullptr;
}

QList<IDOSDataObject*> IDOSProject::objects() const
{
    return m_objects.values();
}

// ===== 批量事务 =====

void IDOSProject::beginUpdate()
{
    ++m_updateDepth;
}

void IDOSProject::endUpdate()
{
    if (m_updateDepth <= 0)
    {
        return;
    }
    --m_updateDepth;
    if (m_updateDepth > 0)
    {
        return;
    }

    // 最外层事务闭合：按 removed → added → changed 顺序各发一次批量信号。
    // 集合在 notify* 中已做归并（同 id 先删后加最终算 added 等），列表天然唯一。
    if (!m_pendingRemoved.isEmpty())
    {
        emit objectsRemoved(m_pendingRemoved.values());
        m_pendingRemoved.clear();
    }
    if (!m_pendingAdded.isEmpty())
    {
        emit objectsAdded(m_pendingAdded.values());
        m_pendingAdded.clear();
    }
    if (!m_pendingChanged.isEmpty())
    {
        emit objectsChanged(m_pendingChanged.values());
        m_pendingChanged.clear();
    }
}

void IDOSProject::notifyAdded(const QString& objectId)
{
    if (m_updateDepth > 0)
    {
        // 删后又加（含同 id 替换）：最终状态为 added，清掉其他两类记录
        m_pendingRemoved.remove(objectId);
        m_pendingChanged.remove(objectId);
        m_pendingAdded.insert(objectId);
        return;
    }
    emit objectAdded(objectId);
}

void IDOSProject::notifyRemoved(const QString& objectId)
{
    if (m_updateDepth > 0)
    {
        // 加了又删 / 改了又删：最终状态为 removed
        m_pendingAdded.remove(objectId);
        m_pendingChanged.remove(objectId);
        m_pendingRemoved.insert(objectId);
        return;
    }
    emit objectRemoved(objectId);
}

void IDOSProject::notifyChanged(const QString& objectId)
{
    if (m_updateDepth > 0)
    {
        // 新对象在事务内的变化不重复记录（最终 added 已包含它）
        if (!m_pendingAdded.contains(objectId))
        {
            m_pendingChanged.insert(objectId);
        }
        return;
    }
    emit objectChanged(objectId);
}

IDOSProjectUpdateGuard::IDOSProjectUpdateGuard(IDOSProject* project)
    : m_project(project)
{
    if (m_project != nullptr)
    {
        m_project->beginUpdate();
    }
}

IDOSProjectUpdateGuard::~IDOSProjectUpdateGuard()
{
    if (m_project != nullptr)
    {
        m_project->endUpdate();
    }
}

// ===== 内部：信号转发 =====

void IDOSProject::connectObject(IDOSDataObject* object)
{
    connect(object, &IDOSDataObject::visibilityChanged, this, &IDOSProject::onObjectVisibilityChanged);
    connect(object, &IDOSDataObject::nameChanged, this, &IDOSProject::onObjectNameChanged);
    connect(object, &IDOSDataObject::dataChanged, this, &IDOSProject::onObjectDataChanged);
}

void IDOSProject::disconnectObject(IDOSDataObject* object)
{
    object->disconnect(this);
}

void IDOSProject::onObjectDataChanged()
{
    IDOSDataObject* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        notifyChanged(obj->objectId());
    }
}

void IDOSProject::onObjectVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);
    IDOSDataObject* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        notifyChanged(obj->objectId());
    }
}

void IDOSProject::onObjectNameChanged(const QString& name)
{
    Q_UNUSED(name);
    IDOSDataObject* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        notifyChanged(obj->objectId());
    }
}
