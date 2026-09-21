#include "idosgridproperty.h"
#include <QDebug>

IDOSGridProperty::IDOSGridProperty(QObject* parent)
    : IDOSDataObject(parent)
    , m_kind(Kind::Static)
    , m_nx(0)
    , m_ny(0)
    , m_nz(0)
{
}

IDOSGridProperty::~IDOSGridProperty() = default;

QString IDOSGridProperty::containerId() const { return m_gridId; }

QString IDOSGridProperty::keyword() const { return m_keyword; }

void IDOSGridProperty::setKeyword(const QString& keyword)
{
    if (m_keyword == keyword) return;
    m_keyword = keyword;
    emit keywordChanged();
    emit dataChanged();
}

IDOSGridProperty::Kind IDOSGridProperty::kind() const { return m_kind; }

void IDOSGridProperty::setKind(Kind kind)
{
    if (m_kind == kind) return;
    m_kind = kind;
    emit kindChanged(kind);
    emit dataChanged();
}

QString IDOSGridProperty::gridId() const { return m_gridId; }

void IDOSGridProperty::setGridId(const QString& gridId)
{
    if (m_gridId == gridId) return;
    m_gridId = gridId;
    emit gridIdChanged();
    emit dataChanged();
}

int IDOSGridProperty::numX() const { return m_nx; }
int IDOSGridProperty::numY() const { return m_ny; }
int IDOSGridProperty::numZ() const { return m_nz; }

void IDOSGridProperty::setDimensions(int nx, int ny, int nz)
{
    if (m_nx == nx && m_ny == ny && m_nz == nz) return;

    m_nx = nx;
    m_ny = ny;
    m_nz = nz;

    // 维度变化后旧数据布局失效，重置为空数组（由导入方重新填入）
    m_values.clear();

    emit dimensionsChanged(nx, ny, nz);
    emit dataChanged();
}

int IDOSGridProperty::totalCellCount() const { return m_nx * m_ny * m_nz; }

const QVector<double>& IDOSGridProperty::values() const { return m_values; }

bool IDOSGridProperty::setValues(const QVector<double>& values)
{
    const int expected = totalCellCount();
    if (values.size() != expected)
    {
        qWarning() << "IDOSGridProperty::setValues: size mismatch for" << m_keyword
                   << "- expected" << expected << "got" << values.size();
        return false;
    }

    m_values = values;
    emit valuesChanged();
    emit dataChanged();
    return true;
}

double IDOSGridProperty::valueAt(int i, int j, int k) const
{
    return m_values.value(cellIndex(i, j, k, m_nx, m_ny), 0.0);
}

void IDOSGridProperty::setValueAt(int i, int j, int k, double value)
{
    const int index = cellIndex(i, j, k, m_nx, m_ny);
    if (index < 0 || index >= m_values.size())
    {
        qWarning() << "IDOSGridProperty::setValueAt: index out of range"
                   << i << j << k;
        return;
    }

    if (m_values.at(index) == value) return;

    m_values[index] = value;
    emit valuesChanged();
    emit dataChanged();
}

int IDOSGridProperty::cellIndex(int i, int j, int k, int nx, int ny)
{
    return i + j * nx + k * nx * ny;
}
