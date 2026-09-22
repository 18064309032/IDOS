#ifndef IDOS_WELL_H
#define IDOS_WELL_H

#include "data/idosdataobject.h"
#include "idoswellcompletion.h"
#include "idoswellsegment.h"
#include "idoswellcontrol.h"
#include "idoswellhead.h"
#include "idoswellpath.h"
#include "idoswelllogset.h"
#include "idoswellmarkerset.h"
#include <QList>

/**
 * @brief 油井数据对象。
 *
 * 对应 Eclipse WELSPECS、COMPDAT、WELSEGS、WLCON、WELLHEAD 关键字。
 */
class CORE_EXPORT IDOSWell : public IDOSDataObject
{
    Q_OBJECT

public:
    /**
     * @brief 井类型。
     */
    enum class Type
    {
        Injector,   // 注水井
        Producer    // 生产井
    };

    explicit IDOSWell(QObject* parent = nullptr);
    ~IDOSWell() override;

    QString typeId() const override { return QStringLiteral("idos.well"); }

    /** 井类型。 */
    Type type() const;
    void setType(Type type);

    /** 井点网格坐标。 */
    int i() const;
    void setI(int i);
    int j() const;
    void setJ(int j);
    int k() const;
    void setK(int k);

    /** 参考深度（米）。 */
    double referenceDepth() const;
    void setReferenceDepth(double depth);

    /** 射孔段列表（COMPDAT）。 */
    QList<IDOSWellCompletion> completions() const;
    void setCompletions(const QList<IDOSWellCompletion>& completions);
    void addCompletion(const IDOSWellCompletion& completion);

    /** 井轨迹段列表（WELSEGS）。 */
    QList<IDOSWellSegment> segments() const;
    void setSegments(const QList<IDOSWellSegment>& segments);
    void addSegment(const IDOSWellSegment& segment);

    /** 井控参数（WLCON）。 */
    IDOSWellControl wellControl() const;
    void setWellControl(const IDOSWellControl& control);

    /** 井头地面数据（WELLHEAD）。 */
    IDOSWellHead wellHead() const;
    void setWellHead(const IDOSWellHead& head);

    // ===== 井数据组合（按井名汇合的四类输入）=====

    /** 井眼轨迹（.dev，实测井身几何；区别于模拟器 segments/WELSEGS）。 */
    const IDOSWellPath& path() const;
    void setPath(const IDOSWellPath& path);

    /** 测井曲线集合（LAS）。 */
    const IDOSWellLogSet& logs() const;
    void setLogs(const IDOSWellLogSet& logs);

    /** 层位 pick 集合（Well Tops）。 */
    const IDOSWellMarkerSet& markers() const;
    void setMarkers(const IDOSWellMarkerSet& markers);

    // ===== 合并支持（mergeFrom 判断"该字段是否被填充过"用）=====

    /** 井头是否已被填充（provider 调过 setWellHead）。 */
    bool hasWellHead() const;
    /** 井眼轨迹是否已被填充。 */
    bool hasPath() const;
    /** 测井曲线集合是否已被填充。 */
    bool hasLogs() const;
    /** 层位 pick 集合是否已被填充。 */
    bool hasMarkers() const;

    /**
     * @brief 把同类井的已填充字段合并进本对象。
     *
     * 仅合并对方 hasXxx 为 true 的字段：header/path 整体覆盖；
     * logs 按 channel 名覆盖、tops 按 horizon 名覆盖（同名替换、新名追加）。
     * 类型不符（非 IDOSWell）no-op。供 IDOSImportCoordinator 多态分派。
     */
    void mergeFrom(const IDOSDataObject* other) override;

    /** 开关井状态。 */
    bool isOpen() const;
    void setOpen(bool open);

    Q_SIGNAL void typeChanged(Type type);
    Q_SIGNAL void gridLocationChanged(int i, int j, int k);
    Q_SIGNAL void referenceDepthChanged(double depth);
    Q_SIGNAL void completionsChanged(const QList<IDOSWellCompletion>& completions);
    Q_SIGNAL void segmentsChanged(const QList<IDOSWellSegment>& segments);
    Q_SIGNAL void wellControlChanged(const IDOSWellControl& control);
    Q_SIGNAL void wellHeadChanged(const IDOSWellHead& head);
    Q_SIGNAL void pathChanged(const IDOSWellPath& path);
    Q_SIGNAL void logsChanged(const IDOSWellLogSet& logs);
    Q_SIGNAL void markersChanged(const IDOSWellMarkerSet& markers);
    Q_SIGNAL void openStatusChanged(bool open);

private:
    Type m_type;
    int m_i;
    int m_j;
    int m_k;
    double m_referenceDepth;
    QList<IDOSWellCompletion> m_completions;
    QList<IDOSWellSegment> m_segments;
    IDOSWellControl m_wellControl;
    IDOSWellHead m_wellHead;
    IDOSWellPath m_path;
    IDOSWellLogSet m_logs;
    IDOSWellMarkerSet m_markers;
    bool m_hasWellHead;
    bool m_hasPath;
    bool m_hasLogs;
    bool m_hasMarkers;
    bool m_open;
};

#endif // IDOS_WELL_H
