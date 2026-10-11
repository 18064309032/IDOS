#include "idoswellpath.h"

IDOSWellPath::IDOSWellPath()
    : m_points()
    , m_sourceComments()
    , m_spatialReference()
{
}

QVector<IDOSWellPathPoint> IDOSWellPath::points() const
{
    return m_points;
}

void IDOSWellPath::setPoints(const QVector<IDOSWellPathPoint>& points)
{
    m_points = points;
}

int IDOSWellPath::pointCount() const
{
    return m_points.size();
}

bool IDOSWellPath::isEmpty() const
{
    return m_points.isEmpty();
}

double IDOSWellPath::firstMd() const
{
    if (m_points.isEmpty())
    {
        return 0.0;
    }
    return m_points.first().md();
}

bool IDOSWellPath::startsAtReferenceDepth() const
{
    return !m_points.isEmpty() && m_points.first().md() == 0.0;
}

void IDOSWellPath::clear()
{
    m_points.clear();
    m_sourceComments.clear();
    m_spatialReference = IDOSWellSpatialReference();
}

QStringList IDOSWellPath::sourceComments() const
{
    return m_sourceComments;
}

void IDOSWellPath::setSourceComments(const QStringList& comments)
{
    m_sourceComments = comments;
}

IDOSWellSpatialReference IDOSWellPath::spatialReference() const
{
    return m_spatialReference;
}

void IDOSWellPath::setSpatialReference(const IDOSWellSpatialReference& reference)
{
    m_spatialReference = reference;
}
