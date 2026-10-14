#include <QObject>

#include "idoswellpath.h"

IDOSWellPath::IDOSWellPath()
    :
    m_points()
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

QVariantMap IDOSWellPath::projectData() const
{
    QVariantMap data;
    QVariantList points;
    for (const IDOSWellPathPoint& value : m_points)
    {
        points.append(value.projectData());
    }
    data.insert(QStringLiteral("points"), points);
    data.insert(QStringLiteral("sourceComments"), QVariant::fromValue(m_sourceComments));
    data.insert(QStringLiteral("spatialReference"), m_spatialReference.projectData());
    return data;
}

bool IDOSWellPath::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("points")) || data.value(QStringLiteral("points")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("points"));
        return false;
    }
    if (!data.contains(QStringLiteral("sourceComments")) || data.value(QStringLiteral("sourceComments")).userType() != qMetaTypeId<QStringList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceComments"));
        return false;
    }
    if (!data.contains(QStringLiteral("spatialReference")) || data.value(QStringLiteral("spatialReference")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("spatialReference"));
        return false;
    }
    m_points.clear();
    for (const QVariant& entry : data.value(QStringLiteral("points")).toList())
    {
        IDOSWellPathPoint value;
        if (!value.restoreProjectData(entry.toMap(), error))
        {
            return false;
        }
        m_points.append(value);
    }
    m_sourceComments = data.value(QStringLiteral("sourceComments")).value<QStringList>();
    if (!m_spatialReference.restoreProjectData(data.value(QStringLiteral("spatialReference")).toMap(), error))
    {
        return false;
    }
    return true;
}
