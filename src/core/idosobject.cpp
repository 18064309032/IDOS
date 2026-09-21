#include "idosobject.h"

#include <QUuid>

IDOSObject::IDOSObject(QObject* parent)
    : QObject(parent)
    , m_objectId(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

IDOSObject::~IDOSObject() = default;

QString IDOSObject::objectId() const
{
    return m_objectId;
}

QString IDOSObject::name() const
{
    return m_name;
}

void IDOSObject::setName(const QString& name)
{
    if (m_name == name)
    {
        return;
    }
    m_name = name;
    emit nameChanged(m_name);
}

void IDOSObject::setObjectId(const QString& id)
{
    if (m_objectId == id)
    {
        return;
    }
    m_objectId = id;
    emit objectIdChanged(m_objectId);
}
