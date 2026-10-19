#include <climits>
#include <utility>

#include <QObject>

#include "idosproject.h"

#include "idosgridobject.h"

IDOSGridObject::IDOSGridObject(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_type(Type::CornerPoint)
    , m_nx(0)
    , m_ny(0)
    , m_nz(0)
{
}

IDOSGridObject::~IDOSGridObject()
{
}

IDOSGridObject::Type IDOSGridObject::type() const
{
    return m_type;
}
void IDOSGridObject::setType(Type type)
{
    if (m_type == type)
    {
        return;
    }
    m_type = type;
    emit typeChanged(m_type);
    emit dataChanged();
}

int IDOSGridObject::nx() const
{
    return m_nx;
}
int IDOSGridObject::ny() const
{
    return m_ny;
}
int IDOSGridObject::nz() const
{
    return m_nz;
}

void IDOSGridObject::setDimensions(int nx, int ny, int nz)
{
    if (m_nx == nx && m_ny == ny && m_nz == nz)
    {
        return;
    }
    m_nx = nx;
    m_ny = ny;
    m_nz = nz;
    // 预分配
    m_coord.resize(6LL * (nx + 1) * (ny + 1));
    m_zcorn.resize(8LL * nx * ny * nz);
    m_actnum.fill(1, nx * ny * nz);
    emit dimensionsChanged(m_nx, m_ny, m_nz);
    emit dataChanged();
}

int IDOSGridObject::pillarCoordIndex(int i, int j) const
{
    return 6 * (i + j * (m_nx + 1));
}

QVector3D IDOSGridObject::pillarPosition(int i, int j, double z) const
{
    const int offset = pillarCoordIndex(i, j);
    const double topZ = m_coord[offset + 2];
    const double bottomZ = m_coord[offset + 5];
    const double fraction = bottomZ == topZ ? 0.0 : (z - topZ) / (bottomZ - topZ);
    const double x = m_coord[offset] + fraction * (m_coord[offset + 3] - m_coord[offset]);
    const double y = m_coord[offset + 1] + fraction * (m_coord[offset + 4] - m_coord[offset + 1]);
    return QVector3D(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

int IDOSGridObject::zcornIndex(int i, int j, int k, int cornerIndex) const
{
    // ZCORN 扁平：i 最快，然后 j，然后 k；每个 cell 8 个角
    return ((i + j * m_nx + k * m_nx * m_ny) * 8) + cornerIndex;
}

int IDOSGridObject::actnumIndex(int i, int j, int k) const
{
    return i + j * m_nx + k * m_nx * m_ny;
}

QVector3D IDOSGridObject::cornerPosition(int i, int j, int k, int cornerIndex) const
{
    QVector3D pos;

    // 根据 cornerIndex 选 COORD 组
    // 0,4 → group 0 (SW)    1,5 → group 1 (SE)
    // 2,6 → group 2 (NE)    3,7 → group 3 (NW)
    int group = 0;
    switch (cornerIndex)
    {
    case 0:
    case 4:
        group = 0;
        break;
    case 1:
    case 5:
        group = 1;
        break;
    case 2:
    case 6:
        group = 2;
        break;
    case 3:
    case 7:
        group = 3;
        break;
    default:
        return pos;
    }

    const int pillarI[4] = {0, 1, 1, 0};
    const int pillarJ[4] = {0, 0, 1, 1};
    return pillarPosition(i + pillarI[group], j + pillarJ[group],
                          m_zcorn[zcornIndex(i, j, k, cornerIndex)]);
}

IDOSGridCell IDOSGridObject::cell(int i, int j, int k) const
{
    IDOSGridCell result;
    for (int c = 0; c < 8; ++c)
    {
        result.setCorner(c, cornerPosition(i, j, k, c));
    }
    result.setActive(isActive(i, j, k));
    return result;
}

bool IDOSGridObject::isActive(int i, int j, int k) const
{
    if (m_actnum.isEmpty())
    {
        return true;
    }
    return m_actnum[actnumIndex(i, j, k)] != 0;
}

int IDOSGridObject::totalCellCount() const
{
    return m_nx * m_ny * m_nz;
}

int IDOSGridObject::activeCellCount() const
{
    if (m_actnum.isEmpty())
    {
        return totalCellCount();
    }
    int count = 0;
    for (int v : m_actnum)
    {
        if (v != 0)
        {
            ++count;
        }
    }
    return count;
}

void IDOSGridObject::setCornerPointData(int nx, int ny, int nz, QVector<double> coord, QVector<double> zcorn,
                                        QVector<int> actnum)
{
    m_nx = nx;
    m_ny = ny;
    m_nz = nz;
    m_coord = std::move(coord);
    m_zcorn = std::move(zcorn);
    m_actnum = std::move(actnum);
    emit dimensionsChanged(m_nx, m_ny, m_nz);
    emit gridDataChanged();
    emit dataChanged();
}

const QVector<double>& IDOSGridObject::coord() const
{
    return m_coord;
}
const QVector<double>& IDOSGridObject::zcorn() const
{
    return m_zcorn;
}
const QVector<int>& IDOSGridObject::actnum() const
{
    return m_actnum;
}

void IDOSGridObject::mergeFrom(const IDOSDataObject* other)
{
    if (other == nullptr || other->typeId() != typeId())
    {
        return; // 类型不符 no-op
    }
    const IDOSGridObject* g = static_cast<const IDOSGridObject*>(other);

    // 网格整体替换：dims + COORD/ZCORN/ACTNUM 一次性覆盖
    setCornerPointData(g->m_nx, g->m_ny, g->m_nz, g->m_coord, g->m_zcorn, g->m_actnum);
}

QString IDOSGridObject::typeId() const
{
    return QStringLiteral("idos.grid");
}

bool IDOSGridObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSGridObject::clone() const
{
    IDOSGridObject* copy = new IDOSGridObject();
    copy->setName(name());
    copy->m_type = m_type;
    copy->m_nx = m_nx;
    copy->m_ny = m_ny;
    copy->m_nz = m_nz;
    copy->m_coord = m_coord;
    copy->m_zcorn = m_zcorn;
    copy->m_actnum = m_actnum;
    return copy;
}

QVariantMap IDOSGridObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("type"), static_cast<int>(m_type));
    data.insert(QStringLiteral("nx"), QVariant::fromValue(m_nx));
    data.insert(QStringLiteral("ny"), QVariant::fromValue(m_ny));
    data.insert(QStringLiteral("nz"), QVariant::fromValue(m_nz));
    data.insert(QStringLiteral("coord"), QVariant::fromValue(m_coord));
    data.insert(QStringLiteral("zcorn"), QVariant::fromValue(m_zcorn));
    data.insert(QStringLiteral("actnum"), QVariant::fromValue(m_actnum));
    return data;
}

bool IDOSGridObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("type")) || data.value(QStringLiteral("type")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("type"));
        return false;
    }
    if (data.value(QStringLiteral("type")).toInt() < 0 || data.value(QStringLiteral("type")).toInt() >= 2)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("nx")) || data.value(QStringLiteral("nx")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("nx"));
        return false;
    }
    if (!data.contains(QStringLiteral("ny")) || data.value(QStringLiteral("ny")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("ny"));
        return false;
    }
    if (!data.contains(QStringLiteral("nz")) || data.value(QStringLiteral("nz")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("nz"));
        return false;
    }
    if (!data.contains(QStringLiteral("coord")) || data.value(QStringLiteral("coord")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coord"));
        return false;
    }
    if (!data.contains(QStringLiteral("zcorn")) || data.value(QStringLiteral("zcorn")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("zcorn"));
        return false;
    }
    if (!data.contains(QStringLiteral("actnum")) || data.value(QStringLiteral("actnum")).userType() != qMetaTypeId<QVector<int>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("actnum"));
        return false;
    }
    m_type = static_cast<Type>(data.value(QStringLiteral("type")).toInt());
    m_nx = data.value(QStringLiteral("nx")).value<int>();
    m_ny = data.value(QStringLiteral("ny")).value<int>();
    m_nz = data.value(QStringLiteral("nz")).value<int>();
    m_coord = data.value(QStringLiteral("coord")).value<QVector<double>>();
    m_zcorn = data.value(QStringLiteral("zcorn")).value<QVector<double>>();
    m_actnum = data.value(QStringLiteral("actnum")).value<QVector<int>>();
    return true;
}
bool IDOSGridObject::validateProjectReferences(const IDOSProject* project, QString& error) const
{
    Q_UNUSED(project);
    if (m_nx < 0 || m_ny < 0 || m_nz < 0 || static_cast<qint64>(m_nx) * m_ny > INT_MAX ||
        static_cast<qint64>(m_nx) * m_ny * m_nz > INT_MAX)
    {
        error = tr("Stored grid dimensions are invalid.");
        return false;
    }
    const qint64 cells = static_cast<qint64>(m_nx) * m_ny * m_nz;
    if ((!m_coord.isEmpty() && m_coord.size() != 6LL * (m_nx + 1LL) * (m_ny + 1LL)) ||
        (!m_zcorn.isEmpty() && m_zcorn.size() != 8LL * cells) ||
        (!m_actnum.isEmpty() && m_actnum.size() != cells))
    {
        error = tr("Stored grid arrays do not match their dimensions.");
        return false;
    }
    return true;
}
