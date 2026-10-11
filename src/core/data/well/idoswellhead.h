#ifndef IDOS_WELL_HEAD_H
#define IDOS_WELL_HEAD_H

#include "idos_core.h"
#include "idoswellspatialreference.h"

/**
 * @brief Wellhead surface data.
 *
 * Corresponds to the Eclipse WELLHEAD keyword.
 * Describes the wellhead surface position and real-time monitoring values.
 */
class CORE_EXPORT IDOSWellHead
{
  public:
    IDOSWellHead();

    double surfaceX() const;
    void setSurfaceX(double x);

    double surfaceY() const;
    void setSurfaceY(double y);

    bool hasSurfacePosition() const;

    double surfaceElevation() const;
    void setSurfaceElevation(double elevation);
    bool hasSurfaceElevation() const;

    IDOSWellSpatialReference spatialReference() const;
    void setSpatialReference(const IDOSWellSpatialReference& reference);

    double waterDepth() const;
    void setWaterDepth(double depth);

    double wellheadPressure() const;
    void setWellheadPressure(double pressure);

    double wellheadTemperature() const;
    void setWellheadTemperature(double temperature);

    double topDepth() const;
    void setTopDepth(double depth);

    double bottomDepth() const;
    void setBottomDepth(double depth);

    double kb() const;
    void setKb(double value);
    int symbol() const;
    void setSymbol(int value);

  private:
    double m_surfaceX;
    double m_surfaceY;
    bool m_hasSurfaceX;
    bool m_hasSurfaceY;
    double m_surfaceElevation;
    bool m_hasSurfaceElevation;
    IDOSWellSpatialReference m_spatialReference;
    double m_waterDepth;
    double m_wellheadPressure;
    double m_wellheadTemperature;
    double m_topDepth;
    double m_bottomDepth;
    double m_kb;
    int m_symbol;
};

#endif // IDOS_WELL_HEAD_H
