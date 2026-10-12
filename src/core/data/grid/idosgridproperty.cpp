#include "log/idoslogger.h"

#include "idosgridproperty.h"

IDOSGridProperty::IDOSGridProperty(QObject* parent)
    : IDOSDataObject(parent)
    , m_kind(Kind::Static)
    , m_nx(0)
    , m_ny(0)
    , m_nz(0)
    , m_currentReportStep(-1)
{
}

IDOSGridProperty::~IDOSGridProperty()
{
}

QString IDOSGridProperty::containerId() const
{
    return m_gridId;
}

QString IDOSGridProperty::keyword() const
{
    return m_keyword;
}

void IDOSGridProperty::setKeyword(const QString& keyword)
{
    if (m_keyword == keyword)
    {
        return;
    }
    m_keyword = keyword;
    emit keywordChanged();
    emit dataChanged();
}

IDOSGridProperty::Kind IDOSGridProperty::kind() const
{
    return m_kind;
}

void IDOSGridProperty::setKind(Kind kind)
{
    if (m_kind == kind)
    {
        return;
    }
    m_kind = kind;
    emit kindChanged(kind);
    emit dataChanged();
}

QString IDOSGridProperty::gridId() const
{
    return m_gridId;
}

void IDOSGridProperty::setGridId(const QString& gridId)
{
    if (m_gridId == gridId)
    {
        return;
    }
    m_gridId = gridId;
    emit gridIdChanged();
    emit dataChanged();
}

int IDOSGridProperty::numX() const
{
    return m_nx;
}
int IDOSGridProperty::numY() const
{
    return m_ny;
}
int IDOSGridProperty::numZ() const
{
    return m_nz;
}

void IDOSGridProperty::setDimensions(int nx, int ny, int nz)
{
    if (m_nx == nx && m_ny == ny && m_nz == nz)
    {
        return;
    }

    m_nx = nx;
    m_ny = ny;
    m_nz = nz;

    // 维度变化后旧数据布局失效，重置为空数组（由导入方重新填入）
    m_values.clear();
    m_reportStepValues.clear();
    m_currentReportStep = -1;

    emit dimensionsChanged(nx, ny, nz);
    emit dataChanged();
}

int IDOSGridProperty::totalCellCount() const
{
    return m_nx * m_ny * m_nz;
}

const QVector<double>& IDOSGridProperty::values() const
{
    QMap<int, QVector<double>>::const_iterator iterator =
        m_reportStepValues.constFind(m_currentReportStep);
    if (iterator != m_reportStepValues.constEnd())
    {
        return iterator.value();
    }
    return m_values;
}

QList<int> IDOSGridProperty::reportSteps() const
{
    return m_reportStepValues.keys();
}

int IDOSGridProperty::currentReportStep() const
{
    return m_currentReportStep;
}

bool IDOSGridProperty::setReportStepValues(int reportStep, const QVector<double>& values)
{
    if (kind() != Kind::Dynamic || reportStep < 0 || values.size() != totalCellCount())
    {
        return false;
    }

    m_reportStepValues.insert(reportStep, values);
    if (m_currentReportStep < 0)
    {
        m_currentReportStep = reportStep;
    }
    if (m_currentReportStep == reportStep)
    {
        emit valuesChanged();
        emit dataChanged();
    }
    return true;
}

bool IDOSGridProperty::setCurrentReportStep(int reportStep)
{
    if (!m_reportStepValues.contains(reportStep))
    {
        return false;
    }
    if (m_currentReportStep == reportStep)
    {
        return true;
    }

    m_currentReportStep = reportStep;
    emit valuesChanged();
    emit dataChanged();
    return true;
}

bool IDOSGridProperty::setValues(const QVector<double>& values)
{
    const int expected = totalCellCount();
    if (values.size() != expected)
    {
        IDOS_WARN(tr("IDOSGridProperty::setValues: size mismatch for %1 - expected %2, got %3")
                      .arg(m_keyword)
                      .arg(expected)
                      .arg(values.size()));
        return false;
    }

    m_values = values;
    emit valuesChanged();
    emit dataChanged();
    return true;
}

double IDOSGridProperty::valueAt(int i, int j, int k) const
{
    const QVector<double>& currentValues = values();
    return currentValues.value(cellIndex(i, j, k, m_nx, m_ny), 0.0);
}

void IDOSGridProperty::setValueAt(int i, int j, int k, double value)
{
    const int index = cellIndex(i, j, k, m_nx, m_ny);
    if (index < 0 || index >= values().size())
    {
        IDOS_WARN(tr("IDOSGridProperty::setValueAt: index out of range (%1, %2, %3)")
                      .arg(i)
                      .arg(j)
                      .arg(k));
        return;
    }

    if (m_reportStepValues.contains(m_currentReportStep))
    {
        QMap<int, QVector<double>>::iterator iterator = m_reportStepValues.find(m_currentReportStep);
        if (iterator.value().at(index) == value)
        {
            return;
        }

        iterator.value()[index] = value;
    }
    else
    {
        if (m_values.at(index) == value)
        {
            return;
        }

        m_values[index] = value;
    }

    emit valuesChanged();
    emit dataChanged();
}

int IDOSGridProperty::cellIndex(int i, int j, int k, int nx, int ny)
{
    return i + j * nx + k * nx * ny;
}

QString IDOSGridProperty::typeId() const
{
    return QStringLiteral("idos.gridproperty");
}
