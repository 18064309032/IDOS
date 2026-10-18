#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QStringList>

#include "idosdataobject.h"
#include "idosproject.h"
#include "idosprojectmetadata.h"

#include "idoslistobjectscommand.h"

IDOSListObjectsCommand::IDOSListObjectsCommand(IDOSProject* project, const QString& name,
                                               const QString& typeId, const QString& containerId,
                                               int offset, int limit)
    :
    IDOSCommand(QStringLiteral("project.objects"), Type::Query, project, QObject::tr("List project objects"))
    , m_nameFilter(name)
    , m_typeId(typeId)
    , m_containerId(containerId)
    , m_offset(offset)
    , m_limit(limit)
{
}

IDOSListObjectsCommand::~IDOSListObjectsCommand() = default;

bool IDOSListObjectsCommand::validateCommand(QString& error) const
{
    if (m_offset < 0 || m_limit < 1 || m_limit > 100)
    {
        error = QObject::tr("Offset must be nonnegative and limit must be between 1 and 100.");
        return false;
    }
    if (!m_containerId.isEmpty() && project()->object(m_containerId) == nullptr)
    {
        error = QObject::tr("The requested container is unavailable.");
        return false;
    }
    return true;
}

bool IDOSListObjectsCommand::apply(QString& error)
{
    Q_UNUSED(error)
    QMap<QString, IDOSDataObject*> matches;
    const QList<IDOSDataObject*> objects = project()->objects();
    for (IDOSDataObject* object : objects)
    {
        if ((m_nameFilter.isEmpty() || object->name().contains(m_nameFilter, Qt::CaseInsensitive)) &&
            (m_typeId.isEmpty() || object->typeId() == m_typeId) &&
            (m_containerId.isEmpty() || object->containerId() == m_containerId))
        {
            matches.insert(object->objectId(), object);
        }
    }
    QJsonArray entries;
    int index = 0;
    for (IDOSDataObject* object : matches)
    {
        if (index++ < m_offset)
        {
            continue;
        }
        if (entries.size() >= m_limit)
        {
            break;
        }
        entries.append(QJsonObject{{QStringLiteral("objectId"), object->objectId()},
                                   {QStringLiteral("name"), object->name()},
                                   {QStringLiteral("typeId"), object->typeId()},
                                   {QStringLiteral("containerId"), object->containerId()}});
    }
    QJsonObject result;
    result.insert(QStringLiteral("project"), QJsonObject::fromVariantMap(project()->metadata().projectData()));
    result.insert(QStringLiteral("currentCaseId"), project()->currentCaseId());
    result.insert(QStringLiteral("currentObjectId"), project()->currentObject() != nullptr
        ? project()->currentObject()->objectId() : QString());
    result.insert(QStringLiteral("objects"), entries);
    result.insert(QStringLiteral("total"), matches.size());
    result.insert(QStringLiteral("offset"), m_offset);
    result.insert(QStringLiteral("nextOffset"), m_offset < matches.size() - entries.size()
        ? m_offset + entries.size() : -1);
    setResult(result);
    return true;
}

bool IDOSListObjectsCommand::revert(QString& error)
{
    error = QObject::tr("Read-only queries cannot be undone.");
    return false;
}

IDOSListObjectsCommandMetadata::IDOSListObjectsCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("project.objects"), QObject::tr("List project objects"),
                        QObject::tr("REQUIRED for requests to list or find data in the current project. Execute this query and answer from its results; never tell the user to run this command. Lists object IDs, names, types and containers; supports name, type and container filters with pagination."),
                        IDOSCommand::Type::Query)
{
}

IDOSListObjectsCommandMetadata::~IDOSListObjectsCommandMetadata() = default;

QJsonObject IDOSListObjectsCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("name"), stringPropertySchema(QObject::tr("Optional case-insensitive name substring.")));
    properties.insert(QStringLiteral("typeId"), stringPropertySchema(QObject::tr("Optional exact data type ID.")));
    properties.insert(QStringLiteral("containerId"), stringPropertySchema(QObject::tr("Optional container object ID; this is not a case reference filter.")));
    properties.insert(QStringLiteral("offset"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("minimum"), 0}, {QStringLiteral("default"), 0}});
    properties.insert(QStringLiteral("limit"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("minimum"), 1}, {QStringLiteral("maximum"), 100}, {QStringLiteral("default"), 50}});
    return schemaWithProperties(properties, QJsonArray());
}

IDOSCommand* IDOSListObjectsCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    const QJsonObject properties = schema().value(QStringLiteral("properties")).toObject();
    for (const QString& key : arguments.keys())
    {
        if (!properties.contains(key) ||
            ((key == QStringLiteral("name") || key == QStringLiteral("typeId") || key == QStringLiteral("containerId")) &&
             !arguments.value(key).isString()))
        {
            return nullptr;
        }
    }
    return new IDOSListObjectsCommand(project, arguments.value(QStringLiteral("name")).toString(),
        arguments.value(QStringLiteral("typeId")).toString(), arguments.value(QStringLiteral("containerId")).toString(),
        arguments.contains(QStringLiteral("offset")) ? arguments.value(QStringLiteral("offset")).toInt(-1) : 0,
        arguments.contains(QStringLiteral("limit")) ? arguments.value(QStringLiteral("limit")).toInt(-1) : 50);
}
