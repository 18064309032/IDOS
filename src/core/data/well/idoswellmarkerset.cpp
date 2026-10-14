#include <QObject>

#include "idoswellmarkerset.h"

IDOSWellMarkerSet::IDOSWellMarkerSet()
{
}

QList<IDOSWellMarker> IDOSWellMarkerSet::markers() const
{
    return m_markers;
}

void IDOSWellMarkerSet::setMarkers(const QList<IDOSWellMarker>& markers)
{
    m_markers = markers;
}

void IDOSWellMarkerSet::addMarker(const IDOSWellMarker& marker)
{
    for (int i = 0; i < m_markers.size(); ++i)
    {
        if (m_markers[i].name() == marker.name())
        {
            m_markers[i].setDepth(marker.depth()); // 同名覆盖深度
            return;
        }
    }
    m_markers.append(marker);
}

const IDOSWellMarker* IDOSWellMarkerSet::marker(const QString& name) const
{
    for (const IDOSWellMarker& m : m_markers)
    {
        if (m.name() == name)
        {
            return &m;
        }
    }
    return nullptr;
}

int IDOSWellMarkerSet::markerCount() const
{
    return m_markers.size();
}

bool IDOSWellMarkerSet::isEmpty() const
{
    return m_markers.isEmpty();
}

void IDOSWellMarkerSet::clear()
{
    m_markers.clear();
}

QVariantMap IDOSWellMarkerSet::projectData() const
{
    QVariantMap data;
    QVariantList markers;
    for (const IDOSWellMarker& value : m_markers)
    {
        markers.append(value.projectData());
    }
    data.insert(QStringLiteral("markers"), markers);
    return data;
}

bool IDOSWellMarkerSet::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("markers")) || data.value(QStringLiteral("markers")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("markers"));
        return false;
    }
    m_markers.clear();
    for (const QVariant& entry : data.value(QStringLiteral("markers")).toList())
    {
        IDOSWellMarker value;
        if (!value.restoreProjectData(entry.toMap(), error))
        {
            return false;
        }
        m_markers.append(value);
    }
    return true;
}
