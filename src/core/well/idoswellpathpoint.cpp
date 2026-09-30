#include "idoswellpathpoint.h"

IDOSWellPathPoint::IDOSWellPathPoint()
    : m_md(0.0)
    , m_x(0.0)
    , m_y(0.0)
    , m_z(0.0)
    , m_tvd(0.0)
    , m_dx(0.0)
    , m_dy(0.0)
    , m_azimuth(0.0)
    , m_inclination(0.0)
{
}

IDOSWellPathPoint::IDOSWellPathPoint(double md, double x, double y, double z, double tvd)
    : m_md(md)
    , m_x(x)
    , m_y(y)
    , m_z(z)
    , m_tvd(tvd)
    , m_dx(0.0)
    , m_dy(0.0)
    , m_azimuth(0.0)
    , m_inclination(0.0)
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
    : m_md(md)
    , m_x(x)
    , m_y(y)
    , m_z(z)
    , m_tvd(tvd)
    , m_dx(dx)
    , m_dy(dy)
    , m_azimuth(azimuth)
    , m_inclination(inclination)
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
