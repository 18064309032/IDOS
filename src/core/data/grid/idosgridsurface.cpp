#include "idosgridsurface.h"

IDOSGridSurface::IDOSGridSurface(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_sourceFile()
    , m_nx(0)
    , m_ny(0)
    , m_originX(0.0)
    , m_originY(0.0)
    , m_spacingX(0.0)
    , m_spacingY(0.0)
    , m_missingValue(0.0)
    , m_values()
    , m_coordinateReferenceSystem()
{
}

QString IDOSGridSurface::typeId() const
{
    return QStringLiteral("idos.grid.surface");
}

QString IDOSGridSurface::sourceFile() const
{
    return m_sourceFile;
}

void IDOSGridSurface::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
}

int IDOSGridSurface::nx() const
{
    return m_nx;
}

void IDOSGridSurface::setNx(int nx)
{
    m_nx = nx;
}

int IDOSGridSurface::ny() const
{
    return m_ny;
}

void IDOSGridSurface::setNy(int ny)
{
    m_ny = ny;
}

double IDOSGridSurface::originX() const
{
    return m_originX;
}

void IDOSGridSurface::setOriginX(double originX)
{
    m_originX = originX;
}

double IDOSGridSurface::originY() const
{
    return m_originY;
}

void IDOSGridSurface::setOriginY(double originY)
{
    m_originY = originY;
}

double IDOSGridSurface::spacingX() const
{
    return m_spacingX;
}

void IDOSGridSurface::setSpacingX(double spacingX)
{
    m_spacingX = spacingX;
}

double IDOSGridSurface::spacingY() const
{
    return m_spacingY;
}

void IDOSGridSurface::setSpacingY(double spacingY)
{
    m_spacingY = spacingY;
}

double IDOSGridSurface::missingValue() const
{
    return m_missingValue;
}

void IDOSGridSurface::setMissingValue(double missingValue)
{
    m_missingValue = missingValue;
}

QVector<double> IDOSGridSurface::values() const
{
    return m_values;
}

void IDOSGridSurface::setValues(const QVector<double>& values)
{
    m_values = values;
}

QString IDOSGridSurface::coordinateReferenceSystem() const
{
    return m_coordinateReferenceSystem;
}

void IDOSGridSurface::setCoordinateReferenceSystem(const QString& coordinateReferenceSystem)
{
    m_coordinateReferenceSystem = coordinateReferenceSystem;
}
