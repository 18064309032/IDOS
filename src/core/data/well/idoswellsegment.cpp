#include "idoswellsegment.h"

IDOSWellSegment::IDOSWellSegment()
    : m_depth(0.0)
    , m_inclination(0.0)
    , m_azimuth(0.0)
    , m_tvd(0.0)
    , m_dx(0.0)
    , m_dy(0.0)
{
}

IDOSWellSegment::IDOSWellSegment(double depth, double inclination, double azimuth,
                                 double tvd, double dx, double dy)
    : m_depth(depth)
    , m_inclination(inclination)
    , m_azimuth(azimuth)
    , m_tvd(tvd)
    , m_dx(dx)
    , m_dy(dy)
{
}

double IDOSWellSegment::depth() const { return m_depth; }
void IDOSWellSegment::setDepth(double depth) { m_depth = depth; }

double IDOSWellSegment::inclination() const { return m_inclination; }
void IDOSWellSegment::setInclination(double inclination) { m_inclination = inclination; }

double IDOSWellSegment::azimuth() const { return m_azimuth; }
void IDOSWellSegment::setAzimuth(double azimuth) { m_azimuth = azimuth; }

double IDOSWellSegment::tvd() const { return m_tvd; }
void IDOSWellSegment::setTvd(double tvd) { m_tvd = tvd; }

double IDOSWellSegment::dx() const { return m_dx; }
void IDOSWellSegment::setDx(double dx) { m_dx = dx; }

double IDOSWellSegment::dy() const { return m_dy; }
void IDOSWellSegment::setDy(double dy) { m_dy = dy; }
