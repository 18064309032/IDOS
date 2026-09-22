#ifndef IDOS_WELL_PATH_H
#define IDOS_WELL_PATH_H

#include "idos_core.h"
#include "idoswellpathpoint.h"

#include <QVector>

/**
 * @brief 井眼轨迹（dev），IDOSWell 的组合成员。
 *
 * 与模拟器多段井模型（IDOSWellSegment / WELSEGS）是两回事：
 * 本类描述实测井身几何，WELSEGS 描述给模拟器的分段建模。
 * 普通类，值语义。
 */
class CORE_EXPORT IDOSWellPath
{
public:
    IDOSWellPath();

    /** 全部轨迹点。 */
    QVector<IDOSWellPathPoint> points() const;
    /** 整体替换轨迹点。 */
    void setPoints(const QVector<IDOSWellPathPoint>& points);

    /** 轨迹点数。 */
    int pointCount() const;
    bool isEmpty() const;
    void clear();

private:
    QVector<IDOSWellPathPoint> m_points;
};

#endif // IDOS_WELL_PATH_H
