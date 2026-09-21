#ifndef IDOS_WELL_H
#define IDOS_WELL_H

#include "../idosdataobject.h"
#include "idoswellcompletion.h"
#include "idoswellsegment.h"
#include "idoswellcontrol.h"
#include "idoswellhead.h"
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
    bool m_open;
};

#endif // IDOS_WELL_H
