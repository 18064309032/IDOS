#ifndef IDOS_WELL_PATH_H
#define IDOS_WELL_PATH_H

#include "idos_core.h"
#include "idoswellpathpoint.h"
#include "idoswellspatialreference.h"

#include <QVector>
#include <QStringList>

/**
 * @brief Wellbore trajectory, owned by IDOSWell.
 *
 * This class describes measured well geometry. It is distinct from an
 * IDOSWellSegment / WELSEGS simulation model.
 */
class CORE_EXPORT IDOSWellPath
{
  public:
    IDOSWellPath();

    QVector<IDOSWellPathPoint> points() const;
    void setPoints(const QVector<IDOSWellPathPoint>& points);

    int pointCount() const;
    bool isEmpty() const;
    double firstMd() const;
    bool startsAtReferenceDepth() const;
    void clear();

    QStringList sourceComments() const;
    void setSourceComments(const QStringList& comments);

    IDOSWellSpatialReference spatialReference() const;
    void setSpatialReference(const IDOSWellSpatialReference& reference);

  private:
    QVector<IDOSWellPathPoint> m_points;
    QStringList m_sourceComments;
    IDOSWellSpatialReference m_spatialReference;
};

#endif // IDOS_WELL_PATH_H
