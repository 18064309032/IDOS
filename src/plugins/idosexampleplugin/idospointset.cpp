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
    if (m_points != points)
    {
        m_points = points;
        emit dataChanged();
    }
}
