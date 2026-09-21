#include "idosgridcell.h"

IDOSGridCell::IDOSGridCell()
    : m_corners{QVector3D(), QVector3D(), QVector3D(), QVector3D(),
                QVector3D(), QVector3D(), QVector3D(), QVector3D()}
    , m_active(true)
{
}

QVector3D IDOSGridCell::corner(int index) const
{
    if (index < 0 || index >= 8) return QVector3D();
    return m_corners[index];
}

void IDOSGridCell::setCorner(int index, const QVector3D& position)
{
    if (index >= 0 && index < 8)
    {
        m_corners[index] = position;
    }
}

bool IDOSGridCell::isActive() const { return m_active; }
void IDOSGridCell::setActive(bool active) { m_active = active; }

// 四面体有向体积，结果可能为负，取绝对值
static double tetrahedronVolume(const QVector3D& a,
                                const QVector3D& b,
                                const QVector3D& c,
                                const QVector3D& d)
{
    QVector3D ba = b - a;
    QVector3D ca = c - a;
    QVector3D da = d - a;
    return qAbs(QVector3D::dotProduct(ba, QVector3D::crossProduct(ca, da))) / 6.0;
}

double IDOSGridCell::volume() const
{
    // 将六面体拆成 5 个四面体，结果相加
    // 顶点编号：0,1,2,3 底；4,5,6,7 顶
    const QVector3D* c = m_corners;
    double v = 0.0;
    v += tetrahedronVolume(c[0], c[1], c[2], c[6]);
    v += tetrahedronVolume(c[0], c[2], c[3], c[6]);
    v += tetrahedronVolume(c[0], c[4], c[5], c[6]);
    v += tetrahedronVolume(c[0], c[5], c[1], c[6]);
    v += tetrahedronVolume(c[0], c[6], c[7], c[4]);
    return v;
}

QVector3D IDOSGridCell::center() const
{
    QVector3D sum(0, 0, 0);
    for (int i = 0; i < 8; ++i)
    {
        sum += m_corners[i];
    }
    return sum / 8.0;
}
