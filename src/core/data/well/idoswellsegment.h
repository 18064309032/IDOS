#ifndef IDOS_WELL_SEGMENT_H
#define IDOS_WELL_SEGMENT_H

#include "idos_core.h"

/**
 * @brief 井轨迹段数据。
 *
 * 对应 Eclipse WELSEGS 关键字的一条轨迹点。
 * 多个 WellSegment 组成一口井的完整井身轨迹。
 */
class CORE_EXPORT IDOSWellSegment
{
public:
    IDOSWellSegment();
    IDOSWellSegment(double depth, double inclination, double azimuth,
                    double tvd, double dx, double dy);

    /** 测深（沿井身的深度，米）。 */
    double depth() const;
    void setDepth(double depth);

    /** 井斜角（从垂直方向偏离的角度，度）。 */
    double inclination() const;
    void setInclination(double inclination);

    /** 方位角（从北顺时针的方向，度）。 */
    double azimuth() const;
    void setAzimuth(double azimuth);

    /** 真垂直深度（米）。 */
    double tvd() const;
    void setTvd(double tvd);

    /** 东向位移（米）。 */
    double dx() const;
    void setDx(double dx);

    /** 北向位移（米）。 */
    double dy() const;
    void setDy(double dy);

private:
    double m_depth;
    double m_inclination;
    double m_azimuth;
    double m_tvd;
    double m_dx;
    double m_dy;
};

#endif // IDOS_WELL_SEGMENT_H
