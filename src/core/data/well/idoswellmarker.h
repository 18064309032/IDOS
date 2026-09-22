#ifndef IDOS_WELL_MARKER_H
#define IDOS_WELL_MARKER_H

#include "idos_core.h"

#include <QString>

/**
 * @brief 井层位标记点（Well Tops，对应 .prn 文件一行）。
 *
 * 一口井在某层位上的 pick 深度。普通类，值语义。
 */
class CORE_EXPORT IDOSWellMarker
{
public:
    IDOSWellMarker();
    IDOSWellMarker(const QString& name, double depth);

    /** 层位名（如 "Top Etive"）。 */
    QString name() const;
    /** pick 深度（米）。 */
    double depth() const;
    void setDepth(double depth);

private:
    QString m_name;
    double m_depth;
};

#endif // IDOS_WELL_MARKER_H
