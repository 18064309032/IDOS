#ifndef IDOS_WELL_GEOMETRY_RESOLVER_H
#define IDOS_WELL_GEOMETRY_RESOLVER_H

#include <QVector3D>

#include "idos_core.h"

class IDOSWellHead;
class IDOSWellPath;

class CORE_EXPORT IDOSWellGeometryResolution
{
  public:
    enum class StartPointType
    {
        Unknown,
        Wellhead,
        TrajectoryStart
    };

    IDOSWellGeometryResolution();
    IDOSWellGeometryResolution(StartPointType type, const QVector3D& point);

    StartPointType startPointType() const;
    bool hasStartPoint() const;
    QVector3D startPoint() const;

  private:

    StartPointType m_startPointType;
    QVector3D m_startPoint;

};

class CORE_EXPORT IDOSWellGeometryResolver
{
  public:
    IDOSWellGeometryResolver();

    IDOSWellGeometryResolution resolve(const IDOSWellHead& head, const IDOSWellPath& path) const;

  private:
    bool canUseWellhead(const IDOSWellHead& head, const IDOSWellPath& path) const;
};

#endif // IDOS_WELL_GEOMETRY_RESOLVER_H
