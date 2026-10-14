#include <QObject>

#include "idoswellpathpoint.h"

IDOSWellPathPoint::IDOSWellPathPoint()
    :
    m_md(0.0)
    , m_x(0.0)
    , m_y(0.0)
    , m_z(0.0)
    , m_tvd(0.0)
    , m_dx(0.0)
    , m_dy(0.0)
    , m_azimuth(0.0)
    , m_inclination(0.0)
    , m_attributes()
{
}

IDOSWellPathPoint::IDOSWellPathPoint(double md, double x, double y, double z, double tvd)
    :
    m_md(md)
    , m_x(x)
    , m_y(y)
    , m_z(z)
    , m_tvd(tvd)
    , m_dx(0.0)
    , m_dy(0.0)
    , m_azimuth(0.0)
    , m_inclination(0.0)
    , m_attributes()
{
}

double IDOSWellPathPoint::md() const
{
    return m_md;
}
double IDOSWellPathPoint::x() const
{
    return m_x;
}
double IDOSWellPathPoint::y() const
{
    return m_y;
}
double IDOSWellPathPoint::z() const
{
    return m_z;
}
double IDOSWellPathPoint::tvd() const
{
    return m_tvd;
}

IDOSWellPathPoint::IDOSWellPathPoint(double md, double x, double y, double z, double tvd, double dx, double dy,
                                     double azimuth, double inclination)
    :
    m_md(md)
    , m_x(x)
    , m_y(y)
    , m_z(z)
    , m_tvd(tvd)
    , m_dx(dx)
    , m_dy(dy)
    , m_azimuth(azimuth)
    , m_inclination(inclination)
    , m_attributes()
{
}

double IDOSWellPathPoint::dx() const
{
    return m_dx;
}

double IDOSWellPathPoint::dy() const
{
    return m_dy;
}

double IDOSWellPathPoint::azimuth() const
{
    return m_azimuth;
}

double IDOSWellPathPoint::inclination() const
{
    return m_inclination;
}

QVariantMap IDOSWellPathPoint::attributes() const
{
    return m_attributes;
}

void IDOSWellPathPoint::setAttributes(const QVariantMap& attributes)
{
    m_attributes = attributes;
}

QVariantMap IDOSWellPathPoint::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("md"), QVariant::fromValue(m_md));
    data.insert(QStringLiteral("x"), QVariant::fromValue(m_x));
    data.insert(QStringLiteral("y"), QVariant::fromValue(m_y));
    data.insert(QStringLiteral("z"), QVariant::fromValue(m_z));
    data.insert(QStringLiteral("tvd"), QVariant::fromValue(m_tvd));
    data.insert(QStringLiteral("dx"), QVariant::fromValue(m_dx));
    data.insert(QStringLiteral("dy"), QVariant::fromValue(m_dy));
    data.insert(QStringLiteral("azimuth"), QVariant::fromValue(m_azimuth));
    data.insert(QStringLiteral("inclination"), QVariant::fromValue(m_inclination));
    data.insert(QStringLiteral("attributes"), QVariant::fromValue(m_attributes));
    return data;
}

bool IDOSWellPathPoint::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("md")) || data.value(QStringLiteral("md")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("md"));
        return false;
    }
    if (!data.contains(QStringLiteral("x")) || data.value(QStringLiteral("x")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("x"));
        return false;
    }
    if (!data.contains(QStringLiteral("y")) || data.value(QStringLiteral("y")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("y"));
        return false;
    }
    if (!data.contains(QStringLiteral("z")) || data.value(QStringLiteral("z")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("z"));
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
    if (!data.contains(QStringLiteral("azimuth")) || data.value(QStringLiteral("azimuth")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("azimuth"));
        return false;
    }
    if (!data.contains(QStringLiteral("inclination")) || data.value(QStringLiteral("inclination")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("inclination"));
        return false;
    }
    if (!data.contains(QStringLiteral("attributes")) || data.value(QStringLiteral("attributes")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("attributes"));
        return false;
    }
    m_md = data.value(QStringLiteral("md")).value<double>();
    m_x = data.value(QStringLiteral("x")).value<double>();
    m_y = data.value(QStringLiteral("y")).value<double>();
    m_z = data.value(QStringLiteral("z")).value<double>();
    m_tvd = data.value(QStringLiteral("tvd")).value<double>();
    m_dx = data.value(QStringLiteral("dx")).value<double>();
    m_dy = data.value(QStringLiteral("dy")).value<double>();
    m_azimuth = data.value(QStringLiteral("azimuth")).value<double>();
    m_inclination = data.value(QStringLiteral("inclination")).value<double>();
    m_attributes = data.value(QStringLiteral("attributes")).value<QVariantMap>();
    return true;
}
