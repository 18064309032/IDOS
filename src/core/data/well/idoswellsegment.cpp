#include <QObject>

#include "idoswellsegment.h"

IDOSWellSegment::IDOSWellSegment()
    :
    m_depth(0.0)
    , m_inclination(0.0)
    , m_azimuth(0.0)
    , m_tvd(0.0)
    , m_dx(0.0)
    , m_dy(0.0)
{
}

IDOSWellSegment::IDOSWellSegment(double depth, double inclination, double azimuth, double tvd, double dx, double dy)
    :
    m_depth(depth)
    , m_inclination(inclination)
    , m_azimuth(azimuth)
    , m_tvd(tvd)
    , m_dx(dx)
    , m_dy(dy)
{
}

double IDOSWellSegment::depth() const
{
    return m_depth;
}
void IDOSWellSegment::setDepth(double depth)
{
    m_depth = depth;
}

double IDOSWellSegment::inclination() const
{
    return m_inclination;
}
void IDOSWellSegment::setInclination(double inclination)
{
    m_inclination = inclination;
}

double IDOSWellSegment::azimuth() const
{
    return m_azimuth;
}
void IDOSWellSegment::setAzimuth(double azimuth)
{
    m_azimuth = azimuth;
}

double IDOSWellSegment::tvd() const
{
    return m_tvd;
}
void IDOSWellSegment::setTvd(double tvd)
{
    m_tvd = tvd;
}

double IDOSWellSegment::dx() const
{
    return m_dx;
}
void IDOSWellSegment::setDx(double dx)
{
    m_dx = dx;
}

double IDOSWellSegment::dy() const
{
    return m_dy;
}
void IDOSWellSegment::setDy(double dy)
{
    m_dy = dy;
}

QVariantMap IDOSWellSegment::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("depth"), QVariant::fromValue(m_depth));
    data.insert(QStringLiteral("inclination"), QVariant::fromValue(m_inclination));
    data.insert(QStringLiteral("azimuth"), QVariant::fromValue(m_azimuth));
    data.insert(QStringLiteral("tvd"), QVariant::fromValue(m_tvd));
    data.insert(QStringLiteral("dx"), QVariant::fromValue(m_dx));
    data.insert(QStringLiteral("dy"), QVariant::fromValue(m_dy));
    return data;
}

bool IDOSWellSegment::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("depth")) || data.value(QStringLiteral("depth")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("depth"));
        return false;
    }
    if (!data.contains(QStringLiteral("inclination")) || data.value(QStringLiteral("inclination")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("inclination"));
        return false;
    }
    if (!data.contains(QStringLiteral("azimuth")) || data.value(QStringLiteral("azimuth")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("azimuth"));
        return false;
    }
    if (!data.contains(QStringLiteral("tvd")) || data.value(QStringLiteral("tvd")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("tvd"));
        return false;
    }
    if (!data.contains(QStringLiteral("dx")) || data.value(QStringLiteral("dx")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("dx"));
        return false;
    }
    if (!data.contains(QStringLiteral("dy")) || data.value(QStringLiteral("dy")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("dy"));
        return false;
    }
    m_depth = data.value(QStringLiteral("depth")).value<double>();
    m_inclination = data.value(QStringLiteral("inclination")).value<double>();
    m_azimuth = data.value(QStringLiteral("azimuth")).value<double>();
    m_tvd = data.value(QStringLiteral("tvd")).value<double>();
    m_dx = data.value(QStringLiteral("dx")).value<double>();
    m_dy = data.value(QStringLiteral("dy")).value<double>();
    return true;
}
