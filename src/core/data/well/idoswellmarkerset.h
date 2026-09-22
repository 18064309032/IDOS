#ifndef IDOS_WELL_MARKER_SET_H
#define IDOS_WELL_MARKER_SET_H

#include "idos_core.h"
#include "idoswellmarker.h"

#include <QList>
#include <QString>

/**
 * @brief 一口井的层位 pick 集合（"Well Tops" 树节点）。
 *
 * IDOSWell 的组合成员，普通类，值语义。重复导入按层位名
 * 覆盖该条，其余层位保留。
 */
class CORE_EXPORT IDOSWellMarkerSet
{
public:
    IDOSWellMarkerSet();

    /** 全部层位（按导入顺序）。 */
    QList<IDOSWellMarker> markers() const;
    /** 整体替换层位列表。 */
    void setMarkers(const QList<IDOSWellMarker>& markers);

    /** 加入一个层位；同名层位已存在则覆盖其深度。 */
    void addMarker(const IDOSWellMarker& marker);

    /** 按层位名取 pick；找不到返回 nullptr。 */
    const IDOSWellMarker* marker(const QString& name) const;

    int markerCount() const;
    bool isEmpty() const;
    void clear();

private:
    QList<IDOSWellMarker> m_markers;
};

#endif // IDOS_WELL_MARKER_SET_H
