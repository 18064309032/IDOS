#include <climits>
#include <utility>

#include <QObject>

#include "idosgridobject.h"
#include "idosproject.h"
#include "logger/idoslogger.h"

#include "idosgridpropertyobject.h"

IDOSGridPropertyObject::IDOSGridPropertyObject(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_kind(Kind::Static)
    , m_nx(0)
    , m_ny(0)
    , m_nz(0)
    , m_valueUnits(ValueUnits::SI)
    , m_currentReportStep(-1)
{
}

IDOSGridPropertyObject::~IDOSGridPropertyObject()
{
}

QString IDOSGridPropertyObject::containerId() const
{
    return m_gridId;
}

QString IDOSGridPropertyObject::keyword() const
{
    return m_keyword;
}

void IDOSGridPropertyObject::setKeyword(const QString& keyword)
{
    if (m_keyword == keyword)
    {
        return;
    }
    m_keyword = keyword;
    emit keywordChanged();
    emit dataChanged();
}

IDOSGridPropertyObject::Kind IDOSGridPropertyObject::kind() const
{
    return m_kind;
}

void IDOSGridPropertyObject::setKind(Kind kind)
{
    if (m_kind == kind)
    {
        return;
    }
    m_kind = kind;
    emit kindChanged(kind);
    emit dataChanged();
}

IDOSGridPropertyObject::ValueUnits IDOSGridPropertyObject::valueUnits() const
{
    return m_valueUnits;
}

void IDOSGridPropertyObject::setValueUnits(ValueUnits valueUnits)
{
    if (m_valueUnits == valueUnits)
    {
        return;
    }
    m_valueUnits = valueUnits;
    emit dataChanged();
}

QString IDOSGridPropertyObject::gridId() const
{
    return m_gridId;
}

void IDOSGridPropertyObject::setGridId(const QString& gridId)
{
    if (m_gridId == gridId)
    {
        return;
    }
    m_gridId = gridId;
    emit gridIdChanged();
    emit dataChanged();
}

int IDOSGridPropertyObject::numX() const
{
    return m_nx;
}
int IDOSGridPropertyObject::numY() const
{
    return m_ny;
}
int IDOSGridPropertyObject::numZ() const
{
    return m_nz;
}

void IDOSGridPropertyObject::setDimensions(int nx, int ny, int nz)
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

int IDOSGridPropertyObject::totalCellCount() const
{
    return m_nx * m_ny * m_nz;
}

const QVector<double>& IDOSGridPropertyObject::values() const
{
    QMap<int, QVector<double>>::const_iterator iterator =
        m_reportStepValues.constFind(m_currentReportStep);
    if (iterator != m_reportStepValues.constEnd())
    {
        return iterator.value();
    }
    return m_values;
}

const QVector<double>& IDOSGridPropertyObject::baseValues() const
{
    return m_values;
}

QList<int> IDOSGridPropertyObject::reportSteps() const
{
    return m_reportStepValues.keys();
}

QVector<double> IDOSGridPropertyObject::reportStepValues(int reportStep) const
{
    return m_reportStepValues.value(reportStep);
}

int IDOSGridPropertyObject::currentReportStep() const
{
    return m_currentReportStep;
}

bool IDOSGridPropertyObject::setReportStepValues(int reportStep, const QVector<double>& values)
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

bool IDOSGridPropertyObject::setCurrentReportStep(int reportStep)
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

bool IDOSGridPropertyObject::setValues(QVector<double> values)
{
    const int expected = totalCellCount();
    if (values.size() != expected)
    {
        IDOS_WARN(tr("IDOSGridPropertyObject::setValues: size mismatch for %1 - expected %2, got %3")
                      .arg(m_keyword)
                      .arg(expected)
                      .arg(values.size()));
        return false;
    }

    m_values = std::move(values);
    emit valuesChanged();
    emit dataChanged();
    return true;
}

double IDOSGridPropertyObject::valueAt(int i, int j, int k) const
{
    const QVector<double>& currentValues = values();
    return currentValues.value(cellIndex(i, j, k, m_nx, m_ny), 0.0);
}

void IDOSGridPropertyObject::setValueAt(int i, int j, int k, double value)
{
    const int index = cellIndex(i, j, k, m_nx, m_ny);
    if (index < 0 || index >= values().size())
    {
        IDOS_WARN(tr("IDOSGridPropertyObject::setValueAt: index out of range (%1, %2, %3)")
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

int IDOSGridPropertyObject::cellIndex(int i, int j, int k, int nx, int ny)
{
    return i + j * nx + k * nx * ny;
}

QString IDOSGridPropertyObject::typeId() const
{
    return QStringLiteral("idos.gridproperty");
}

bool IDOSGridPropertyObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSGridPropertyObject::clone() const
{
    IDOSGridPropertyObject* copy = new IDOSGridPropertyObject();
    copy->setName(name());
    copy->m_keyword = m_keyword;
    copy->m_kind = m_kind;
    copy->m_gridId = m_gridId;
    copy->m_nx = m_nx;
    copy->m_ny = m_ny;
    copy->m_nz = m_nz;
    copy->m_valueUnits = m_valueUnits;
    copy->m_values = m_values;
    copy->m_reportStepValues = m_reportStepValues;
    copy->m_currentReportStep = m_currentReportStep;
    return copy;
}

QVariantMap IDOSGridPropertyObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("keyword"), QVariant::fromValue(m_keyword));
    data.insert(QStringLiteral("kind"), static_cast<int>(m_kind));
    data.insert(QStringLiteral("gridId"), QVariant::fromValue(m_gridId));
    data.insert(QStringLiteral("nx"), QVariant::fromValue(m_nx));
    data.insert(QStringLiteral("ny"), QVariant::fromValue(m_ny));
    data.insert(QStringLiteral("nz"), QVariant::fromValue(m_nz));
    data.insert(QStringLiteral("valueUnits"), static_cast<int>(m_valueUnits));
    data.insert(QStringLiteral("values"), QVariant::fromValue(m_values));
    QVariantMap reportStepValues;
    for (QMap<int, QVector<double>>::const_iterator it = m_reportStepValues.constBegin(); it != m_reportStepValues.constEnd(); ++it)
    {
        reportStepValues.insert(QString::number(it.key()), QVariant::fromValue(it.value()));
    }
    data.insert(QStringLiteral("reportStepValues"), reportStepValues);
    data.insert(QStringLiteral("currentReportStep"), QVariant::fromValue(m_currentReportStep));
    return data;
}

bool IDOSGridPropertyObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("keyword")) || data.value(QStringLiteral("keyword")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("keyword"));
        return false;
    }
    if (!data.contains(QStringLiteral("kind")) || data.value(QStringLiteral("kind")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("kind"));
        return false;
    }
    if (data.value(QStringLiteral("kind")).toInt() < 0 || data.value(QStringLiteral("kind")).toInt() >= 2)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("gridId")) || data.value(QStringLiteral("gridId")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("gridId"));
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
    if (!data.contains(QStringLiteral("values")) || data.value(QStringLiteral("values")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("values"));
        return false;
    }
    if (!data.contains(QStringLiteral("reportStepValues")) || data.value(QStringLiteral("reportStepValues")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("reportStepValues"));
        return false;
    }
    if (!data.contains(QStringLiteral("currentReportStep")) || data.value(QStringLiteral("currentReportStep")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("currentReportStep"));
        return false;
    }
    if (data.contains(QStringLiteral("valueUnits")) &&
        (data.value(QStringLiteral("valueUnits")).userType() != qMetaTypeId<int>() ||
         data.value(QStringLiteral("valueUnits")).toInt() < 0 || data.value(QStringLiteral("valueUnits")).toInt() >= 2))
    {
        error = QObject::tr("A stored property unit state is invalid.");
        return false;
    }
    m_keyword = data.value(QStringLiteral("keyword")).value<QString>();
    m_kind = static_cast<Kind>(data.value(QStringLiteral("kind")).toInt());
    m_gridId = data.value(QStringLiteral("gridId")).value<QString>();
    m_nx = data.value(QStringLiteral("nx")).value<int>();
    m_ny = data.value(QStringLiteral("ny")).value<int>();
    m_nz = data.value(QStringLiteral("nz")).value<int>();
    m_valueUnits = data.contains(QStringLiteral("valueUnits"))
                       ? static_cast<ValueUnits>(data.value(QStringLiteral("valueUnits")).toInt())
                       : ValueUnits::SI;
    m_values = data.value(QStringLiteral("values")).value<QVector<double>>();
    m_reportStepValues.clear();
    const QVariantMap reportStepValues = data.value(QStringLiteral("reportStepValues")).toMap();
    for (QVariantMap::const_iterator it = reportStepValues.constBegin(); it != reportStepValues.constEnd(); ++it)
    {
        bool valid = false;
        const int step = it.key().toInt(&valid);
        if (!valid || it.value().userType() != qMetaTypeId<QVector<double>>())
        {
            error = QObject::tr("A stored report step is invalid.");
            return false;
        }
        m_reportStepValues.insert(step, it.value().value<QVector<double>>());
    }
    m_currentReportStep = data.value(QStringLiteral("currentReportStep")).value<int>();
    return true;
}
bool IDOSGridPropertyObject::validateProjectReferences(const IDOSProject* project, QString& error) const
{
    IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(project->object(m_gridId));
    if (grid == nullptr || grid->nx() != m_nx || grid->ny() != m_ny || grid->nz() != m_nz ||
        m_nx < 0 || m_ny < 0 || m_nz < 0 || static_cast<qint64>(m_nx) * m_ny > INT_MAX ||
        static_cast<qint64>(m_nx) * m_ny * m_nz != values().size())
    {
        error = tr("A stored property has an invalid grid, dimensions or value array.");
        return false;
    }
    for (QMap<int, QVector<double>>::const_iterator it = m_reportStepValues.constBegin(); it != m_reportStepValues.constEnd(); ++it)
    {
        if (it.key() < 0 || it.value().size() != values().size())
        {
            error = tr("A stored report step has an invalid value array.");
            return false;
        }
    }
    if (!m_reportStepValues.isEmpty() && !m_reportStepValues.contains(m_currentReportStep))
    {
        error = tr("The selected property report step is missing.");
        return false;
    }
    return true;
}
