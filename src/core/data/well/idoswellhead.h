#ifndef IDOS_WELL_HEAD_H
#define IDOS_WELL_HEAD_H

#include "idos_core.h"

/**
 * @brief 井头地面数据。
 *
 * 对应 Eclipse WELLHEAD 关键字。
 * 描述井头在地面的位置和实时监测参数。
 */
class CORE_EXPORT IDOSWellHead
{
public:
    IDOSWellHead();

    /** 地面 X 坐标（米或经纬度，取决于坐标系）。 */
    double surfaceX() const;
    void setSurfaceX(double x);

    /** 地面 Y 坐标。 */
    double surfaceY() const;
    void setSurfaceY(double y);

    /** 地面海拔高度（米，正为海平面以上）。 */
    double surfaceElevation() const;
    void setSurfaceElevation(double elevation);

    /** 水深（米，海上井）。 */
    double waterDepth() const;
    void setWaterDepth(double depth);

    /** 井口压力（WHP，bar）。 */
    double wellheadPressure() const;
    void setWellheadPressure(double pressure);

    /** 井口温度（WHT，°C）。 */
    double wellheadTemperature() const;
    void setWellheadTemperature(double temperature);

private:
    double m_surfaceX;
    double m_surfaceY;
    double m_surfaceElevation;
    double m_waterDepth;
    double m_wellheadPressure;
    double m_wellheadTemperature;
};

#endif // IDOS_WELL_HEAD_H
