#include <QJsonArray>
#include <QObject>

#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosproject.h"

#include "idosupdategridfiltercommand.h"

IDOSUpdateGridFilterCommand::IDOSUpdateGridFilterCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("grid.filter.update"), Type::Action, project, QObject::tr("Grid filter"))
    , m_arguments(arguments)
    , m_captured(false)
{
}

IDOSUpdateGridFilterCommand::~IDOSUpdateGridFilterCommand()
{
}

bool IDOSUpdateGridFilterCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }
    IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(project()->object(m_arguments.value(QStringLiteral("filterId")).toString()));
    if (filter == nullptr || (m_filter != nullptr && m_filter != filter))
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    const QString gridId = filter->gridId();
    IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(project()->object(gridId));
    if (grid == nullptr)
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
    if (!IDOSGridFilterObject::validateCriteria(criteria, error))
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
    QStringList propertyIds = IDOSGridFilterObject::propertyIds(criteria);
    const QString displayId = criteria.value(QStringLiteral("displayPropertyId")).toString();
    if (!displayId.isEmpty() && !propertyIds.contains(displayId))
    {
        propertyIds.append(displayId);
    }
    for (const QString& id : propertyIds)
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(project()->object(id));
        if (property == nullptr || property->gridId() != gridId || property->numX() != grid->nx() ||
            property->numY() != grid->ny() || property->numZ() != grid->nz())
        {
            error = QObject::tr("A filter property is missing or incompatible with the source grid.");
            return false;
        }
    }
    return true;
}

QJsonObject IDOSUpdateGridFilterCommand::buildPreview() const
{
    return m_arguments;
}

bool IDOSUpdateGridFilterCommand::apply(QString& error)
{
    m_filter = qobject_cast<IDOSGridFilterObject*>(project()->object(m_arguments.value(QStringLiteral("filterId")).toString()));
    if (m_filter == nullptr)
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    if (!m_captured)
    {
        m_previousCriteria = m_filter->criteria();
        m_captured = true;
    }
    if (!m_filter->setCriteria(m_arguments.value(QStringLiteral("criteria")).toObject().toVariantMap()))
    {
        error = QObject::tr("Filter criteria must be an object.");
        return false;
    }
    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("filterId"), m_filter->objectId());
    setResult(commandResult);
    return true;
}

bool IDOSUpdateGridFilterCommand::revert(QString& error)
{
    if (project() == nullptr || m_filter == nullptr || project()->object(m_filter->objectId()) != m_filter.data())
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    return m_filter->setCriteria(m_previousCriteria);
}

IDOSUpdateGridFilterCommandMetadata::IDOSUpdateGridFilterCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("grid.filter.update"), QObject::tr("Grid filter"),
        QObject::tr("Create, update or delete a saved grid filter."), IDOSCommand::Type::Action)
{
}

QJsonObject IDOSUpdateGridFilterCommandMetadata::schema() const
{
    QJsonObject properties;
    QJsonArray required;
    const QStringList keys{QStringLiteral("filterId"), QStringLiteral("criteria")};
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

IDOSCommand* IDOSUpdateGridFilterCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSUpdateGridFilterCommand(project, arguments);
}
