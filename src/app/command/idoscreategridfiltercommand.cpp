#include <QJsonArray>
#include <QObject>

#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosproject.h"

#include "idoscreategridfiltercommand.h"

IDOSCreateGridFilterCommand::IDOSCreateGridFilterCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("grid.filter.create"), Type::Action, project, QObject::tr("Grid filter"))
    , m_arguments(arguments)
{
}

IDOSCreateGridFilterCommand::~IDOSCreateGridFilterCommand()
{
    if (m_filter != nullptr && m_filter->parent() == nullptr)
    {
        delete m_filter.data();
    }
}

bool IDOSCreateGridFilterCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }
    if (m_filter == nullptr && !result().isEmpty())
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    const QString gridId = m_arguments.value(QStringLiteral("gridId")).toString();
    IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(project()->object(gridId));
    if (grid == nullptr || m_arguments.value(QStringLiteral("name")).toString().trimmed().isEmpty())
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

QJsonObject IDOSCreateGridFilterCommand::buildPreview() const
{
    return m_arguments;
}

bool IDOSCreateGridFilterCommand::apply(QString& error)
{
    if (m_filter == nullptr)
    {
        m_filter = new IDOSGridFilterObject();
        m_filter->setGridId(m_arguments.value(QStringLiteral("gridId")).toString());
        m_filter->setName(m_arguments.value(QStringLiteral("name")).toString().trimmed());
        m_filter->setCriteria(m_arguments.value(QStringLiteral("criteria")).toObject().toVariantMap());
    }
    if (m_filter->parent() != nullptr || !project()->add(m_filter.data()))
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("filterId"), m_filter->objectId());
    setResult(commandResult);
    return true;
}

bool IDOSCreateGridFilterCommand::revert(QString& error)
{
    if (project() == nullptr || m_filter == nullptr || project()->object(m_filter->objectId()) != m_filter.data())
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    project()->take(m_filter->objectId());
    return true;
}

IDOSCreateGridFilterCommandMetadata::IDOSCreateGridFilterCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("grid.filter.create"), QObject::tr("Grid filter"),
        QObject::tr("Create, update or delete a saved grid filter."), IDOSCommand::Type::Action)
{
}

QJsonObject IDOSCreateGridFilterCommandMetadata::schema() const
{
    QJsonObject properties;
    QJsonArray required;
    const QStringList keys{QStringLiteral("gridId"), QStringLiteral("name"), QStringLiteral("criteria")};
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

IDOSCommand* IDOSCreateGridFilterCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSCreateGridFilterCommand(project, arguments);
}
