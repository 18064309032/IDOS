#include <QJsonArray>
#include <QObject>
#include <QSet>
#include <QSignalBlocker>
#include <QUuid>
#include <QVariantList>

#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/fluidrock/idosfluidrocktable.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"
#include "data/well/idoswell.h"
#include "idosdataobject.h"
#include "idosproject.h"

#include "idostransferobjectscommand.h"

IDOSTransferObjectsCommand::IDOSTransferObjectsCommand(
    IDOSProject* project, const QString& operation, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("objects.") + operation, Type::Action, project, QObject::tr("Transfer objects"))
    , m_operation(operation)
    , m_arguments(arguments)
    , m_initialized(false)
{
}

IDOSTransferObjectsCommand::~IDOSTransferObjectsCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_copies)
    {
        if (object != nullptr && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
}

bool IDOSTransferObjectsCommand::validateCommand(QString& error) const
{
    if (m_initialized)
    {
        return checkState(false, error);
    }
    const QJsonArray items = m_arguments.value(QStringLiteral("items")).toArray();
    IDOSCaseObject* targetCase = qobject_cast<IDOSCaseObject*>(
        project()->object(m_arguments.value(QStringLiteral("targetCaseId")).toString()));
    IDOSGrid* targetGrid = qobject_cast<IDOSGrid*>(
        project()->object(m_arguments.value(QStringLiteral("targetGridId")).toString()));
    if (items.isEmpty() || (m_operation != QStringLiteral("copy") && m_operation != QStringLiteral("move")))
    {
        error = QObject::tr("No transferable objects were selected.");
        return false;
    }
    const QString targetCaseId = m_arguments.value(QStringLiteral("targetCaseId")).toString();
    const QString targetGridId = m_arguments.value(QStringLiteral("targetGridId")).toString();
    if ((!targetCaseId.isEmpty() && targetCase == nullptr) || (!targetGridId.isEmpty() && targetGrid == nullptr))
    {
        error = QObject::tr("The destination is no longer available.");
        return false;
    }
    QSet<QString> movingNames;
    QSet<QString> seen;
    for (const QJsonValue& value : items)
    {
        const QJsonObject item = value.toObject();
        const QString id = item.value(QStringLiteral("objectId")).toString();
        IDOSDataObject* object = project()->object(id);
        if (object == nullptr || !object->canClone() || seen.contains(id))
        {
            error = QObject::tr("A selected object is missing, duplicated or cannot be copied.");
            return false;
        }
        seen.insert(id);
        if (!item.value(QStringLiteral("partKey")).toString().isEmpty() || !item.value(QStringLiteral("itemKey")).toString().isEmpty())
        {
            error = QObject::tr("Select a complete data object.");
            return false;
        }
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(object);
        IDOSGrid* grid = qobject_cast<IDOSGrid*>(object);
        IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(object);
        if (targetCase != nullptr && caseObject == nullptr && grid == nullptr && property == nullptr && filter == nullptr &&
            qobject_cast<IDOSWell*>(object) == nullptr && qobject_cast<IDOSFluidRockTable*>(object) == nullptr)
        {
            error = QObject::tr("This data type cannot be added to a simulation case.");
            return false;
        }
        if ((property != nullptr || filter != nullptr) && targetGrid == nullptr)
        {
            error = QObject::tr("Select a destination grid for properties or filters.");
            return false;
        }
        if (property != nullptr && (property->numX() != targetGrid->nx() ||
            property->numY() != targetGrid->ny() || property->numZ() != targetGrid->nz()))
        {
            error = QObject::tr("The property dimensions do not match the destination grid.");
            return false;
        }
        if (filter != nullptr && targetGrid->objectId() != filter->gridId())
        {
            error = QObject::tr("Copy filters together with their grid when changing grids.");
            return false;
        }
        if (grid != nullptr && targetCase == nullptr)
        {
            error = QObject::tr("Select a destination case for the grid.");
            return false;
        }
        if (caseObject != nullptr && m_operation == QStringLiteral("move"))
        {
            error = QObject::tr("Cases already belong to this project and cannot be moved into another case.");
            return false;
        }
        if (m_operation == QStringLiteral("move"))
        {
            const QString sourceCaseId = item.value(QStringLiteral("sourceCaseId")).toString();
            IDOSCaseObject* source = qobject_cast<IDOSCaseObject*>(project()->object(sourceCaseId));
            if (property != nullptr || filter != nullptr)
            {
                if (object->containerId() == targetGrid->objectId())
                {
                    error = QObject::tr("The object is already in the destination.");
                    return false;
                }
                if (property != nullptr)
                {
                    const QString name = property->name().toCaseFolded();
                    if (movingNames.contains(name))
                    {
                        error = QObject::tr("The selected properties have conflicting names.");
                        return false;
                    }
                    movingNames.insert(name);
                    for (IDOSDataObject* candidate : project()->objects())
                    {
                        IDOSGridFilter* dependent = qobject_cast<IDOSGridFilter*>(candidate);
                        if (dependent != nullptr && IDOSGridFilter::propertyIds(dependent->criteria()).contains(id))
                        {
                            error = QObject::tr("A grid filter uses this property. Copy it instead of moving it.");
                            return false;
                        }
                        IDOSGridProperty* existing = qobject_cast<IDOSGridProperty*>(candidate);
                        if (existing != nullptr && existing->gridId() == targetGrid->objectId() &&
                            existing->name().compare(property->name(), Qt::CaseInsensitive) == 0)
                        {
                            error = QObject::tr("The destination already contains an object with this name.");
                            return false;
                        }
                    }
                }
            }
            else
            {
                if (source == nullptr || targetCase == nullptr || source == targetCase)
                {
                    error = QObject::tr("Select a different destination case to move this object.");
                    return false;
                }
                bool found = false;
                for (const IDOSCaseItemRef& ref : source->itemRefs())
                {
                    if (ref.objectId() == id && ref.partKey().isEmpty() && ref.itemKey().isEmpty())
                    {
                        found = true;
                    }
                }
                if (!found)
                {
                    error = QObject::tr("The source case no longer contains the selected object.");
                    return false;
                }
                for (const IDOSCaseItemRef& ref : targetCase->itemRefs())
                {
                    if (ref.objectId() == id)
                    {
                        error = QObject::tr("The object is already in the destination.");
                        return false;
                    }
                }
            }
        }
    }
    // Selecting a parent and its independently stored descendants must not duplicate them twice.
    for (const QString& id : seen)
    {
        IDOSDataObject* object = project()->object(id);
        if (seen.contains(object->containerId()))
        {
            error = QObject::tr("Select either the parent object or its children.");
            return false;
        }
    }
    return true;
}

QString IDOSTransferObjectsCommand::uniqueName(const IDOSDataObject* object) const
{
    const QString base = QObject::tr("%1 copy").arg(object->name());
    QString name = base;
    int index = 2;
    bool exists = true;
    while (exists)
    {
        exists = false;
        const QList<IDOSDataObject*> objects = project()->objects();
        for (IDOSDataObject* candidate : objects)
        {
            if (candidate->typeId() == object->typeId() && candidate->name().compare(name, Qt::CaseInsensitive) == 0)
            {
                exists = true;
            }
        }
        for (const QPointer<IDOSDataObject>& candidate : m_copies)
        {
            if (candidate != nullptr && candidate->typeId() == object->typeId() &&
                candidate->name().compare(name, Qt::CaseInsensitive) == 0)
            {
                exists = true;
            }
        }
        if (exists)
        {
            name = QObject::tr("%1 copy %2").arg(object->name()).arg(index++);
        }
    }
    return name;
}

IDOSDataObject* IDOSTransferObjectsCommand::copyObject(
    IDOSDataObject* source, QHash<QString, QString>& ids, QString& error)
{
    if (source == nullptr)
    {
        error = QObject::tr("A dependent object is missing.");
        return nullptr;
    }
    if (ids.contains(source->objectId()))
    {
        for (const QPointer<IDOSDataObject>& copy : m_copies)
        {
            if (copy != nullptr && copy->objectId() == ids.value(source->objectId()))
            {
                return copy.data();
            }
        }
    }
    IDOSDataObject* copy = source->clone();
    if (copy == nullptr)
    {
        error = QObject::tr("A dependent object cannot be copied.");
        return nullptr;
    }
    copy->setName(uniqueName(source));
    ids.insert(source->objectId(), copy->objectId());
    m_copies.append(QPointer<IDOSDataObject>(copy));
    IDOSCaseObject* sourceCase = qobject_cast<IDOSCaseObject*>(source);
    IDOSCaseObject* copyCase = qobject_cast<IDOSCaseObject*>(copy);
    if (sourceCase != nullptr && copyCase != nullptr)
    {
        QHash<QString, QString> wellNames;
        QList<IDOSCaseItemRef> refs = sourceCase->itemRefs();
        for (IDOSCaseItemRef& ref : refs)
        {
            IDOSDataObject* child = copyObject(project()->object(ref.objectId()), ids, error);
            if (child == nullptr)
            {
                return nullptr;
            }
            IDOSWell* sourceWell = qobject_cast<IDOSWell*>(project()->object(ref.objectId()));
            if (sourceWell != nullptr)
            {
                wellNames.insert(sourceWell->name(), child->name());
            }
            ref.setObjectId(child->objectId());
            ref.setRefId(QUuid::createUuid().toString(QUuid::WithoutBraces));
        }
        copyCase->setItemRefs(refs);
        IDOSSimulationCaseObject* simulation = qobject_cast<IDOSSimulationCaseObject*>(copyCase);
        if (simulation != nullptr)
        {
            QVariantList rows = simulation->wellControlRows();
            for (QVariant& value : rows)
            {
                QVariantMap row = value.toMap();
                QVariantMap fields = row.value(QStringLiteral("fields")).toMap();
                const QString name = fields.value(QStringLiteral("WELL")).toString();
                if (wellNames.contains(name))
                {
                    fields.insert(QStringLiteral("WELL"), wellNames.value(name));
                    row.insert(QStringLiteral("fields"), fields);
                    value = row;
                }
            }
            simulation->setWellControlRows(rows);
        }
    }
    if (qobject_cast<IDOSGrid*>(source) != nullptr)
    {
        for (IDOSDataObject* child : project()->objects())
        {
            if (child->containerId() != source->objectId())
            {
                continue;
            }
            IDOSDataObject* childCopy = copyObject(child, ids, error);
            if (childCopy == nullptr)
            {
                return nullptr;
            }
            IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(childCopy);
            IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(childCopy);
            if (property != nullptr)
            {
                property->setGridId(copy->objectId());
            }
            if (filter != nullptr)
            {
                filter->setGridId(copy->objectId());
            }
            // Child names remain unchanged inside an independent copied grid.
            childCopy->setName(child->name());
        }
    }
    return copy;
}

void IDOSTransferObjectsCommand::remapFilters(const QHash<QString, QString>& ids)
{
    for (const QPointer<IDOSDataObject>& copy : m_copies)
    {
        IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(copy.data());
        if (filter == nullptr)
        {
            continue;
        }
        QVariantMap criteria = filter->criteria();
        QVariantList conditions = criteria.value(QStringLiteral("conditions")).toList();
        for (QVariant& value : conditions)
        {
            QVariantMap condition = value.toMap();
            const QString oldId = condition.value(QStringLiteral("propertyId")).toString();
            if (ids.contains(oldId))
            {
                condition.insert(QStringLiteral("propertyId"), ids.value(oldId));
            }
            value = condition;
        }
        criteria.insert(QStringLiteral("conditions"), conditions);
        filter->setCriteria(criteria);
    }
}

bool IDOSTransferObjectsCommand::prepare(QString& error)
{
    const QString targetCaseId = m_arguments.value(QStringLiteral("targetCaseId")).toString();
    const QString targetGridId = m_arguments.value(QStringLiteral("targetGridId")).toString();
    QHash<QString, QString> ids;
    const QJsonArray items = m_arguments.value(QStringLiteral("items")).toArray();
    for (const QJsonValue& value : items)
    {
        const QJsonObject item = value.toObject();
        IDOSDataObject* source = project()->object(item.value(QStringLiteral("objectId")).toString());
        IDOSDataObject* object = source;
        if (m_operation == QStringLiteral("copy"))
        {
            object = copyObject(source, ids, error);
            if (object == nullptr)
            {
                return false;
            }
        }
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(object);
        if (property != nullptr || filter != nullptr)
        {
            if (m_operation == QStringLiteral("copy"))
            {
                if (property != nullptr)
                {
                    property->setGridId(targetGridId);
                }
                if (filter != nullptr)
                {
                    filter->setGridId(targetGridId);
                }
            }
            else
            {
                m_beforeContainers.insert(object->objectId(), object->containerId());
                m_afterContainers.insert(object->objectId(), targetGridId);
            }
            continue;
        }
        if (qobject_cast<IDOSCaseObject*>(object) != nullptr || targetCaseId.isEmpty())
        {
            continue;
        }
        if (!m_beforeRefs.contains(targetCaseId))
        {
            m_beforeRefs.insert(targetCaseId, qobject_cast<IDOSCaseObject*>(project()->object(targetCaseId))->itemRefs());
            m_afterRefs.insert(targetCaseId, m_beforeRefs.value(targetCaseId));
        }
        IDOSCaseItemRef ref;
        if (m_operation == QStringLiteral("move"))
        {
            const QString sourceId = item.value(QStringLiteral("sourceCaseId")).toString();
            if (!m_beforeRefs.contains(sourceId))
            {
                m_beforeRefs.insert(sourceId, qobject_cast<IDOSCaseObject*>(project()->object(sourceId))->itemRefs());
                m_afterRefs.insert(sourceId, m_beforeRefs.value(sourceId));
            }
            QList<IDOSCaseItemRef>& refs = m_afterRefs[sourceId];
            for (int index = refs.size() - 1; index >= 0; --index)
            {
                if (refs.at(index).objectId() == source->objectId())
                {
                    ref = refs.takeAt(index);
                }
            }
        }
        else
        {
            ref.setRefId(QUuid::createUuid().toString(QUuid::WithoutBraces));
            ref.setRole(qobject_cast<IDOSGrid*>(object) != nullptr ? QStringLiteral("case.grid")
                : qobject_cast<IDOSWell*>(object) != nullptr ? QStringLiteral("case.well") : QStringLiteral("case.fluidRockTable"));
            ref.setObjectId(object->objectId());
        }
        m_afterRefs[targetCaseId].append(ref);
    }
    remapFilters(ids);
    m_initialized = true;
    return true;
}

bool IDOSTransferObjectsCommand::checkState(bool applied, QString& error) const
{
    const QHash<QString, QList<IDOSCaseItemRef>>& refs = applied ? m_afterRefs : m_beforeRefs;
    for (QHash<QString, QList<IDOSCaseItemRef>>::const_iterator it = refs.constBegin(); it != refs.constEnd(); ++it)
    {
        IDOSCaseObject* object = qobject_cast<IDOSCaseObject*>(project()->object(it.key()));
        if (object == nullptr || !sameReferences(object->itemRefs(), it.value()))
        {
            error = QObject::tr("The case contents changed outside this operation.");
            return false;
        }
    }
    const QHash<QString, QString>& containers = applied ? m_afterContainers : m_beforeContainers;
    for (QHash<QString, QString>::const_iterator it = containers.constBegin(); it != containers.constEnd(); ++it)
    {
        IDOSDataObject* object = project()->object(it.key());
        if (object == nullptr || object->containerId() != it.value() || project()->object(it.value()) == nullptr)
        {
            error = QObject::tr("The object or its destination is no longer available.");
            return false;
        }
    }
    for (const QPointer<IDOSDataObject>& copy : m_copies)
    {
        if (copy == nullptr || (applied ? project()->object(copy->objectId()) != copy.data()
                                       : project()->object(copy->objectId()) != nullptr || copy->parent() != nullptr))
        {
            error = QObject::tr("A copied object is no longer available.");
            return false;
        }
    }
    return true;
}

void IDOSTransferObjectsCommand::writeState(bool applied)
{
    project()->beginUpdate();
    if (applied)
    {
        for (const QPointer<IDOSDataObject>& copy : m_copies)
        {
            project()->add(copy.data());
        }
    }
    const QHash<QString, QString>& containers = applied ? m_afterContainers : m_beforeContainers;
    for (QHash<QString, QString>::const_iterator it = containers.constBegin(); it != containers.constEnd(); ++it)
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(project()->object(it.key()));
        IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(project()->object(it.key()));
        if (property != nullptr)
        {
            property->setGridId(it.value());
        }
        if (filter != nullptr)
        {
            filter->setGridId(it.value());
        }
    }
    const QHash<QString, QList<IDOSCaseItemRef>>& refs = applied ? m_afterRefs : m_beforeRefs;
    for (QHash<QString, QList<IDOSCaseItemRef>>::const_iterator it = refs.constBegin(); it != refs.constEnd(); ++it)
    {
        IDOSCaseObject* object = qobject_cast<IDOSCaseObject*>(project()->object(it.key()));
        const QSignalBlocker blocker(object);
        object->setItemRefs(it.value());
    }
    for (QHash<QString, QList<IDOSCaseItemRef>>::const_iterator it = refs.constBegin(); it != refs.constEnd(); ++it)
    {
        emit project()->object(it.key())->dataChanged();
    }
    if (!applied)
    {
        for (int index = m_copies.size() - 1; index >= 0; --index)
        {
            project()->take(m_copies.at(index)->objectId());
        }
    }
    project()->endUpdate();
}

bool IDOSTransferObjectsCommand::apply(QString& error)
{
    if (!m_initialized && !prepare(error))
    {
        return false;
    }
    if (!checkState(false, error))
    {
        return false;
    }
    writeState(true);
    QJsonArray ids;
    for (const QPointer<IDOSDataObject>& copy : m_copies)
    {
        ids.append(copy->objectId());
    }
    QJsonObject result;
    result.insert(QStringLiteral("objectIds"), ids);
    setResult(result);
    return true;
}

bool IDOSTransferObjectsCommand::revert(QString& error)
{
    if (!checkState(true, error))
    {
        return false;
    }
    writeState(false);
    return true;
}

IDOSTransferObjectsCommandMetadata::IDOSTransferObjectsCommandMetadata(const QString& operation)
    :
    IDOSCommandMetadata(QStringLiteral("objects.") + operation, QObject::tr("Transfer objects"),
        QObject::tr("Copy independent objects or move objects between compatible containers."), IDOSCommand::Type::Action)
    , m_operation(operation)
{
}

QJsonObject IDOSTransferObjectsCommandMetadata::schema() const
{
    QJsonObject properties;
    QJsonObject item;
    item.insert(QStringLiteral("type"), QStringLiteral("object"));
    QJsonObject array;
    array.insert(QStringLiteral("type"), QStringLiteral("array"));
    array.insert(QStringLiteral("items"), item);
    properties.insert(QStringLiteral("items"), array);
    QJsonObject string;
    string.insert(QStringLiteral("type"), QStringLiteral("string"));
    properties.insert(QStringLiteral("targetCaseId"), string);
    properties.insert(QStringLiteral("targetGridId"), string);
    QJsonArray required;
    required.append(QStringLiteral("items"));
    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    schema.insert(QStringLiteral("properties"), properties);
    schema.insert(QStringLiteral("required"), required);
    return schema;
}

IDOSCommand* IDOSTransferObjectsCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSTransferObjectsCommand(project, m_operation, arguments);
}

bool IDOSTransferObjectsCommand::sameReferences(
    const QList<IDOSCaseItemRef>& first, const QList<IDOSCaseItemRef>& second) const
{
    if (first.size() != second.size())
    {
        return false;
    }
    for (int index = 0; index < first.size(); ++index)
    {
        const IDOSCaseItemRef& a = first.at(index);
        const IDOSCaseItemRef& b = second.at(index);
        if (a.refId() != b.refId() || a.role() != b.role() || a.objectId() != b.objectId() ||
            a.partKey() != b.partKey() || a.itemKey() != b.itemKey())
        {
            return false;
        }
    }
    return true;
}
