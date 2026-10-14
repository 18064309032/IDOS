#include <QObject>

#include "idoswellcontrol.h"

IDOSWellControl::IDOSWellControl()
    :
    m_wellIndex(0.0)
    , m_skinFactor(0.0)
    , m_dFactor(0.0)
    , m_active(false)
{
}

double IDOSWellControl::wellIndex() const
{
    return m_wellIndex;
}
void IDOSWellControl::setWellIndex(double wi)
{
    m_wellIndex = wi;
}

double IDOSWellControl::skinFactor() const
{
    return m_skinFactor;
}
void IDOSWellControl::setSkinFactor(double skin)
{
    m_skinFactor = skin;
}

double IDOSWellControl::dFactor() const
{
    return m_dFactor;
}
void IDOSWellControl::setDFactor(double d)
{
    m_dFactor = d;
}

bool IDOSWellControl::isActive() const
{
    return m_active;
}
void IDOSWellControl::setActive(bool active)
{
    m_active = active;
}

QVariantMap IDOSWellControl::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("wellIndex"), QVariant::fromValue(m_wellIndex));
    data.insert(QStringLiteral("skinFactor"), QVariant::fromValue(m_skinFactor));
    data.insert(QStringLiteral("dFactor"), QVariant::fromValue(m_dFactor));
    data.insert(QStringLiteral("active"), QVariant::fromValue(m_active));
    return data;
}

bool IDOSWellControl::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("wellIndex")) || data.value(QStringLiteral("wellIndex")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("wellIndex"));
        return false;
    }
    if (!data.contains(QStringLiteral("skinFactor")) || data.value(QStringLiteral("skinFactor")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("skinFactor"));
        return false;
    }
    if (!data.contains(QStringLiteral("dFactor")) || data.value(QStringLiteral("dFactor")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("dFactor"));
        return false;
    }
    if (!data.contains(QStringLiteral("active")) || data.value(QStringLiteral("active")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("active"));
        return false;
    }
    m_wellIndex = data.value(QStringLiteral("wellIndex")).value<double>();
    m_skinFactor = data.value(QStringLiteral("skinFactor")).value<double>();
    m_dFactor = data.value(QStringLiteral("dFactor")).value<double>();
    m_active = data.value(QStringLiteral("active")).value<bool>();
    return true;
}
