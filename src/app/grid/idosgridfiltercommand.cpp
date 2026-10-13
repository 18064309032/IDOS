#include <QJsonArray>
#include <QObject>

#include "idosgrid.h"
#include "idosgridfilter.h"
#include "idosgridproperty.h"
#include "idosproject.h"

#include "idosgridfiltercommand.h"

IDOSGridFilterCommand::IDOSGridFilterCommand(IDOSProject* project, const QString& operation, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("grid.filter.") + operation, Type::Action, project, QObject::tr("Grid filter"))
    , m_operation(operation)
    , m_arguments(arguments)
    , m_filter(nullptr)
    , m_previousCriteria()
    , m_captured(false)
    , m_ownsDetached(false)
{
}

IDOSGridFilterCommand::~IDOSGridFilterCommand()
{
    if (m_ownsDetached && m_filter != nullptr && m_filter->parent() == nullptr)
    {
        delete m_filter.data();
    }
}

bool IDOSGridFilterCommand::validateCommand(QString& error) const
{
    if (project() == nullptr || (m_operation != QStringLiteral("create") && m_operation != QStringLiteral("update") && m_operation != QStringLiteral("delete")))
    {
        error = QObject::tr("The target project or filter operation is unavailable.");
        return false;
    }
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(project()->object(m_arguments.value(QStringLiteral("filterId")).toString()));
    if (m_operation != QStringLiteral("create") && filter == nullptr && m_filter == nullptr)
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    if (m_operation == QStringLiteral("delete"))
    {
        return true;
    }
    const QString gridId = m_operation == QStringLiteral("create") ? m_arguments.value(QStringLiteral("gridId")).toString()
        : (filter != nullptr ? filter->gridId() : m_filter->gridId());
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(project()->object(gridId));
    if (grid == nullptr || (m_operation == QStringLiteral("create") && m_arguments.value(QStringLiteral("name")).toString().trimmed().isEmpty()))
    {
        error = QObject::tr("A grid filter requires a source grid and a nonempty name.");
        return false;
    }
    if (!m_arguments.value(QStringLiteral("criteria")).isObject())
    {
        error = QObject::tr("Filter criteria must be an object.");
        return false;
    }
    const QVariantMap criteria = m_arguments.value(QStringLiteral("criteria")).toObject().toVariantMap();
    if (!IDOSGridFilter::validateCriteria(criteria, error))
    {
        return false;
    }
    const QVariantMap axes = criteria.value(QStringLiteral("axes")).toMap();
    const QStringList keys = {QStringLiteral("i"), QStringLiteral("j"), QStringLiteral("k")};
    const QList<int> dimensions = {grid->nx(), grid->ny(), grid->nz()};
    for (int index = 0; index < keys.size(); ++index)
    {
        const QVariantMap axis = axes.value(keys.at(index)).toMap();
        for (const QVariant& value : axis.value(QStringLiteral("values")).toList())
        {
            if (value.toInt() > dimensions.at(index))
            {
                error = QObject::tr("A filter index range exceeds the source grid dimensions.");
                return false;
            }
        }
        if (axis.value(QStringLiteral("enabled")).toBool() && axis.value(QStringLiteral("maximum")).toInt() > dimensions.at(index))
        {
            error = QObject::tr("A filter index range exceeds the source grid dimensions.");
            return false;
        }
    }
    QStringList propertyIds = IDOSGridFilter::propertyIds(criteria);
    const QString displayId = criteria.value(QStringLiteral("displayPropertyId")).toString();
    if (!displayId.isEmpty() && !propertyIds.contains(displayId))
    {
        propertyIds.append(displayId);
    }
    for (const QString& id : propertyIds)
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(project()->object(id));
        if (property == nullptr || property->gridId() != gridId || property->numX() != grid->nx() ||
            property->numY() != grid->ny() || property->numZ() != grid->nz())
        {
            error = QObject::tr("A filter property is missing or incompatible with the source grid.");
            return false;
        }
    }
    return true;
}

QJsonObject IDOSGridFilterCommand::buildPreview() const
{
    return m_arguments;
}

bool IDOSGridFilterCommand::apply(QString& error)
{
    Q_UNUSED(error)
    if (m_operation == QStringLiteral("create"))
    {
        if (m_filter == nullptr)
        {
            m_filter = new IDOSGridFilter();
            m_filter->setGridId(m_arguments.value(QStringLiteral("gridId")).toString());
            m_filter->setName(m_arguments.value(QStringLiteral("name")).toString().trimmed());
            m_filter->setCriteria(m_arguments.value(QStringLiteral("criteria")).toObject().toVariantMap());
        }
        project()->add(m_filter.data());
        m_ownsDetached = false;
    }
    else
    {
        m_filter = qobject_cast<IDOSGridFilter*>(project()->object(m_arguments.value(QStringLiteral("filterId")).toString()));
        if (m_filter == nullptr)
        {
            error = QObject::tr("The target filter is unavailable.");
            return false;
        }
        if (m_operation == QStringLiteral("delete"))
        {
            m_ownsDetached = project()->take(m_filter->objectId()) == m_filter.data();
        }
        else
        {
            if (!m_captured)
            {
                m_previousCriteria = m_filter->criteria();
                m_captured = true;
            }
            m_filter->setCriteria(m_arguments.value(QStringLiteral("criteria")).toObject().toVariantMap());
        }
    }
    QJsonObject result;
    result.insert(QStringLiteral("filterId"), m_filter->objectId());
    setResult(result);
    return true;
}

bool IDOSGridFilterCommand::revert(QString& error)
{
    if (project() == nullptr || m_filter == nullptr)
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    if (m_operation == QStringLiteral("create"))
    {
        m_ownsDetached = project()->take(m_filter->objectId()) == m_filter.data();
        return m_ownsDetached;
    }
    if (m_operation == QStringLiteral("delete"))
    {
        project()->add(m_filter.data());
        m_ownsDetached = false;
    }
    else
    {
        m_filter->setCriteria(m_previousCriteria);
    }
    return true;
}

IDOSGridFilterCommandMetadata::IDOSGridFilterCommandMetadata(const QString& operation)
    :
    IDOSCommandMetadata(QStringLiteral("grid.filter.") + operation, QObject::tr("Grid filter"),
        QObject::tr("Create, update or delete a saved grid filter."), IDOSCommand::Type::Action)
    , m_operation(operation)
{
}

QJsonObject IDOSGridFilterCommandMetadata::schema() const
{
    QJsonObject properties;
    QJsonArray required;
    const QStringList keys = m_operation == QStringLiteral("create")
        ? QStringList{QStringLiteral("gridId"), QStringLiteral("name"), QStringLiteral("criteria")}
        : m_operation == QStringLiteral("update")
            ? QStringList{QStringLiteral("filterId"), QStringLiteral("criteria")}
            : QStringList{QStringLiteral("filterId")};
    for (const QString& key : keys)
    {
        QJsonObject property;
        property.insert(QStringLiteral("type"), key == QStringLiteral("criteria") ? QStringLiteral("object") : QStringLiteral("string"));
        properties.insert(key, property);
        required.append(key);
    }
    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    schema.insert(QStringLiteral("properties"), properties);
    schema.insert(QStringLiteral("required"), required);
    return schema;
}

IDOSCommand* IDOSGridFilterCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSGridFilterCommand(project, m_operation, arguments);
}
