#include <algorithm>

#include "idospointset.h"

IDOSPointSet::IDOSPointSet(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

QString IDOSPointSet::typeId() const
{
    return QString::fromLatin1("idos.example.pointset");
}

QVector<QVector3D> IDOSPointSet::points() const
{
    return m_points;
}

void IDOSPointSet::setPoints(const QVector<QVector3D>& points)
{
    if (m_points.size() != points.size() || !std::equal(m_points.cbegin(), m_points.cend(), points.cbegin()))
    {
        m_points = points;
        emit dataChanged();
    }
}
QVariantMap IDOSPointSet::projectData() const
{
    QVector<float> coordinates;
    coordinates.reserve(m_points.size() * 3);
    for (const QVector3D& point : m_points)
    {
        coordinates.append(point.x());
        coordinates.append(point.y());
        coordinates.append(point.z());
    }
    QVariantMap data;
    data.insert(QStringLiteral("coordinates"), QVariant::fromValue(coordinates));
    return data;
}

bool IDOSPointSet::restoreProjectData(const QVariantMap& data, QString& error)
{
    const QVariant field = data.value(QStringLiteral("coordinates"));
    if (field.userType() != qMetaTypeId<QVector<float>>())
    {
        error = tr("Stored point coordinates have an invalid type.");
        return false;
    }
    const QVector<float> coordinates = field.value<QVector<float>>();
    if (coordinates.size() % 3 != 0)
    {
        error = tr("Stored point coordinates have an invalid length.");
        return false;
    }
    m_points.clear();
    for (int index = 0; index < coordinates.size(); index += 3)
    {
        m_points.append(QVector3D(coordinates.at(index), coordinates.at(index + 1), coordinates.at(index + 2)));
    }
    return true;
}
