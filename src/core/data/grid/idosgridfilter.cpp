#include <cmath>

#include <QObject>
#include <QVariantList>

#include "idosgridproperty.h"

#include "idosgridfilter.h"

IDOSGridFilter::IDOSGridFilter(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_gridId()
    , m_criteria()
{
}

QString IDOSGridFilter::typeId() const
{
    return QStringLiteral("idos.grid.filter");
}

QString IDOSGridFilter::containerId() const
{
    return m_gridId;
}

QString IDOSGridFilter::gridId() const
{
    return m_gridId;
}

void IDOSGridFilter::setGridId(const QString& id)
{
    if (m_gridId != id)
    {
        m_gridId = id;
        emit dataChanged();
    }
}

QVariantMap IDOSGridFilter::criteria() const
{
    return m_criteria;
}

bool IDOSGridFilter::setCriteria(const QVariantMap& criteria)
{
    QString error;
    if (!validateCriteria(criteria, error))
    {
        return false;
    }
    if (m_criteria != criteria)
    {
        m_criteria = criteria;
        emit dataChanged();
    }
    return true;
}

bool IDOSGridFilter::validateCriteria(const QVariantMap& criteria, QString& error)
{
    if ((criteria.contains(QStringLiteral("axes")) && criteria.value(QStringLiteral("axes")).type() != QVariant::Map) ||
        (criteria.contains(QStringLiteral("conditions")) && criteria.value(QStringLiteral("conditions")).type() != QVariant::List))
    {
        error = QObject::tr("Filter criteria contain an invalid range or condition collection.");
        return false;
    }
    const QStringList flags = {QStringLiteral("axisUnion"), QStringLiteral("anyProperty"), QStringLiteral("invert")};
    for (const QString& flag : flags)
    {
        if (criteria.contains(flag) && criteria.value(flag).type() != QVariant::Bool)
        {
            error = QObject::tr("Filter combination and inversion options must be boolean values.");
            return false;
        }
    }
    for (const QString& key : QStringList{QStringLiteral("axisOperation"), QStringLiteral("propertyOperation")})
    {
        const QString operation = criteria.value(key, QStringLiteral("show")).toString();
        if (operation != QStringLiteral("show") && operation != QStringLiteral("hide") && operation != QStringLiteral("invert"))
        {
            error = QObject::tr("Filter operations must be Show, Hide or Invert.");
            return false;
        }
    }
    const QVariantMap axes = criteria.value(QStringLiteral("axes")).toMap();
    for (const QString& axis : QStringList{QStringLiteral("i"), QStringLiteral("j"), QStringLiteral("k")})
    {
        if (axes.contains(axis) && axes.value(axis).type() != QVariant::Map)
        {
            error = QObject::tr("Filter criteria contain an invalid range or condition collection.");
            return false;
        }
        const QVariantMap range = axes.value(axis).toMap();
        if (range.contains(QStringLiteral("enabled")) && range.value(QStringLiteral("enabled")).type() != QVariant::Bool)
        {
            error = QObject::tr("Filter combination and inversion options must be boolean values.");
            return false;
        }
        if (range.contains(QStringLiteral("values")) && range.value(QStringLiteral("values")).type() != QVariant::List)
        {
            error = QObject::tr("Filter criteria contain an invalid range or condition collection.");
            return false;
        }
        for (const QVariant& value : range.value(QStringLiteral("values")).toList())
        {
            bool valid = false;
            const int index = value.toInt(&valid);
            if (!valid || index < 1 || value.toDouble() != index)
            {
                error = QObject::tr("Grid index ranges must use positive integers with minimum not greater than maximum.");
                return false;
            }
        }
        if (range.value(QStringLiteral("enabled")).toBool())
        {
            bool firstValid = false;
            bool lastValid = false;
            const int first = range.value(QStringLiteral("minimum")).toInt(&firstValid);
            const int last = range.value(QStringLiteral("maximum")).toInt(&lastValid);
            if (!firstValid || !lastValid || first < 1 || last < first ||
                range.value(QStringLiteral("minimum")).toDouble() != first ||
                range.value(QStringLiteral("maximum")).toDouble() != last)
            {
                error = QObject::tr("Grid index ranges must use positive integers with minimum not greater than maximum.");
                return false;
            }
        }
    }
    for (const QVariant& entry : criteria.value(QStringLiteral("conditions")).toList())
    {
        const QVariantMap condition = entry.toMap();
        for (const QString& flag : QStringList{QStringLiteral("enabled"), QStringLiteral("includeMinimum"), QStringLiteral("includeMaximum"), QStringLiteral("includeInvalidValues")})
        {
            if (condition.contains(flag) && condition.value(flag).type() != QVariant::Bool)
            {
                error = QObject::tr("Filter combination and inversion options must be boolean values.");
                return false;
            }
        }
        if (!condition.value(QStringLiteral("enabled"), true).toBool())
        {
            continue;
        }
        const QString operation = condition.value(QStringLiteral("operation")).toString();
        bool minimumValid = false;
        bool maximumValid = false;
        const double minimum = condition.value(QStringLiteral("minimum")).toDouble(&minimumValid);
        const double maximum = condition.value(QStringLiteral("maximum")).toDouble(&maximumValid);
        bool epsilonValid = false;
        const double epsilon = condition.value(QStringLiteral("epsilon"), 0.0).toDouble(&epsilonValid);
        if (condition.value(QStringLiteral("propertyId")).toString().isEmpty() || !minimumValid || !maximumValid ||
            !std::isfinite(minimum) || !std::isfinite(maximum) || !epsilonValid || !std::isfinite(epsilon) || epsilon < 0.0 ||
            (operation != QStringLiteral("between") && operation != QStringLiteral("greaterEqual") &&
             operation != QStringLiteral("lessEqual") && operation != QStringLiteral("equal") &&
             operation != QStringLiteral("greater") && operation != QStringLiteral("less") && operation != QStringLiteral("notEqual")) ||
            (operation == QStringLiteral("between") && minimum > maximum))
        {
            error = QObject::tr("A property condition requires an available property, a comparison and a valid finite range.");
            return false;
        }
    }
    return true;
}

QStringList IDOSGridFilter::propertyIds(const QVariantMap& criteria)
{
    QStringList ids;
    for (const QVariant& entry : criteria.value(QStringLiteral("conditions")).toList())
    {
        if (!entry.toMap().value(QStringLiteral("enabled"), true).toBool())
        {
            continue;
        }
        const QString id = entry.toMap().value(QStringLiteral("propertyId")).toString();
        if (!ids.contains(id))
        {
            ids.append(id);
        }
    }
    return ids;
}

bool IDOSGridFilter::propertyMatches(int comparison, double value, double minimum, double maximum,
                                     double epsilon, bool includeMinimum, bool includeMaximum, bool includeInvalid)
{
    if (!std::isfinite(value))
    {
        return includeInvalid;
    }
    switch (comparison)
    {
    case 1:
        return value > minimum;
    case 2:
        return value >= minimum;
    case 3:
        return value < maximum;
    case 4:
        return value <= maximum;
    case 5:
        return std::abs(value - minimum) <= epsilon;
    case 6:
        return std::abs(value - minimum) > epsilon;
    default:
        return (includeMinimum ? value >= minimum : value > minimum) && (includeMaximum ? value <= maximum : value < maximum);
    }
}

QVector<quint8> IDOSGridFilter::selection(const QVariantMap& criteria, int nx, int ny, int nz,
                                         const QMap<QString, IDOSGridProperty*>& properties)
{
    if (nx <= 0 || ny <= 0 || nz <= 0)
    {
        return {};
    }
    const int count = nx * ny * nz;
    const QVariantList conditions = criteria.value(QStringLiteral("conditions")).toList();
    const bool any = criteria.value(QStringLiteral("anyProperty")).toBool();
    QVector<quint8> propertyMask(count, any ? 0 : 1);
    int conditionCount = 0;
    const QStringList comparisons = {QStringLiteral("between"), QStringLiteral("greater"), QStringLiteral("greaterEqual"),
        QStringLiteral("less"), QStringLiteral("lessEqual"), QStringLiteral("equal"), QStringLiteral("notEqual")};
    for (const QVariant& entry : conditions)
    {
        const QVariantMap condition = entry.toMap();
        if (!condition.value(QStringLiteral("enabled"), true).toBool())
        {
            continue;
        }
        ++conditionCount;
        IDOSGridProperty* property = properties.value(condition.value(QStringLiteral("propertyId")).toString());
        if (property == nullptr || property->values().size() != count)
        {
            return QVector<quint8>(count, 0);
        }
        const QVector<double>& values = property->values();
        const double minimum = condition.value(QStringLiteral("minimum")).toDouble();
        const double maximum = condition.value(QStringLiteral("maximum")).toDouble();
        const double epsilon = condition.value(QStringLiteral("epsilon"), 0.0).toDouble();
        const bool includeMinimum = condition.value(QStringLiteral("includeMinimum"), true).toBool();
        const bool includeMaximum = condition.value(QStringLiteral("includeMaximum"), true).toBool();
        const bool includeInvalid = condition.value(QStringLiteral("includeInvalidValues"), false).toBool();
        const int comparison = comparisons.indexOf(condition.value(QStringLiteral("operation")).toString());
        for (int index = 0; index < count; ++index)
        {
            const bool match = propertyMatches(comparison, values.at(index), minimum, maximum, epsilon, includeMinimum, includeMaximum, includeInvalid);
            propertyMask[index] = any ? propertyMask.at(index) || match : propertyMask.at(index) && match;
        }
    }
    const QVariantMap axes = criteria.value(QStringLiteral("axes")).toMap();
    const QStringList names = {QStringLiteral("i"), QStringLiteral("j"), QStringLiteral("k")};
    const int dimensions[3] = {nx, ny, nz};
    QVector<quint8> axisMasks[3];
    bool active[3];
    int axisCount = 0;
    for (int axis = 0; axis < 3; ++axis)
    {
        const QVariantMap range = axes.value(names.at(axis)).toMap();
        const bool enabled = range.value(QStringLiteral("enabled")).toBool();
        const QVariantList values = range.value(QStringLiteral("values")).toList();
        const int minimum = range.value(QStringLiteral("minimum")).toInt();
        const int maximum = range.value(QStringLiteral("maximum")).toInt();
        active[axis] = enabled || !values.isEmpty();
        axisCount += active[axis] ? 1 : 0;
        axisMasks[axis].fill(0, dimensions[axis]);
        if (enabled)
        {
            for (int value = qMax(1, minimum); value <= qMin(dimensions[axis], maximum); ++value)
            {
                axisMasks[axis][value - 1] = 1;
            }
        }
        for (const QVariant& entry : values)
        {
            const int value = entry.toInt();
            if (value >= 1 && value <= dimensions[axis])
            {
                axisMasks[axis][value - 1] = 1;
            }
        }
    }
    const bool axisUnion = criteria.value(QStringLiteral("axisUnion")).toBool();
    const bool axisInvert = criteria.value(QStringLiteral("axisOperation"), QStringLiteral("show")).toString() != QStringLiteral("show");
    const bool propertyInvert = criteria.value(QStringLiteral("propertyOperation"), QStringLiteral("show")).toString() != QStringLiteral("show");
    const bool invert = criteria.value(QStringLiteral("invert")).toBool();
    for (int index = 0; index < count; ++index)
    {
        const int indices[3] = {index % nx, (index / nx) % ny, index / (nx * ny)};
        bool axisMatch = !axisUnion;
        for (int axis = 0; axis < 3; ++axis)
        {
            if (active[axis])
            {
                const bool match = axisMasks[axis].at(indices[axis]) != 0;
                axisMatch = axisUnion ? axisMatch || match : axisMatch && match;
            }
        }
        axisMatch = axisCount == 0 ? true : axisInvert ? !axisMatch : axisMatch;
        const bool propertyMatch = conditionCount == 0 ? true : propertyInvert ? !propertyMask.at(index) : propertyMask.at(index) != 0;
        const bool match = axisMatch && propertyMatch;
        propertyMask[index] = invert ? !match : match;
    }
    return propertyMask;
}

bool IDOSGridFilter::matches(const QVariantMap& criteria, int i, int j, int k, int globalIndex,
                             const QMap<QString, IDOSGridProperty*>& properties)
{
    const QVariantMap axes = criteria.value(QStringLiteral("axes")).toMap();
    const bool axisUnion = criteria.value(QStringLiteral("axisUnion")).toBool();
    bool axisMatch = !axisUnion;
    int axisCount = 0;
    const QStringList names = {QStringLiteral("i"), QStringLiteral("j"), QStringLiteral("k")};
    const int indices[3] = {i + 1, j + 1, k + 1};
    for (int axis = 0; axis < 3; ++axis)
    {
        const QVariantMap range = axes.value(names.at(axis)).toMap();
        const QVariantList values = range.value(QStringLiteral("values")).toList();
        if (range.value(QStringLiteral("enabled")).toBool() || !values.isEmpty())
        {
            const bool match = (range.value(QStringLiteral("enabled")).toBool() && indices[axis] >= range.value(QStringLiteral("minimum")).toInt()
                && indices[axis] <= range.value(QStringLiteral("maximum")).toInt()) || values.contains(indices[axis]);
            axisMatch = axisUnion ? axisMatch || match : axisMatch && match;
            ++axisCount;
        }
    }
    axisMatch = axisCount == 0 ? true : criteria.value(QStringLiteral("axisOperation"), QStringLiteral("show")).toString() != QStringLiteral("show") ? !axisMatch : axisMatch;
    const bool any = criteria.value(QStringLiteral("anyProperty")).toBool();
    bool propertyMatch = !any;
    int conditionCount = 0;
    const QStringList comparisons = {QStringLiteral("between"), QStringLiteral("greater"), QStringLiteral("greaterEqual"),
        QStringLiteral("less"), QStringLiteral("lessEqual"), QStringLiteral("equal"), QStringLiteral("notEqual")};
    for (const QVariant& entry : criteria.value(QStringLiteral("conditions")).toList())
    {
        const QVariantMap condition = entry.toMap();
        if (!condition.value(QStringLiteral("enabled"), true).toBool())
        {
            continue;
        }
        ++conditionCount;
        IDOSGridProperty* property = properties.value(condition.value(QStringLiteral("propertyId")).toString());
        if (property == nullptr || globalIndex < 0 || globalIndex >= property->values().size())
        {
            return false;
        }
        const bool match = propertyMatches(comparisons.indexOf(condition.value(QStringLiteral("operation")).toString()), property->values().at(globalIndex),
            condition.value(QStringLiteral("minimum")).toDouble(), condition.value(QStringLiteral("maximum")).toDouble(),
            condition.value(QStringLiteral("epsilon"), 0.0).toDouble(), condition.value(QStringLiteral("includeMinimum"), true).toBool(),
            condition.value(QStringLiteral("includeMaximum"), true).toBool(), condition.value(QStringLiteral("includeInvalidValues"), false).toBool());
        propertyMatch = any ? propertyMatch || match : propertyMatch && match;
    }
    propertyMatch = conditionCount == 0 ? true : criteria.value(QStringLiteral("propertyOperation"), QStringLiteral("show")).toString() != QStringLiteral("show") ? !propertyMatch : propertyMatch;
    const bool match = axisMatch && propertyMatch;
    return criteria.value(QStringLiteral("invert")).toBool() ? !match : match;
}
