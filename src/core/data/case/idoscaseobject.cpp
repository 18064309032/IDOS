#include <QObject>
#include <QSet>

#include "idosproject.h"

#include "idoscaseobject.h"

IDOSCaseObject::IDOSCaseObject(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

IDOSCaseObject::~IDOSCaseObject()
{
}

QList<IDOSCaseItemRef> IDOSCaseObject::itemRefs() const
{
    return m_itemRefs;
}

void IDOSCaseObject::setItemRefs(const QList<IDOSCaseItemRef>& refs)
{
    QList<IDOSCaseItemRef> uniqueRefs;
    QSet<QString> seen;
    for (const IDOSCaseItemRef& ref : refs)
    {
        const QString signature = ref.role() + QLatin1Char(':') + ref.objectId() + QLatin1Char(':') +
                                 ref.partKey() + QLatin1Char(':') + ref.itemKey() + QLatin1Char(':') +
                                 ref.refId();
        if (!seen.contains(signature))
        {
            seen.insert(signature);
            uniqueRefs.append(ref);
        }
    }
    m_itemRefs = uniqueRefs;
    emit dataChanged();
}

void IDOSCaseObject::addItemRef(const IDOSCaseItemRef& ref)
{
    for (const IDOSCaseItemRef& existing : m_itemRefs)
    {
        const QString signature = existing.role() + QLatin1Char(':') + existing.objectId() + QLatin1Char(':') +
                                 existing.partKey() + QLatin1Char(':') + existing.itemKey() + QLatin1Char(':') +
                                 existing.refId();
        const QString incoming = ref.role() + QLatin1Char(':') + ref.objectId() + QLatin1Char(':') +
                                 ref.partKey() + QLatin1Char(':') + ref.itemKey() + QLatin1Char(':') +
                                 ref.refId();
        if (signature == incoming)
        {
            return;
        }
    }
    m_itemRefs.append(ref);
    emit dataChanged();
}

void IDOSCaseObject::clearItemRefs()
{
    if (m_itemRefs.isEmpty())
    {
        return;
    }

    m_itemRefs.clear();
    emit dataChanged();
}

QVariantMap IDOSCaseObject::projectData() const
{
    QVariantMap data;
    QVariantList itemRefs;
    for (const IDOSCaseItemRef& value : m_itemRefs)
    {
        itemRefs.append(value.projectData());
    }
    data.insert(QStringLiteral("itemRefs"), itemRefs);
    return data;
}

bool IDOSCaseObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("itemRefs")) || data.value(QStringLiteral("itemRefs")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("itemRefs"));
        return false;
    }
    m_itemRefs.clear();
    for (const QVariant& entry : data.value(QStringLiteral("itemRefs")).toList())
    {
        IDOSCaseItemRef value;
        if (!value.restoreProjectData(entry.toMap(), error))
        {
            return false;
        }
        m_itemRefs.append(value);
    }
    return true;
}
bool IDOSCaseObject::validateProjectReferences(const IDOSProject* project, QString& error) const
{
    QSet<QString> identities;
    for (const IDOSCaseItemRef& ref : m_itemRefs)
    {
        if (ref.objectId() == objectId() || project->object(ref.objectId()) == nullptr ||
            (!ref.refId().isEmpty() && identities.contains(ref.refId())))
        {
            error = tr("A case contains a missing object, a self reference or duplicate reference identities.");
            return false;
        }
        identities.insert(ref.refId());
    }
    return true;
}
