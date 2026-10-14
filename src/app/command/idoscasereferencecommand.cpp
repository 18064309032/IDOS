#include <memory>

#include <QJsonArray>
#include <QJsonDocument>
#include <QPointer>
#include <QSet>
#include <QSignalBlocker>
#include <QStringList>
#include <QUuid>

#include "data/case/idossimulationcaseobject.h"
#include "data/fluidrock/idosfluidrocktable.h"
#include "data/well/idoswell.h"
#include "idosproject.h"

#include "idoscasereferencecommand.h"

IDOSCaseReferenceCommand::IDOSCaseReferenceCommand(IDOSProject* project, const QString& operation,
                                                   const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("case.references.") + operation, Type::Action, project,
                QObject::tr("Change case references"))
    , m_operation(operation)
    , m_arguments(arguments)
    , m_initialized(false)
{
}

bool IDOSCaseReferenceCommand::matches(const IDOSCaseItemRef& reference, const QJsonObject& item)
{
    return reference.objectId() == item.value(QStringLiteral("objectId")).toString() &&
           reference.role() == item.value(QStringLiteral("role")).toString() &&
           reference.partKey() == item.value(QStringLiteral("partKey")).toString() &&
           reference.itemKey() == item.value(QStringLiteral("itemKey")).toString() &&
           (item.value(QStringLiteral("refId")).toString().isEmpty() ||
            reference.refId() == item.value(QStringLiteral("refId")).toString());
}

bool IDOSCaseReferenceCommand::prepare(QHash<QString, QList<IDOSCaseItemRef>>& before,
                                      QHash<QString, QList<IDOSCaseItemRef>>& after, QString& error) const
{
    if (m_operation != QStringLiteral("copy") && m_operation != QStringLiteral("move") &&
        m_operation != QStringLiteral("remove"))
    {
        error = QObject::tr("Unsupported reference operation.");
        return false;
    }
    const QJsonArray items = m_arguments.value(QStringLiteral("items")).toArray();
    if (items.isEmpty())
    {
        error = QObject::tr("Select at least one well or table reference.");
        return false;
    }
    const QString targetId = m_arguments.value(QStringLiteral("targetCaseId")).toString();
    if (m_operation != QStringLiteral("remove"))
    {
        IDOSSimulationCaseObject* target = qobject_cast<IDOSSimulationCaseObject*>(project()->object(targetId));
        if (target == nullptr)
        {
            error = QObject::tr("The target simulation case is unavailable.");
            return false;
        }
        before.insert(targetId, target->itemRefs());
        after.insert(targetId, target->itemRefs());
    }
    QSet<QByteArray> sources;
    for (const QJsonValue& value : items)
    {
        const QJsonObject item = value.toObject();
        const QString objectId = item.value(QStringLiteral("objectId")).toString();
        const QString sourceId = item.value(QStringLiteral("sourceCaseId")).toString();
        const QString role = item.value(QStringLiteral("role")).toString();
        IDOSDataObject* object = project()->object(objectId);
        if (!((role == QStringLiteral("case.well") && qobject_cast<IDOSWell*>(object) != nullptr) ||
              (role == QStringLiteral("case.fluidRockTable") && qobject_cast<IDOSFluidRockTable*>(object) != nullptr)))
        {
            error = QObject::tr("Only existing wells and fluid or rock tables can be referenced.");
            return false;
        }
        QJsonObject identity;
        identity.insert(QStringLiteral("objectId"), objectId);
        identity.insert(QStringLiteral("sourceCaseId"), sourceId);
        identity.insert(QStringLiteral("role"), role);
        identity.insert(QStringLiteral("partKey"), item.value(QStringLiteral("partKey")).toString());
        identity.insert(QStringLiteral("itemKey"), item.value(QStringLiteral("itemKey")).toString());
        identity.insert(QStringLiteral("refId"), item.value(QStringLiteral("refId")).toString());
        const QByteArray key = QJsonDocument(identity).toJson(QJsonDocument::Compact);
        if (sources.contains(key))
        {
            error = QObject::tr("The selection contains a duplicate reference.");
            return false;
        }
        sources.insert(key);
        IDOSCaseItemRef reference(role, objectId);
        if (!sourceId.isEmpty())
        {
            IDOSSimulationCaseObject* source = qobject_cast<IDOSSimulationCaseObject*>(project()->object(sourceId));
            if (source == nullptr || (sourceId == targetId && m_operation != QStringLiteral("remove")))
            {
                error = QObject::tr("Choose a different, available target case.");
                return false;
            }
            int found = -1;
            const QList<IDOSCaseItemRef> references = source->itemRefs();
            for (int index = 0; index < references.size(); ++index)
            {
                if (matches(references.at(index), item))
                {
                    if (found >= 0)
                    {
                        error = QObject::tr("The source reference is ambiguous.");
                        return false;
                    }
                    found = index;
                }
            }
            if (found < 0)
            {
                error = QObject::tr("The source reference is no longer available.");
                return false;
            }
            reference = references.at(found);
            if (m_operation != QStringLiteral("copy"))
            {
                if (!before.contains(sourceId))
                {
                    before.insert(sourceId, references);
                    after.insert(sourceId, references);
                }
                QList<IDOSCaseItemRef>& remaining = after[sourceId];
                int removeIndex = -1;
                for (int index = 0; index < remaining.size(); ++index)
                {
                    if (matches(remaining.at(index), item))
                    {
                        removeIndex = index;
                        break;
                    }
                }
                if (removeIndex < 0)
                {
                    error = QObject::tr("The selection contains overlapping references.");
                    return false;
                }
                remaining.removeAt(removeIndex);
            }
        }
        else if (m_operation != QStringLiteral("copy") ||
                 !item.value(QStringLiteral("partKey")).toString().isEmpty() ||
                 !item.value(QStringLiteral("itemKey")).toString().isEmpty() ||
                 !item.value(QStringLiteral("refId")).toString().isEmpty())
        {
            error = QObject::tr("Cut and remove require an existing case reference.");
            return false;
        }
        if (m_operation != QStringLiteral("remove"))
        {
            for (const IDOSCaseItemRef& existing : after.value(targetId))
            {
                if (existing.objectId() == reference.objectId() && existing.role() == reference.role() &&
                    existing.partKey() == reference.partKey() && existing.itemKey() == reference.itemKey())
                {
                    error = QObject::tr("The target case already contains this reference.");
                    return false;
                }
            }
            if (m_operation == QStringLiteral("copy") || reference.refId().isEmpty())
            {
                reference.setRefId(QUuid::createUuid().toString(QUuid::WithoutBraces));
            }
            after[targetId].append(reference);
        }
    }
    return true;
}

bool IDOSCaseReferenceCommand::sameList(const QList<IDOSCaseItemRef>& first,
                                      const QList<IDOSCaseItemRef>& second) const
{
    if (first.size() != second.size())
    {
        return false;
    }
    for (int index = 0; index < first.size(); ++index)
    {
        const IDOSCaseItemRef& left = first.at(index);
        const IDOSCaseItemRef& right = second.at(index);
        if (left.refId() != right.refId() || left.role() != right.role() ||
            left.objectId() != right.objectId() || left.partKey() != right.partKey() ||
            left.itemKey() != right.itemKey())
        {
            return false;
        }
    }
    return true;
}

bool IDOSCaseReferenceCommand::checkState(const QHash<QString, QList<IDOSCaseItemRef>>& expected,
                                        QString& error) const
{
    for (const QJsonValue& value : m_arguments.value(QStringLiteral("items")).toArray())
    {
        if (!supportsObject(value.toObject()))
        {
            error = QObject::tr("The referenced well or table is no longer available.");
            return false;
        }
    }
    for (QHash<QString, QList<IDOSCaseItemRef>>::const_iterator it = expected.constBegin(); it != expected.constEnd(); ++it)
    {
        IDOSSimulationCaseObject* object = qobject_cast<IDOSSimulationCaseObject*>(project()->object(it.key()));
        if (object == nullptr || !sameList(object->itemRefs(), it.value()))
        {
            error = QObject::tr("Case references changed outside this command.");
            return false;
        }
    }
    return true;
}

bool IDOSCaseReferenceCommand::supportsObject(const QJsonObject& item) const
{
    IDOSDataObject* object = project()->object(item.value(QStringLiteral("objectId")).toString());
    const QString role = item.value(QStringLiteral("role")).toString();
    return (role == QStringLiteral("case.well") && qobject_cast<IDOSWell*>(object) != nullptr) ||
           (role == QStringLiteral("case.fluidRockTable") && qobject_cast<IDOSFluidRockTable*>(object) != nullptr);
}

bool IDOSCaseReferenceCommand::validateCommand(QString& error) const
{
    if (m_initialized)
    {
        return checkState(m_before, error);
    }
    QHash<QString, QList<IDOSCaseItemRef>> before;
    QHash<QString, QList<IDOSCaseItemRef>> after;
    return prepare(before, after, error);
}

bool IDOSCaseReferenceCommand::writeState(const QHash<QString, QList<IDOSCaseItemRef>>& state, QString& error)
{
    QList<QPointer<IDOSSimulationCaseObject>> objects;
    QList<std::shared_ptr<QSignalBlocker>> blockers;
    for (QHash<QString, QList<IDOSCaseItemRef>>::const_iterator it = state.constBegin(); it != state.constEnd(); ++it)
    {
        IDOSSimulationCaseObject* object = qobject_cast<IDOSSimulationCaseObject*>(project()->object(it.key()));
        if (object == nullptr)
        {
            error = QObject::tr("The simulation case is unavailable.");
            return false;
        }
        objects.append(object);
        blockers.append(std::make_shared<QSignalBlocker>(object));
    }
    // Install all lists before publishing any business notification. No fallible
    // operation remains after preflight; observers always see both sides updated.
    for (const QPointer<IDOSSimulationCaseObject>& object : objects)
    {
        object->setItemRefs(state.value(object->objectId()));
    }
    blockers.clear();
    project()->beginUpdate();
    for (const QPointer<IDOSSimulationCaseObject>& object : objects)
    {
        if (object != nullptr)
        {
            emit object->dataChanged();
        }
    }
    project()->endUpdate();
    return true;
}

bool IDOSCaseReferenceCommand::apply(QString& error)
{
    if (!m_initialized)
    {
        if (!prepare(m_before, m_after, error))
        {
            return false;
        }
        m_initialized = true;
    }
    if (!checkState(m_before, error) || !writeState(m_after, error))
    {
        return false;
    }
    QJsonObject result;
    const QJsonArray items = m_arguments.value(QStringLiteral("items")).toArray();
    result.insert(QStringLiteral("count"), items.size());
    result.insert(QStringLiteral("targetCaseId"), m_arguments.value(QStringLiteral("targetCaseId")));
    if (items.size() == 1 && items.first().toObject().value(QStringLiteral("role")).toString() == QStringLiteral("case.fluidRockTable"))
    {
        result.insert(QStringLiteral("tableId"), items.first().toObject().value(QStringLiteral("objectId")));
    }
    setResult(result);
    return true;
}

bool IDOSCaseReferenceCommand::revert(QString& error)
{
    return checkState(m_after, error) && writeState(m_before, error);
}

IDOSCaseReferenceCommandMetadata::IDOSCaseReferenceCommandMetadata(const QString& operation)
    :
    IDOSCommandMetadata(QStringLiteral("case.references.") + operation,
                        QObject::tr("Change case references"),
                        QObject::tr("Copy, move or remove well and table references atomically."),
                        IDOSCommand::Type::Action)
    , m_operation(operation)
{
}

QJsonObject IDOSCaseReferenceCommandMetadata::schema() const
{
    QJsonObject itemProperties;
    for (const QString& name : QStringList{QStringLiteral("objectId"), QStringLiteral("role"),
                                         QStringLiteral("sourceCaseId"), QStringLiteral("refId"),
                                         QStringLiteral("partKey"), QStringLiteral("itemKey")})
    {
        itemProperties.insert(name, QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}});
    }
    QJsonObject itemSchema;
    itemSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
    itemSchema.insert(QStringLiteral("properties"), itemProperties);
    itemSchema.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("objectId"), QStringLiteral("role")});
    QJsonObject properties;
    properties.insert(QStringLiteral("items"), QJsonObject{{QStringLiteral("type"), QStringLiteral("array")},
                                                         {QStringLiteral("items"), itemSchema}});
    properties.insert(QStringLiteral("targetCaseId"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}});
    QJsonArray required{QStringLiteral("items")};
    if (m_operation != QStringLiteral("remove"))
    {
        required.append(QStringLiteral("targetCaseId"));
    }
    return QJsonObject{{QStringLiteral("type"), QStringLiteral("object")},
                       {QStringLiteral("properties"), properties}, {QStringLiteral("required"), required}};
}

IDOSCommand* IDOSCaseReferenceCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSCaseReferenceCommand(project, m_operation, arguments);
}
