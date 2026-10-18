#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QStringList>

#include "data/case/idoscaseitemref.h"
#include "data/case/idoscaseobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "data/well/idoswellobject.h"
#include "idosdataobject.h"
#include "idosproject.h"

#include "idosinspectobjectcommand.h"

IDOSInspectObjectCommand::IDOSInspectObjectCommand(IDOSProject* project, const QString& objectId,
                                                   int offset, int limit)
    :
    IDOSCommand(QStringLiteral("project.inspect"), Type::Query, project, QObject::tr("Inspect project object"))
    , m_objectId(objectId)
    , m_offset(offset)
    , m_limit(limit)
{
}

IDOSInspectObjectCommand::~IDOSInspectObjectCommand() = default;

bool IDOSInspectObjectCommand::validateCommand(QString& error) const
{
    if (m_objectId.isEmpty() || project()->object(m_objectId) == nullptr)
    {
        error = QObject::tr("The requested object is unavailable.");
        return false;
    }
    if (m_offset < 0 || m_limit < 1 || m_limit > 100)
    {
        error = QObject::tr("Offset must be nonnegative and limit must be between 1 and 100.");
        return false;
    }
    return true;
}

bool IDOSInspectObjectCommand::apply(QString& error)
{
    Q_UNUSED(error)
    IDOSDataObject* object = project()->object(m_objectId);
    QJsonObject result{{QStringLiteral("objectId"), object->objectId()},
                       {QStringLiteral("name"), object->name()},
                       {QStringLiteral("typeId"), object->typeId()},
                       {QStringLiteral("containerId"), object->containerId()}};
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(object);
    if (caseObject != nullptr)
    {
        const QList<IDOSCaseItemRef> refs = caseObject->itemRefs();
        QJsonArray entries;
        for (int index = m_offset; index < refs.size() && entries.size() < m_limit; ++index)
        {
            const IDOSCaseItemRef& ref = refs.at(index);
            IDOSDataObject* source = project()->object(ref.objectId());
            QJsonObject entry = QJsonObject::fromVariantMap(ref.projectData());
            entry.insert(QStringLiteral("available"), source != nullptr);
            if (source != nullptr)
            {
                entry.insert(QStringLiteral("name"), source->name());
                entry.insert(QStringLiteral("typeId"), source->typeId());
            }
            entries.append(entry);
        }
        result.insert(QStringLiteral("references"), entries);
        result.insert(QStringLiteral("total"), refs.size());
        result.insert(QStringLiteral("offset"), m_offset);
        result.insert(QStringLiteral("nextOffset"), m_offset < refs.size() - entries.size()
            ? m_offset + entries.size() : -1);
    }
    IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(object);
    if (grid != nullptr)
    {
        result.insert(QStringLiteral("dimensions"), QJsonArray{grid->nx(), grid->ny(), grid->nz()});
        result.insert(QStringLiteral("cellCount"), grid->totalCellCount());
    }
    IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
    if (property != nullptr)
    {
        result.insert(QStringLiteral("keyword"), property->keyword());
        result.insert(QStringLiteral("dimensions"), QJsonArray{property->numX(), property->numY(), property->numZ()});
        result.insert(QStringLiteral("gridId"), property->gridId());
    }
    IDOSWellObject* well = qobject_cast<IDOSWellObject*>(object);
    if (well != nullptr)
    {
        result.insert(QStringLiteral("wellType"), well->type() == IDOSWellObject::Type::Producer
            ? QStringLiteral("producer") : QStringLiteral("injector"));
        result.insert(QStringLiteral("referenceDepth"), well->referenceDepth());
        result.insert(QStringLiteral("cell"), QJsonArray{well->i(), well->j(), well->k()});
    }
    setResult(result);
    return true;
}

bool IDOSInspectObjectCommand::revert(QString& error)
{
    error = QObject::tr("Read-only queries cannot be undone.");
    return false;
}

IDOSInspectObjectCommandMetadata::IDOSInspectObjectCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("project.inspect"), QObject::tr("Inspect project object"),
                        QObject::tr("Inspect an existing object by ID. Return identity and supported summary fields, including paginated case references. Does not return bulk grid values or log samples."),
                        IDOSCommand::Type::Query)
{
}

IDOSInspectObjectCommandMetadata::~IDOSInspectObjectCommandMetadata() = default;

QJsonObject IDOSInspectObjectCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("objectId"), stringPropertySchema(QObject::tr("Object ID returned by a project query.")));
    properties.insert(QStringLiteral("offset"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("minimum"), 0}, {QStringLiteral("default"), 0}});
    properties.insert(QStringLiteral("limit"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("minimum"), 1}, {QStringLiteral("maximum"), 100}, {QStringLiteral("default"), 50}});
    return schemaWithProperties(properties, QJsonArray{QStringLiteral("objectId")});
}

IDOSCommand* IDOSInspectObjectCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    const QJsonObject properties = schema().value(QStringLiteral("properties")).toObject();
    if (!arguments.value(QStringLiteral("objectId")).isString())
    {
        return nullptr;
    }
    for (const QString& key : arguments.keys())
    {
        if (!properties.contains(key))
        {
            return nullptr;
        }
    }
    return new IDOSInspectObjectCommand(project, arguments.value(QStringLiteral("objectId")).toString(),
        arguments.contains(QStringLiteral("offset")) ? arguments.value(QStringLiteral("offset")).toInt(-1) : 0,
        arguments.contains(QStringLiteral("limit")) ? arguments.value(QStringLiteral("limit")).toInt(-1) : 50);
}
