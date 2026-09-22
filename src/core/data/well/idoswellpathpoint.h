#ifndef IDOS_WELL_PATH_POINT_H
#define IDOS_WELL_PATH_POINT_H

#include "idos_core.h"

/**
 * @brief 井眼轨迹点（实际井怎么钻的，对应 Petrel .dev 文件一行）。
 *
 * 独立类但不继承 QObject，保持值语义以便拷贝和容器存储。
 * 以 X/Y/Z/TVD 为真值；.dev 文件里的 AZIM/INCL 列在该
 * 数据集中不可靠，不纳入。Z 向下为负（dev 约定），TVD 为正深度。
 */
class CORE_EXPORT IDOSWellPathPoint
{
public:
    IDOSWellPathPoint();
    IDOSWellPathPoint(double md, double x, double y, double z, double tvd);

    /** 测深（米）。 */
    double md() const;
    /** 东向坐标。 */
    double x() const;
    /** 北向坐标。 */
    double y() const;
    /** 垂向坐标（向上为正）。 */
    double z() const;
    /** 真垂直深度（米）。 */
    double tvd() const;

private:
    double m_md;
    double m_x;
    double m_y;
    double m_z;
    double m_tvd;
};

#endif // IDOS_WELL_PATH_POINT_H
