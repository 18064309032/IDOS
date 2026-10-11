#include "idoswellhead.h"
#include "idoswellpath.h"
#include "idoswellpathpoint.h"

#include "idoswellgeometryresolver.h"

IDOSWellGeometryResolution::IDOSWellGeometryResolution()
    : m_startPointType(StartPointType::Unknown)
    , m_startPoint()
{
}

IDOSWellGeometryResolution::StartPointType IDOSWellGeometryResolution::startPointType() const
{
    return m_startPointType;
}

bool IDOSWellGeometryResolution::hasStartPoint() const
{
    return m_startPointType != StartPointType::Unknown;
}

QVector3D IDOSWellGeometryResolution::startPoint() const
{
    return m_startPoint;
}

void IDOSWellGeometryResolution::setStartPoint(StartPointType type, const QVector3D& point)
{
    m_startPointType = type;
    m_startPoint = point;
}

IDOSWellGeometryResolver::IDOSWellGeometryResolver()
{
}

IDOSWellGeometryResolution IDOSWellGeometryResolver::resolve(const IDOSWellHead& head,
                                                              const IDOSWellPath& path) const
{
    IDOSWellGeometryResolution resolution;
    if (!path.isEmpty())
    {
        if (canUseWellhead(head, path))
        {
            resolution.setStartPoint(IDOSWellGeometryResolution::StartPointType::Wellhead,
                                     QVector3D(head.surfaceX(), head.surfaceY(), head.surfaceElevation()));
            return resolution;
        }

        const IDOSWellPathPoint point = path.points().first();
        resolution.setStartPoint(IDOSWellGeometryResolution::StartPointType::TrajectoryStart,
                                 QVector3D(point.x(), point.y(), point.z()));
        return resolution;
    }

    // 独立井口只作为平面位置标记，不与轨迹深度做拼接；缺少高程或垂直基准时按 z=0 显示。
    if (head.hasSurfacePosition())
    {
        const double surfaceElevation = head.hasSurfaceElevation() ? head.surfaceElevation() : 0.0;
        resolution.setStartPoint(IDOSWellGeometryResolution::StartPointType::Wellhead,
                                 QVector3D(head.surfaceX(), head.surfaceY(), surfaceElevation));
    }
    return resolution;
}

bool IDOSWellGeometryResolver::canUseWellhead(const IDOSWellHead& head, const IDOSWellPath& path) const
{
    return path.startsAtReferenceDepth() && head.hasSurfacePosition() && head.hasSurfaceElevation()
           && head.spatialReference().isCompatibleWith(path.spatialReference());
}
