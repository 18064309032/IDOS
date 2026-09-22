#include "idoswellpathpoint.h"

IDOSWellPathPoint::IDOSWellPathPoint()
    : m_md(0.0)
    , m_x(0.0)
    , m_y(0.0)
    , m_z(0.0)
    , m_tvd(0.0)
{
}

IDOSWellPathPoint::IDOSWellPathPoint(double md, double x, double y, double z, double tvd)
    : m_md(md)
    , m_x(x)
    , m_y(y)
    , m_z(z)
    , m_tvd(tvd)
{
}

double IDOSWellPathPoint::md() const { return m_md; }
double IDOSWellPathPoint::x() const { return m_x; }
double IDOSWellPathPoint::y() const { return m_y; }
double IDOSWellPathPoint::z() const { return m_z; }
double IDOSWellPathPoint::tvd() const { return m_tvd; }
