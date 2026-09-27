#include "idoswellpath.h"

IDOSWellPath::IDOSWellPath()
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

void IDOSWellPath::clear()
{
    m_points.clear();
    m_sourceComments.clear();
}

QStringList IDOSWellPath::sourceComments() const
{
    return m_sourceComments;
}
void IDOSWellPath::setSourceComments(const QStringList& comments)
{
    m_sourceComments = comments;
}
