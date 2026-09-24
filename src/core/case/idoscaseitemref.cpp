#include "case/idoscaseitemref.h"

IDOSCaseItemRef::IDOSCaseItemRef() = default;

IDOSCaseItemRef::IDOSCaseItemRef(const QString& role, const QString& objectId)
    : m_role(role)
    , m_objectId(objectId)
{
}

QString IDOSCaseItemRef::refId() const
{
    return m_refId;
}

{
    m_refId = refId;
}

QString IDOSCaseItemRef::role() const
{
    return m_role;
}

void IDOSCaseItemRef::setRole(const QString& role)
{
    m_role = role;
}

QString IDOSCaseItemRef::objectId() const
{
    return m_objectId;
}

void IDOSCaseItemRef::setObjectId(const QString& objectId)
{
    m_objectId = objectId;
}

QString IDOSCaseItemRef::partKey() const
{
    return m_partKey;
}

void IDOSCaseItemRef::setPartKey(const QString& partKey)
{
    m_partKey = partKey;
}

QString IDOSCaseItemRef::itemKey() const
{
    return m_itemKey;
}

void IDOSCaseItemRef::setItemKey(const QString& itemKey)
{
    m_itemKey = itemKey;
}
