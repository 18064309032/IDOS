#include <QObject>

#include "idoscaseitemref.h"

IDOSCaseItemRef::IDOSCaseItemRef()
{
}

IDOSCaseItemRef::IDOSCaseItemRef(const QString& role, const QString& objectId)
    :
    m_role(role)
    , m_objectId(objectId)
{
}

QString IDOSCaseItemRef::refId() const
{
    return m_refId;
}

void IDOSCaseItemRef::setRefId(const QString& refId)
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

QVariantMap IDOSCaseItemRef::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("refId"), QVariant::fromValue(m_refId));
    data.insert(QStringLiteral("role"), QVariant::fromValue(m_role));
    data.insert(QStringLiteral("objectId"), QVariant::fromValue(m_objectId));
    data.insert(QStringLiteral("partKey"), QVariant::fromValue(m_partKey));
    data.insert(QStringLiteral("itemKey"), QVariant::fromValue(m_itemKey));
    return data;
}

bool IDOSCaseItemRef::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("refId")) || data.value(QStringLiteral("refId")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("refId"));
        return false;
    }
    if (!data.contains(QStringLiteral("role")) || data.value(QStringLiteral("role")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("role"));
        return false;
    }
    if (!data.contains(QStringLiteral("objectId")) || data.value(QStringLiteral("objectId")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("objectId"));
        return false;
    }
    if (!data.contains(QStringLiteral("partKey")) || data.value(QStringLiteral("partKey")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("partKey"));
        return false;
    }
    if (!data.contains(QStringLiteral("itemKey")) || data.value(QStringLiteral("itemKey")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("itemKey"));
        return false;
    }
    m_refId = data.value(QStringLiteral("refId")).value<QString>();
    m_role = data.value(QStringLiteral("role")).value<QString>();
    m_objectId = data.value(QStringLiteral("objectId")).value<QString>();
    m_partKey = data.value(QStringLiteral("partKey")).value<QString>();
    m_itemKey = data.value(QStringLiteral("itemKey")).value<QString>();
    return true;
}
