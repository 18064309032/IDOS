#include <cmath>

#include <QUuid>

#include "idosrenderprovider.h"

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
    if (m_provider != nullptr)
    {
        m_provider->update();
    }
}

bool IDOSRenderObject::isVisible() const
{
    return m_provider != nullptr ? m_provider->saveState().value(QStringLiteral("visible"), false).toBool() : false;
}

void IDOSRenderObject::setVisible(bool visible)
{
    if (isVisible() == visible)
    {
        return;
    }
    QVariantMap state = m_provider != nullptr ? m_provider->saveState() : QVariantMap();
    state.insert(QStringLiteral("visible"), visible);
    if (m_provider != nullptr)
    {
        m_provider->restoreState(state);
    }
}

bool IDOSRenderObject::isSelected() const
{
    return m_provider != nullptr ? m_provider->saveState().value(QStringLiteral("highlighted"), false).toBool() : false;
}

void IDOSRenderObject::setSelected(bool highlighted)
{
    QVariantMap state = m_provider != nullptr ? m_provider->saveState() : QVariantMap();
    state.insert(QStringLiteral("highlighted"), highlighted);
    if (m_provider != nullptr)
    {
        m_provider->restoreState(state);
    }
}

double IDOSRenderObject::opacity() const
{
    return m_provider != nullptr ? m_provider->saveState().value(QStringLiteral("opacity"), 1.0).toDouble() : 1.0;
}

void IDOSRenderObject::setOpacity(double opacity)
{
    if (!std::isfinite(opacity))
    {
        return;
    }
    opacity = qBound(0.0, opacity, 1.0);
    QVariantMap state = m_provider != nullptr ? m_provider->saveState() : QVariantMap();
    state.insert(QStringLiteral("opacity"), opacity);
    if (m_provider != nullptr)
    {
        m_provider->restoreState(state);
    }
}
