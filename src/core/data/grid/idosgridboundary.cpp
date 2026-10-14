#include "idosgridboundary.h"

IDOSGridBoundary::IDOSGridBoundary(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_sourceFile()
    , m_xCoordinates()
    , m_yCoordinates()
    , m_zCoordinates()
    , m_groupIds()
    , m_coordinateReferenceSystem()
{
}

QString IDOSGridBoundary::typeId() const
{
    return QStringLiteral("idos.grid.boundary");
}

QString IDOSGridBoundary::sourceFile() const
{
    return m_sourceFile;
}

void IDOSGridBoundary::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
}

QVector<double> IDOSGridBoundary::xCoordinates() const
{
    return m_xCoordinates;
}

void IDOSGridBoundary::setXCoordinates(const QVector<double>& xCoordinates)
{
    m_xCoordinates = xCoordinates;
}

QVector<double> IDOSGridBoundary::yCoordinates() const
{
    return m_yCoordinates;
}

void IDOSGridBoundary::setYCoordinates(const QVector<double>& yCoordinates)
{
    m_yCoordinates = yCoordinates;
}

QVector<double> IDOSGridBoundary::zCoordinates() const
{
    return m_zCoordinates;
}

void IDOSGridBoundary::setZCoordinates(const QVector<double>& zCoordinates)
{
    m_zCoordinates = zCoordinates;
}

QVector<int> IDOSGridBoundary::groupIds() const
{
    return m_groupIds;
}

void IDOSGridBoundary::setGroupIds(const QVector<int>& groupIds)
{
    m_groupIds = groupIds;
}

QString IDOSGridBoundary::coordinateReferenceSystem() const
{
    return m_coordinateReferenceSystem;
}

void IDOSGridBoundary::setCoordinateReferenceSystem(const QString& coordinateReferenceSystem)
{
    m_coordinateReferenceSystem = coordinateReferenceSystem;
}
