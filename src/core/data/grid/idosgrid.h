#ifndef IDOS_GRID_H
#define IDOS_GRID_H

#include "../idosdataobject.h"
#include "idosgridcell.h"
#include <QVector>
#include <QVector3D>

/**
 * @brief 网格数据对象。
 *
 * 支持 Eclipse 角点网格（COORD + ZCORN + ACTNUM）。
 * 内部采用扁平连续数组存储，保证大网格（百万级单元）的内存效率和遍历性能。
 */
class CORE_EXPORT IDOSGrid : public IDOSDataObject
{
    Q_OBJECT

public:
    /**
     * @brief 网格类型。
     */
    enum class Type
    {
        CornerPoint,    // 角点网格，COORD + ZCORN
        BlockCentered   // 块中心网格，DX + DY + DZ + TOPS（暂未实现）
    };

    explicit IDOSGrid(QObject* parent = nullptr);
    ~IDOSGrid() override;

    /** 网格类型。 */
    Type type() const;
    void setType(Type type);

    /** 网格尺寸。 */
    int nx() const;
    int ny() const;
    int nz() const;

    /** 设置网格尺寸（分配空数据）。 */
    void setDimensions(int nx, int ny, int nz);

    /**
     * @brief 查询单元角点坐标。
     * @param i,j,k 单元索引（0-based）
     * @param cornerIndex 角点索引（0-7，遵循 IDOSGridCell 约定）
     * @return 三维坐标
     */
    QVector3D cornerPosition(int i, int j, int k, int cornerIndex) const;

    /**
     * @brief 获取完整单元值对象。
     * @param i,j,k 单元索引
     * @return 包含 8 个角点和活跃状态的值类型
     */
    IDOSGridCell cell(int i, int j, int k) const;

    /** 判断单元是否活跃。 */
    bool isActive(int i, int j, int k) const;

    /** 总单元数（nx*ny*nz）。 */
    int totalCellCount() const;

    /** 活跃单元数。 */
    int activeCellCount() const;

    /**
     * @brief 一次性加载角点网格数据（导入时调用）。
     * @param nx,ny,nz 网格尺寸
     * @param coord COORD 数组，4*(nx+1)*(ny+1)*2 个 double（Eclipse 原始 XY）
     * @param zcorn ZCORN 数组，8*nx*ny*nz 个 double
     * @param actnum ACTNUM 数组，nx*ny*nz 个 int（0=不活跃）
     */
    void setCornerPointData(int nx, int ny, int nz,
                            const QVector<double>& coord,
                            const QVector<double>& zcorn,
                            const QVector<int>& actnum);

    /** COORD 原始数组（只读访问，用于导出/调试）。 */
    const QVector<double>& coord() const;

    /** ZCORN 原始数组（只读访问）。 */
    const QVector<double>& zcorn() const;

    /** ACTNUM 原始数组（只读访问）。 */
    const QVector<int>& actnum() const;

    Q_SIGNAL void typeChanged(Type type);
    Q_SIGNAL void dimensionsChanged(int nx, int ny, int nz);
    Q_SIGNAL void gridDataChanged();

private:
    // COORD 4 组含义：
    //   group 0 = 单元西南角 (i, j) 的 XY
    //   group 1 = 单元东南角 (i+1, j) 的 XY
    //   group 2 = 单元东北角 (i+1, j+1) 的 XY
    //   group 3 = 单元西北角 (i, j+1) 的 XY
    int coordIndex(int group, int i, int j) const;
    int zcornIndex(int i, int j, int k, int cornerIndex) const;
    int actnumIndex(int i, int j, int k) const;

    Type m_type;
    int m_nx;
    int m_ny;
    int m_nz;

    // 扁平存储
    QVector<double> m_coord;    // 4*(nx+1)*(ny+1)*2 — Eclipse 原始 XY
    QVector<double> m_zcorn;    // 8*nx*ny*nz — 每个角的 Z
    QVector<int> m_actnum;      // nx*ny*nz
};

#endif // IDOS_GRID_H
