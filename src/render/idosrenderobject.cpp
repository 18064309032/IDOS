#include "idosrenderprovider.h"
#include "idosrenderobject.h"

IDOSRenderObject::IDOSRenderObject(IDOSRenderProvider* provider)
    :
    m_provider(provider)
{
}

IDOSRenderObject::~IDOSRenderObject()
{
    delete m_provider;
}

void IDOSRenderObject::setRenderProvider(IDOSRenderProvider* provider)
{
    if (m_provider == provider)
    {
        return;
    }
    delete m_provider;
    m_provider = provider;
}

IDOSRenderProvider* IDOSRenderObject::renderProvider() const
{
    return m_provider;
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
