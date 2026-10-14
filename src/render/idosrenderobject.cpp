#include <QUuid>

#include "providers/idosrenderprovider.h"

#include "idosrenderobject.h"

IDOSRenderObject::IDOSRenderObject(IDOSRenderProvider* provider)
    :
    m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_name()
    , m_provider(provider)
{
}

IDOSRenderObject::~IDOSRenderObject()
{
    delete m_provider;
}

QString IDOSRenderObject::id() const
{
    return m_id;
}

QString IDOSRenderObject::name() const
{
    return m_name;
}

void IDOSRenderObject::setName(const QString& name)
{
    m_name = name;
}

IDOSRenderProvider* IDOSRenderObject::provider() const
{
    return m_provider;
}

void IDOSRenderObject::update()
{
    provider()->update();
}

bool IDOSRenderObject::visible() const
{
    return provider()->visible();
}

void IDOSRenderObject::setVisible(bool visible)
{
    provider()->setVisible(visible);
}

bool IDOSRenderObject::highlighted() const
{
    return provider()->highlighted();
}

void IDOSRenderObject::setHighlighted(bool highlighted)
{
    provider()->setHighlighted(highlighted);
}

double IDOSRenderObject::opacity() const
{
    return provider()->opacity();
}

void IDOSRenderObject::setOpacity(double opacity)
{
    provider()->setOpacity(opacity);
}
