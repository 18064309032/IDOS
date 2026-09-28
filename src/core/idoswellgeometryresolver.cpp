#include "idoswellgeometryresolver.h"
#include "idoswellhead.h"
#include "idoswellpath.h"
#include "idoswellpathpoint.h"

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

    if (head.hasSurfacePosition() && head.hasSurfaceElevation()
        && head.spatialReference().hasKnownVerticalReference())
    {
        resolution.setStartPoint(IDOSWellGeometryResolution::StartPointType::Wellhead,
                                 QVector3D(head.surfaceX(), head.surfaceY(), head.surfaceElevation()));
    }
    return resolution;
}

bool IDOSWellGeometryResolver::canUseWellhead(const IDOSWellHead& head, const IDOSWellPath& path) const
{
    return path.startsAtReferenceDepth() && head.hasSurfacePosition() && head.hasSurfaceElevation()
           && head.spatialReference().isCompatibleWith(path.spatialReference());
}
