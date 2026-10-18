#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>

#include "data/case/idoscaseobject.h"
#include "data/fluidrock/idosfluidrocktableobject.h"
#include "data/fracture/idosfracturematerialcatalogobject.h"
#include "data/fracture/idosfracturescheduleobject.h"
#include "data/fracture/idosnaturalfracturesetobject.h"
#include "data/grid/idosgridboundaryobject.h"
#include "data/grid/idosgridsurfaceobject.h"
#include "data/production/idosproductionhistoryobject.h"
#include "data/seismic/idosmicroseismicsetobject.h"
#include "data/seismic/idosseismicvolumeobject.h"
#include "idosproject.h"

#include "idosdeletedatacommand.h"

IDOSDeleteDataCommand::IDOSDeleteDataCommand(IDOSProject* project, const QStringList& objectIds)
    :
    IDOSCommand(QStringLiteral("data.delete"), Type::Action, project, QObject::tr("Delete data"))
    , m_objectIds(objectIds)
{
}

IDOSDeleteDataCommand::~IDOSDeleteDataCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
}

bool IDOSDeleteDataCommand::supportsObject(const IDOSDataObject* object)
{
    return qobject_cast<const IDOSNaturalFractureSetObject*>(object) != nullptr
        || qobject_cast<const IDOSMicroseismicSetObject*>(object) != nullptr
        || qobject_cast<const IDOSSeismicVolumeObject*>(object) != nullptr
        || qobject_cast<const IDOSFractureScheduleObject*>(object) != nullptr
        || qobject_cast<const IDOSFractureMaterialCatalogObject*>(object) != nullptr
        || qobject_cast<const IDOSProductionHistoryObject*>(object) != nullptr
        || qobject_cast<const IDOSGridSurfaceObject*>(object) != nullptr
        || qobject_cast<const IDOSGridBoundaryObject*>(object) != nullptr
        || qobject_cast<const IDOSFluidRockTableObject*>(object) != nullptr;
}

bool IDOSDeleteDataCommand::validateCommand(QString& error) const
{
    if (project() == nullptr || m_objectIds.isEmpty())
    {
        error = QObject::tr("No data is selected for deletion.");
        return false;
    }
    QSet<QString> unique;
    for (const QString& id : m_objectIds)
    {
        if (unique.contains(id) || !supportsObject(project()->object(id)))
        {
            error = QObject::tr("The selected data is unavailable or requires its own delete command.");
            return false;
        }
        unique.insert(id);
    }
    return true;
}

QJsonObject IDOSDeleteDataCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("objectIds"), QJsonArray::fromStringList(m_objectIds));
    return preview;
}

bool IDOSDeleteDataCommand::apply(QString& error)
{
    if (!validateCommand(error))
    {
        return false;
    }
    m_caseReferences.clear();
    m_objects.clear();
    project()->beginUpdate();
    for (IDOSDataObject* object : project()->objects())
    {
        IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(object);
        if (caseObject == nullptr)
        {
            continue;
        }
        QList<IDOSCaseItemRef> refs = caseObject->itemRefs();
        const QList<IDOSCaseItemRef> before = refs;
        for (int index = refs.size() - 1; index >= 0; --index)
        {
            if (m_objectIds.contains(refs.at(index).objectId()))
            {
                refs.removeAt(index);
            }
        }
        if (refs.size() != before.size())
        {
            m_caseReferences.insert(caseObject->objectId(), before);
            caseObject->setItemRefs(refs);
        }
    }
    for (const QString& id : m_objectIds)
    {
        IDOSDataObject* object = project()->take(id);
        if (object == nullptr)
        {
            for (const QPointer<IDOSDataObject>& detached : m_objects)
            {
                project()->add(detached.data());
            }
            restoreReferences();
            project()->endUpdate();
            m_objects.clear();
            error = QObject::tr("The selected data is unavailable or requires its own delete command.");
            return false;
        }
        m_objects.append(object);
    }
    project()->endUpdate();
    setResult(buildPreview());
    return true;
}

bool IDOSDeleteDataCommand::revert(QString& error)
{
    if (project() == nullptr || m_objects.size() != m_objectIds.size())
    {
        error = QObject::tr("The deleted data cannot be restored.");
        return false;
    }
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (object.isNull() || object->parent() != nullptr || project()->object(object->objectId()) != nullptr)
        {
            error = QObject::tr("The deleted data cannot be restored.");
            return false;
        }
    }
    project()->beginUpdate();
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        project()->add(object.data());
    }
    restoreReferences();
    project()->endUpdate();
    return true;
}

void IDOSDeleteDataCommand::restoreReferences()
{
    QHash<QString, QList<IDOSCaseItemRef>>::const_iterator iterator = m_caseReferences.constBegin();
    while (iterator != m_caseReferences.constEnd())
    {
        IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(project()->object(iterator.key()));
        if (caseObject != nullptr)
        {
            caseObject->setItemRefs(iterator.value());
        }
        ++iterator;
    }
}

IDOSDeleteDataCommandMetadata::IDOSDeleteDataCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("data.delete"), QObject::tr("Delete data"),
        QObject::tr("Delete imported data and its case references."), IDOSCommand::Type::Action)
{
}

QJsonObject IDOSDeleteDataCommandMetadata::schema() const
{
    QJsonObject item;
    item.insert(QStringLiteral("type"), QStringLiteral("string"));
    QJsonObject ids;
    ids.insert(QStringLiteral("type"), QStringLiteral("array"));
    ids.insert(QStringLiteral("items"), item);
    ids.insert(QStringLiteral("minItems"), 1);
    QJsonObject properties;
    properties.insert(QStringLiteral("objectIds"), ids);
    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    schema.insert(QStringLiteral("properties"), properties);
    schema.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("objectIds")});
    return schema;
}

IDOSCommand* IDOSDeleteDataCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    QStringList ids;
    for (const QJsonValue& value : arguments.value(QStringLiteral("objectIds")).toArray())
    {
        ids.append(value.toString());
    }
    return new IDOSDeleteDataCommand(project, ids);
}
