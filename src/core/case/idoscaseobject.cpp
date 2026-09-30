#include "idoscaseobject.h"

#include <QSet>

IDOSCaseObject::IDOSCaseObject(QObject* parent)
    : IDOSDataObject(parent)
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
