#include <QObject>

#include "idoswellspatialreference.h"

IDOSWellSpatialReference::IDOSWellSpatialReference()
    :
    m_verticalDatum(VerticalDatum::Unknown)
    , m_depthDirection(DepthDirection::Unknown)
{
}

IDOSWellSpatialReference::VerticalDatum IDOSWellSpatialReference::verticalDatum() const
{
    return m_verticalDatum;
}

void IDOSWellSpatialReference::setVerticalDatum(VerticalDatum datum)
{
    m_verticalDatum = datum;
}

IDOSWellSpatialReference::DepthDirection IDOSWellSpatialReference::depthDirection() const
{
    return m_depthDirection;
}

void IDOSWellSpatialReference::setDepthDirection(DepthDirection direction)
{
    m_depthDirection = direction;
}

bool IDOSWellSpatialReference::hasKnownVerticalReference() const
{
    return m_verticalDatum != VerticalDatum::Unknown && m_depthDirection != DepthDirection::Unknown;
}

bool IDOSWellSpatialReference::isCompatibleWith(const IDOSWellSpatialReference& other) const
{
    return hasKnownVerticalReference() && other.hasKnownVerticalReference()
           && m_verticalDatum == other.m_verticalDatum && m_depthDirection == other.m_depthDirection;
}

QVariantMap IDOSWellSpatialReference::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("verticalDatum"), static_cast<int>(m_verticalDatum));
    data.insert(QStringLiteral("depthDirection"), static_cast<int>(m_depthDirection));
    return data;
}

bool IDOSWellSpatialReference::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("verticalDatum")) || data.value(QStringLiteral("verticalDatum")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("verticalDatum"));
        return false;
    }
    if (data.value(QStringLiteral("verticalDatum")).toInt() < 0 || data.value(QStringLiteral("verticalDatum")).toInt() >= 4)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("depthDirection")) || data.value(QStringLiteral("depthDirection")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("depthDirection"));
        return false;
    }
    if (data.value(QStringLiteral("depthDirection")).toInt() < 0 || data.value(QStringLiteral("depthDirection")).toInt() >= 3)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    m_verticalDatum = static_cast<VerticalDatum>(data.value(QStringLiteral("verticalDatum")).toInt());
    m_depthDirection = static_cast<DepthDirection>(data.value(QStringLiteral("depthDirection")).toInt());
    return true;
}
