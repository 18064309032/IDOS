#ifndef IDOS_GRID_CELL_H
#define IDOS_GRID_CELL_H

#include <QVector3D>

/**
 * @brief 网格单元值类型。
 *
 * 独立值类型，不继承 QObject。存储单个角点网格单元的 8 个顶点坐标。
 * 仅作为 IDOSGrid::cell() 的返回值，不作为主存储容器。
 *
 * 角点索引约定（遵循 Eclipse 标准）：
 * @code
 *   7 ------- 6       k+1 顶面
 *  / |      / |
 * 4 ------- 5 |
 * |  3 ----|-- 2      k 底面
 * | /      | /
 * 0 ------- 1
 * @endcode
 *   底: 0=(i,j,k), 1=(i+1,j,k), 2=(i+1,j+1,k), 3=(i,j+1,k)
 *   顶: 4=(i,j,k+1), 5=(i+1,j,k+1), 6=(i+1,j+1,k+1), 7=(i,j+1,k+1)
 */
class IDOSGridCell
{
public:
    IDOSGridCell();

    /** 查询指定角点坐标（0-7）。 */
    QVector3D corner(int index) const;
    void setCorner(int index, const QVector3D& position);

    /** 单元是否活跃。 */
    bool isActive() const;
    void setActive(bool active);

    /** 单元体积（立方米）。 */
    double volume() const;

    /** 单元中心坐标。 */
    QVector3D center() const;

private:
    QVector3D m_corners[8];
    bool m_active;
};

#endif // IDOS_GRID_CELL_H
