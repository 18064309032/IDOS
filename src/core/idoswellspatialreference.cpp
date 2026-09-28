#include "idoswellspatialreference.h"

IDOSWellSpatialReference::IDOSWellSpatialReference()
    : m_verticalDatum(VerticalDatum::Unknown)
    , m_depthDirection(DepthDirection::Unknown)
{
}

IDOSWellSpatialReference::VerticalDatum IDOSWellSpatialReference::verticalDatum() const
{
    return m_verticalDatum;
}

void IDOSWellSpatialReference::setVerticalDatum(VerticalDatum datum)
{
    m_verticalDatum = datum;
}

IDOSWellSpatialReference::DepthDirection IDOSWellSpatialReference::depthDirection() const
{
    return m_depthDirection;
}

void IDOSWellSpatialReference::setDepthDirection(DepthDirection direction)
{
    m_depthDirection = direction;
}

bool IDOSWellSpatialReference::hasKnownVerticalReference() const
{
    return m_verticalDatum != VerticalDatum::Unknown && m_depthDirection != DepthDirection::Unknown;
}

bool IDOSWellSpatialReference::isCompatibleWith(const IDOSWellSpatialReference& other) const
{
    return hasKnownVerticalReference() && other.hasKnownVerticalReference()
           && m_verticalDatum == other.m_verticalDatum && m_depthDirection == other.m_depthDirection;
}
