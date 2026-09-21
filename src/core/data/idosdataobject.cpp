#include "idosdataobject.h"

IDOSDataObject::IDOSDataObject(QObject* parent)
    : IDOSObject(parent)
    , m_visible(true)
{
}

IDOSDataObject::~IDOSDataObject() = default;

bool IDOSDataObject::isVisible() const
{
    return m_visible;
}

void IDOSDataObject::setVisible(bool visible)
{
    if (m_visible == visible)
    {
        return;
    }
    m_visible = visible;
    emit visibilityChanged(m_visible);
}
