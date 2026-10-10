#include "idosrendermesh.h"

IDOSRenderMesh::IDOSRenderMesh(IDOSRenderProvider* provider)
    :
    IDOSRenderObject(provider)
    , m_displayMode(IDOSDisplayMode::Surface)
{
}

IDOSRenderMesh::~IDOSRenderMesh()
{
}

const QVector<QVector3D>& IDOSRenderMesh::points() const
{
    return m_points;
}

void IDOSRenderMesh::setPoints(const QVector<QVector3D>& points)
{
    m_points = points;
}

const QVector<int>& IDOSRenderMesh::hexahedra() const
{
    return m_hexahedra;
}

void IDOSRenderMesh::setHexahedra(const QVector<int>& hexahedra)
{
    m_hexahedra = hexahedra;
}

int IDOSRenderMesh::addPoint(const QVector3D& point)
{
    m_points.append(point);
    return m_points.size() - 1;
}

void IDOSRenderMesh::addHexahedron(int point0, int point1, int point2, int point3, int point4, int point5, int point6,
                                   int point7)
{
    addHexahedron(point0, point1, point2, point3, point4, point5, point6, point7, -1);
}

void IDOSRenderMesh::addHexahedron(int point0, int point1, int point2, int point3, int point4, int point5, int point6,
                                   int point7, int globalIndex)
{
    m_hexahedra.append(point0);
    m_hexahedra.append(point1);
    m_hexahedra.append(point2);
    m_hexahedra.append(point3);
    m_hexahedra.append(point4);
    m_hexahedra.append(point5);
    m_hexahedra.append(point6);
    m_hexahedra.append(point7);
    m_cellGlobalIndices.append(globalIndex);
}

int IDOSRenderMesh::hexahedronCount() const
{
    return m_hexahedra.size() / 8;
}

const QVector<int>& IDOSRenderMesh::cellGlobalIndices() const
{
    return m_cellGlobalIndices;
}

QString IDOSRenderMesh::cellScalarName() const
{
    return m_cellScalarName;
}

const QVector<double>& IDOSRenderMesh::cellScalars() const
{
    return m_cellScalars;
}

void IDOSRenderMesh::setCellScalars(const QString& name, const QVector<double>& values)
{
    m_cellScalarName = name;
    m_cellScalars = values;
}

bool IDOSRenderMesh::hasCellScalars() const
{
    return !m_cellScalarName.isEmpty() && m_cellScalars.size() == hexahedronCount();
}

IDOSDisplayMode IDOSRenderMesh::displayMode() const
{
    return m_displayMode;
}

void IDOSRenderMesh::setDisplayMode(IDOSDisplayMode mode)
{
    m_displayMode = mode;
}

void IDOSRenderMesh::clearCellScalars()
{
    m_cellScalarName.clear();
    m_cellScalars.clear();
}

void IDOSRenderMesh::clear()
{
    m_points.clear();
    m_hexahedra.clear();
    m_cellGlobalIndices.clear();
    m_cellScalarName.clear();
    m_cellScalars.clear();
    m_displayMode = IDOSDisplayMode::Surface;
}
