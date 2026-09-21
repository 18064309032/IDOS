#ifndef IDOS_GRID_PROPERTY_H
#define IDOS_GRID_PROPERTY_H

#include "data/idosdataobject.h"
#include "idos_core.h"
#include <QVector>

/**
 * @brief 网格属性数据对象（PERMX/PORO 等三维属性场）。
 *
 * 每个属性是 nx*ny*nz 的标量场，Eclipse natural order 存储
 * （i 最快：index = i + j*nx + k*nx*ny）。
 *
 * 归属关系：属性通过 gridId 引用所属网格（对象本体由 Project 统一持有，
 * 符合"模型引用输入数据"的架构约定），containerId() 返回 gridId，
 * 模型树中属性作为网格 provider 子树内的对象节点展示（带可见性勾选框）。
 *
 * 静态属性（PERMX/PORO/SATNUM...）来自岩石/流体模型；
 * 动态属性（压力/饱和度场等）来自模拟结果（后续类型扩展）。
 */
class CORE_EXPORT IDOSGridProperty : public IDOSDataObject
{
    Q_OBJECT

public:
    /** 属性类别（对应 tNavigator 模型树的"静态属性/动态属性"分组）。 */
    enum class Kind
    {
        Static,     // 静态属性：PERMX、PORO 等
        Dynamic     // 动态属性：模拟结果场（压力、饱和度等）
    };

    explicit IDOSGridProperty(QObject* parent = nullptr);
    ~IDOSGridProperty() override;

    QString typeId() const override { return QStringLiteral("idos.gridproperty"); }

    /**
     * @brief 容器对象引用：返回所属网格的 objectId。
     *
     * 树模型据此把本对象归入网格的子树（非顶层节点）。
     */
    QString containerId() const override;

    /** 属性关键字名（如 "PERMX"），展示名默认取此值。 */
    QString keyword() const;
    void setKeyword(const QString& keyword);

    /** 属性类别。 */
    Kind kind() const;
    void setKind(Kind kind);

    /** 所属网格的 objectId（引用，不持有对象）。 */
    QString gridId() const;
    void setGridId(const QString& gridId);

    /** 属性场维度快照（应与所属网格一致；导入时确定）。 */
    int numX() const;
    int numY() const;
    int numZ() const;
    void setDimensions(int nx, int ny, int nz);

    /** 总单元数（nx*ny*nz）。 */
    int totalCellCount() const;

    // ===== 数据访问 =====

    /** 属性值数组（Eclipse natural order，长度 nx*ny*nz）。 */
    const QVector<double>& values() const;

    /**
     * @brief 一次性设置属性数组（导入时调用）。
     * @return true 成功；false 尺寸与维度不符，未写入。
     */
    bool setValues(const QVector<double>& values);

    /** 按单元索引取值。 */
    double valueAt(int i, int j, int k) const;

    /** 按单元索引写值（发射 valuesChanged）。 */
    void setValueAt(int i, int j, int k, double value);

    /** Eclipse natural order 线性索引：i 最快。 */
    static int cellIndex(int i, int j, int k, int nx, int ny);

    Q_SIGNAL void keywordChanged();
    Q_SIGNAL void kindChanged(Kind kind);
    Q_SIGNAL void gridIdChanged();
    Q_SIGNAL void dimensionsChanged(int nx, int ny, int nz);
    Q_SIGNAL void valuesChanged();

private:
    QString m_keyword;
    Kind m_kind;
    QString m_gridId;
    int m_nx;
    int m_ny;
    int m_nz;
    QVector<double> m_values;   // nx*ny*nz，natural order
};

#endif // IDOS_GRID_PROPERTY_H
