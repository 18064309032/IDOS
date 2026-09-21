#include "idosgrid.h"

IDOSGrid::IDOSGrid(QObject* parent)
    : IDOSDataObject(parent)
    , m_type(Type::CornerPoint)
    , m_nx(0)
    , m_ny(0)
    , m_nz(0)
{
}

IDOSGrid::~IDOSGrid() = default;

IDOSGrid::Type IDOSGrid::type() const { return m_type; }
void IDOSGrid::setType(Type type)
{
    if (m_type == type) return;
    m_type = type;
    emit typeChanged(m_type);
    emit dataChanged();
}

int IDOSGrid::nx() const { return m_nx; }
int IDOSGrid::ny() const { return m_ny; }
int IDOSGrid::nz() const { return m_nz; }

void IDOSGrid::setDimensions(int nx, int ny, int nz)
{
    if (m_nx == nx && m_ny == ny && m_nz == nz) return;
    m_nx = nx;
    m_ny = ny;
    m_nz = nz;
    // 预分配
    m_coord.resize(4LL * (nx + 1) * (ny + 1) * 2);
    m_zcorn.resize(8LL * nx * ny * nz);
    m_actnum.fill(1, nx * ny * nz);
    emit dimensionsChanged(m_nx, m_ny, m_nz);
    emit dataChanged();
}

int IDOSGrid::coordIndex(int group, int i, int j) const
{
    // COORD 扁平：group 最快，然后 i，然后 j
    const int strideIJ = (m_ny + 1) * 2; // 每格 2 个 double (x,y)
    const int strideGroup = (m_nx + 1) * strideIJ;
    return group * strideGroup + i * strideIJ + j * 2;
}

int IDOSGrid::zcornIndex(int i, int j, int k, int cornerIndex) const
{
    // ZCORN 扁平：i 最快，然后 j，然后 k；每个 cell 8 个角
    return ((i * m_ny + j + k * m_nx * m_ny) * 8) + cornerIndex;
}

int IDOSGrid::actnumIndex(int i, int j, int k) const
{
    return i + j * m_nx + k * m_nx * m_ny;
}

QVector3D IDOSGrid::cornerPosition(int i, int j, int k, int cornerIndex) const
{
    QVector3D pos;

    // 根据 cornerIndex 选 COORD 组
    // 0,4 → group 0 (SW)    1,5 → group 1 (SE)
    // 2,6 → group 2 (NE)    3,7 → group 3 (NW)
    int group = 0;
    switch (cornerIndex)
    {
    case 0:
    case 4: group = 0; break;
    case 1:
    case 5: group = 1; break;
    case 2:
    case 6: group = 2; break;
    case 3:
    case 7: group = 3; break;
    default:
        return pos;
    }

    const int ci = coordIndex(group, i, j);
    pos.setX(m_coord[ci]);
    pos.setY(m_coord[ci + 1]);
    pos.setZ(m_zcorn[zcornIndex(i, j, k, cornerIndex)]);

    return pos;
}

IDOSGridCell IDOSGrid::cell(int i, int j, int k) const
{
    IDOSGridCell result;
    for (int c = 0; c < 8; ++c)
    {
        result.setCorner(c, cornerPosition(i, j, k, c));
    }
    result.setActive(isActive(i, j, k));
    return result;
}

bool IDOSGrid::isActive(int i, int j, int k) const
{
    if (m_actnum.isEmpty()) return true;
    return m_actnum[actnumIndex(i, j, k)] != 0;
}

int IDOSGrid::totalCellCount() const
{
    return m_nx * m_ny * m_nz;
}

int IDOSGrid::activeCellCount() const
{
    if (m_actnum.isEmpty()) return totalCellCount();
    int count = 0;
    for (int v : m_actnum)
    {
        if (v != 0) ++count;
    }
    return count;
}

void IDOSGrid::setCornerPointData(int nx, int ny, int nz,
                                  const QVector<double>& coord,
                                  const QVector<double>& zcorn,
                                  const QVector<int>& actnum)
{
    m_nx = nx;
    m_ny = ny;
    m_nz = nz;
    m_coord = coord;
    m_zcorn = zcorn;
    m_actnum = actnum;
    emit dimensionsChanged(m_nx, m_ny, m_nz);
    emit gridDataChanged();
    emit dataChanged();
}

const QVector<double>& IDOSGrid::coord() const { return m_coord; }
const QVector<double>& IDOSGrid::zcorn() const { return m_zcorn; }
const QVector<int>& IDOSGrid::actnum() const { return m_actnum; }
