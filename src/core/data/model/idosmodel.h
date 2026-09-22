#ifndef IDOS_MODEL_H
#define IDOS_MODEL_H

#include "data/idosdataobject.h"
#include "idos_core.h"
#include <QStringList>

class IDOSProject;

/**
 * @brief 模拟工况对象（模型树的根业务对象，如 "UPSACLE_GASWATER_ECL"）。
 *
 * 工况 = 方案配置 + 对输入数据的引用集合（"配料清单"）：
 * - 通过 gridId/wellIds 引用 Project 中的网格与井（objectId 弱引用，
 *   数据本体永远只有一份，由 Project 统一持有）；
 * - 工况专属配置（井控参数、属性倍乘因子等）后续以子类型/值对象扩展。
 *
 * 引用约定：
 * - 被引用对象删除时由树/工程层联动处理（先断引用或提示）；
 * - 序列化时只写引用 ID，加载按 objectId 恢复。
 */
class CORE_EXPORT IDOSModel : public IDOSDataObject
{
    Q_OBJECT

public:
    explicit IDOSModel(QObject* parent = nullptr);
    ~IDOSModel() override;

    QString typeId() const override { return QStringLiteral("idos.model"); }

    // ===== 引用：网格 =====

    /** 引用的网格 objectId；空表示尚未组装网格。 */
    QString gridId() const;
    void setGridId(const QString& gridId);

    // ===== 引用：井 =====

    /** 本工况调度的井 objectId 列表（有序）。 */
    const QStringList& wellIds() const;

    /** 添加井引用；重复添加无副作用。 */
    void addWellRef(const QString& wellId);

    /** 移除井引用；不存在时无副作用。 */
    void removeWellRef(const QString& wellId);

    /** 是否引用了指定对象（删除保护查询用）。 */
    bool references(const QString& objectId) const;

    // ===== 引用解析（provider 留 pending 名/路径，Coordinator 后处理）=====

    /**
     * @brief provider 暂存的待解析井名清单（来自 .DATA 的 WELSPECS）。
     *
     * 不是序列化字段——只在导入瞬时存在，resolveReferences 跑完即清空。
     * 导入协调者 mergeFrom/addObject 完成后调 resolveReferences 把名字
     * 通过 Project::objectByName 转 objectId 写进 m_wellIds。
     */
    QStringList pendingWellNames() const;
    void setPendingWellNames(const QStringList& names);

    /**
     * @brief .DATA 的 GRID 段 INCLUDE 的 .EGRID 绝对路径。
     *
     * 瞬时字段——不序列化。pendingImportPaths() 返回它让 Coordinator
     * 递归把网格加进 Project，之后 resolveReferences 走 basename 找 grid
     * 把 gridId 补上。二次导入完成后清空。
     */
    QString pendingGridPath() const;
    void setPendingGridPath(const QString& path);

    /**
     * @brief 返回 [m_pendingGridPath]（空则返回空列表，基类约定）。
     */
    QStringList pendingImportPaths() const override;

    /**
     * @brief 用 Project 解析 pendingWellNames → wellIds + pendingGridPath → gridId。
     *
     * 幂等：命中的井名/gridId 解析后从 pending 清除；未命中的井名保留等
     * 下次导入补链。gridId 解析失败时 pending path 保留供下次。
     */
    void resolveReferences(IDOSProject* project) override;

    Q_SIGNAL void gridRefChanged();
    Q_SIGNAL void wellRefsChanged();

private:
    QString m_gridId;
    QStringList m_wellIds;
    QStringList m_pendingWellNames;   // 瞬时：resolveReferences 后清空命中部分
    QString m_pendingGridPath;        // 瞬时：gridId 解析后清空
};

#endif // IDOS_MODEL_H
