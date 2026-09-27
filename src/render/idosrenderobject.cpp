#include "idosrenderobject.h"

IDOSRenderObject::IDOSRenderObject()
    : m_visible(true)
{
}

IDOSRenderObject::~IDOSRenderObject()
{
}

QString IDOSRenderObject::id() const
{
    return m_id;
}

void IDOSRenderObject::setId(const QString& id)
{
    m_id = id;
}

QString IDOSRenderObject::name() const
{
    return m_name;
}

void IDOSRenderObject::setName(const QString& name)
{
    m_name = name;
}

bool IDOSRenderObject::visible() const
{
    return m_visible;
}

void IDOSRenderObject::setVisible(bool visible)
{
    m_visible = visible;
}
