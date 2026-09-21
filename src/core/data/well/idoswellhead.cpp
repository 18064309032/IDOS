#include "idoswellhead.h"

IDOSWellHead::IDOSWellHead()
    : m_surfaceX(0.0)
    , m_surfaceY(0.0)
    , m_surfaceElevation(0.0)
    , m_waterDepth(0.0)
    , m_wellheadPressure(0.0)
    , m_wellheadTemperature(0.0)
{
}

double IDOSWellHead::surfaceX() const { return m_surfaceX; }
void IDOSWellHead::setSurfaceX(double x) { m_surfaceX = x; }

double IDOSWellHead::surfaceY() const { return m_surfaceY; }
void IDOSWellHead::setSurfaceY(double y) { m_surfaceY = y; }

double IDOSWellHead::surfaceElevation() const { return m_surfaceElevation; }
void IDOSWellHead::setSurfaceElevation(double elevation) { m_surfaceElevation = elevation; }

double IDOSWellHead::waterDepth() const { return m_waterDepth; }
void IDOSWellHead::setWaterDepth(double depth) { m_waterDepth = depth; }

double IDOSWellHead::wellheadPressure() const { return m_wellheadPressure; }
void IDOSWellHead::setWellheadPressure(double pressure) { m_wellheadPressure = pressure; }

double IDOSWellHead::wellheadTemperature() const { return m_wellheadTemperature; }
void IDOSWellHead::setWellheadTemperature(double temperature) { m_wellheadTemperature = temperature; }
