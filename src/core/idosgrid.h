#ifndef IDOS_GRID_H
#define IDOS_GRID_H

#include <QVector>
#include <QVector3D>

#include "idosdataobject.h"
#include "idosgridcell.h"

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
        CornerPoint,  // 角点网格，COORD + ZCORN
        BlockCentered // 块中心网格，DX + DY + DZ + TOPS（暂未实现）
    };

    explicit IDOSGrid(QObject* parent = nullptr);
    ~IDOSGrid() override;

    QString typeId() const override;

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
     * @param coord COORD 数组，6*(nx+1)*(ny+1) 个 double（Eclipse 原始 pillar 顶 XYZ + 底 XYZ）
     * @param zcorn 每单元连续八个深度值，角点采用 VTK 环绕顺序，单元按 i、j、k 排列
     * @param actnum ACTNUM 数组，nx*ny*nz 个 int（0=不活跃）
     */
    void setCornerPointData(int nx, int ny, int nz, const QVector<double>& coord, const QVector<double>& zcorn,
                            const QVector<int>& actnum);

    /** COORD 原始数组（只读访问，用于导出/调试）。 */
    const QVector<double>& coord() const;

    /** ZCORN 每单元角点数组（只读访问，非 Eclipse 原始布局）。 */
    const QVector<double>& zcorn() const;

    /** ACTNUM 原始数组（只读访问）。 */
    const QVector<int>& actnum() const;

    /**
     * @brief 同名网格重复导入时整体替换（一个 grid 文件即一份完整网格）。
     *
     * 类型不符 no-op（基类约定）。命中即用 other 的 dims/COORD/ZCORN/ACTNUM
     * 全量覆盖本对象，再发射 gridDataChanged/dataChanged。
     */
    void mergeFrom(const IDOSDataObject* other) override;

    Q_SIGNAL void typeChanged(Type type);
    Q_SIGNAL void dimensionsChanged(int nx, int ny, int nz);
    Q_SIGNAL void gridDataChanged();

  private:
    int pillarCoordIndex(int i, int j) const;
    QVector3D pillarPosition(int i, int j, double z) const;
    int zcornIndex(int i, int j, int k, int cornerIndex) const;
    int actnumIndex(int i, int j, int k) const;

    Type m_type;
    int m_nx;
    int m_ny;
    int m_nz;

    // 扁平存储
    QVector<double> m_coord; // 6*(nx+1)*(ny+1) — Eclipse 原始 COORD
    QVector<double> m_zcorn; // 8*nx*ny*nz — 每个角的 Z
    QVector<int> m_actnum;   // nx*ny*nz
};

#endif // IDOS_GRID_H
