#include "case/idoscaseobject.h"

IDOSCaseObject::IDOSCaseObject(QObject* parent)
    : IDOSDataObject(parent)
{
}

IDOSCaseObject::~IDOSCaseObject() = default;

QList<IDOSCaseItemRef> IDOSCaseObject::itemRefs() const
{
    return m_itemRefs;
}

void IDOSCaseObject::setItemRefs(const QList<IDOSCaseItemRef>& refs)
{
    m_itemRefs = refs;
    emit dataChanged();
}

void IDOSCaseObject::addItemRef(const IDOSCaseItemRef& ref)
{
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
