#ifndef IDOS_WELL_SPATIAL_REFERENCE_H
#define IDOS_WELL_SPATIAL_REFERENCE_H

#include "idos_core.h"

class CORE_EXPORT IDOSWellSpatialReference
{
  public:
    enum class VerticalDatum
    {
        Unknown,
        KellyBushing,
        MeanSeaLevel,
        TrueVerticalDepthSubsea
    };

    enum class DepthDirection
    {
        Unknown,
        PositiveDown,
        PositiveUp
    };

    IDOSWellSpatialReference();

    VerticalDatum verticalDatum() const;
    void setVerticalDatum(VerticalDatum datum);

    DepthDirection depthDirection() const;
    void setDepthDirection(DepthDirection direction);

    bool hasKnownVerticalReference() const;
    bool isCompatibleWith(const IDOSWellSpatialReference& other) const;

  private:
    VerticalDatum m_verticalDatum;
    DepthDirection m_depthDirection;
};

#endif // IDOS_WELL_SPATIAL_REFERENCE_H
