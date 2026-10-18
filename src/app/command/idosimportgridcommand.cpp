#include <memory>

#include <QFileInfo>
#include <QJsonArray>
#include <QObject>
#include <QSet>
#include <QStringList>

#include "data/case/idoscaseitemref.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationsummaryresultsobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataprovidermetadata.h"
#include "providers/idosdataproviderregistry.h"

#include "idosimportgridcommand.h"

IDOSImportGridCommand::IDOSImportGridCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("grid.import"), Type::Action, project, QObject::tr("Import grid"))
    , m_filePath(arguments.value(QStringLiteral("filePath")).toString())
    , m_loaded(false)
    , m_targetCaseId(arguments.value(QStringLiteral("targetCaseId")).toString())
    , m_replaceExistingGrid(arguments.value(QStringLiteral("replaceExistingGrid")).toBool())
{
}

IDOSImportGridCommand::~IDOSImportGridCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
    for (const QPointer<IDOSDataObject>& object : m_replacedObjects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
}

bool IDOSImportGridCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_filePath.trimmed().isEmpty() || (!m_loaded && !QFileInfo::exists(m_filePath)))
    {
        error = QObject::tr("The import file is unavailable.");
        return false;
    }

    if (m_targetCaseId.isEmpty())
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    IDOSProject* targetProject = project();
    IDOSCaseObject* targetCase =
        qobject_cast<IDOSCaseObject*>(targetProject->object(m_targetCaseId));
    if (targetCase == nullptr)
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    const QList<IDOSCaseItemRef> refs = targetCase->itemRefs();
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.grid"))
        {
            if (!m_replaceExistingGrid)
            {
                error = QObject::tr("The target case already has a grid.");
                return false;
            }
            return true;
        }
    }

    return true;
}

QJsonObject IDOSImportGridCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("targetCaseId"), m_targetCaseId);
    preview.insert(QStringLiteral("replaceExistingGrid"), m_replaceExistingGrid);
    return preview;
}

bool IDOSImportGridCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (!m_loaded && !loadObjects(error))
    {
        return false;
    }

    if (m_replaceExistingGrid)
    {
        captureReplacementObjects();
    }

    captureTargetCaseReferences();
    QSet<QString> importedIds;
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (object.isNull() || object->parent() != nullptr || object->objectId().isEmpty() ||
            targetProject->object(object->objectId()) != nullptr || importedIds.contains(object->objectId()))
        {
            error = QObject::tr("The import target changed or is unavailable.");
            return false;
        }
        importedIds.insert(object->objectId());
    }
    for (const QPointer<IDOSDataObject>& object : m_replacedObjects)
    {
        if (object.isNull() || targetProject->object(object->objectId()) != object.data())
        {
            error = QObject::tr("The import target changed or is unavailable.");
            return false;
        }
    }

    targetProject->beginUpdate();
    if (m_replaceExistingGrid)
    {
        detachReplacementObjects();
    }

    QStringList objectIds;
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            targetProject->add(object.data());
            objectIds.append(object->objectId());
        }
    }
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() != nullptr)
        {
            object->resolveReferences(targetProject);
        }
    }
    appendTargetCaseReferences();

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("filePath"), m_filePath);
    commandResult.insert(QStringLiteral("objectCount"), objectIds.size());
    QJsonArray ids;
    for (const QString& objectId : objectIds)
    {
        ids.append(objectId);
    }
    commandResult.insert(QStringLiteral("objectIds"), ids);
    setResult(commandResult);
    targetProject->endUpdate();
    return true;
}

bool IDOSImportGridCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (object.isNull() || targetProject->object(object->objectId()) != object.data())
        {
            error = QObject::tr("The import target changed or is unavailable.");
            return false;
        }
    }
    if (qobject_cast<IDOSCaseObject*>(targetProject->object(m_targetCaseId)) == nullptr)
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }
    for (const QPointer<IDOSDataObject>& object : m_replacedObjects)
    {
        if (object.isNull() || object->parent() != nullptr || targetProject->object(object->objectId()) != nullptr)
        {
            error = QObject::tr("The import target changed or is unavailable.");
            return false;
        }
    }

    targetProject->beginUpdate();
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() != nullptr)
        {
            targetProject->take(object->objectId());
        }
    }
    restoreTargetCaseReferences();
    restoreReplacementObjects();
    targetProject->endUpdate();
    return true;
}

bool IDOSImportGridCommand::loadObjects(QString& error)
{
    IDOSDataProviderMetadata* metadata = nullptr;
    std::unique_ptr<IDOSDataProvider> provider;
    const QList<IDOSDataProviderMetadata*> metadataList =
        IDOSDataProviderRegistry::instance().metadataForFile(m_filePath);
    if (!metadataList.isEmpty())
    {
        metadata = metadataList.first();
    }
    if (metadata == nullptr)
    {
        error = QObject::tr("No provider matches file: %1").arg(m_filePath);
        return false;
    }
    provider = metadata->create();
    if (provider == nullptr)
    {
        error = QObject::tr("Failed to create provider for: %1").arg(m_filePath);
        return false;
    }

    QList<IDOSDataObject*> objects = provider->read(m_filePath);
    if (objects.isEmpty() || !provider->lastError().isEmpty())
    {
        error = provider->lastError().isEmpty()
            ? QObject::tr("The import file did not contain supported objects.")
            : provider->lastError();
        qDeleteAll(objects);
        return false;
    }

    bool importsSummary = false;
    for (IDOSDataObject* object : objects)
    {
        if (qobject_cast<IDOSSimulationSummaryResultsObject*>(object) != nullptr)
        {
            importsSummary = true;
        }
    }
    const IDOSCaseObject* targetCase =
        qobject_cast<IDOSCaseObject*>(project()->object(m_targetCaseId));
    if (importsSummary && targetCase != nullptr)
    {
        for (const IDOSCaseItemRef& ref : targetCase->itemRefs())
        {
            if (ref.role() == QStringLiteral("case.summaryResults"))
            {
                error = QObject::tr("Summary results have already been imported for this case.");
                qDeleteAll(objects);
                return false;
            }
        }
    }

    for (IDOSDataObject* object : objects)
    {
        if (object == nullptr)
        {
            continue;
        }

        if (qobject_cast<IDOSGridObject*>(object) == nullptr &&
            qobject_cast<IDOSGridPropertyObject*>(object) == nullptr &&
            qobject_cast<IDOSSimulationSummaryResultsObject*>(object) == nullptr)
        {
            delete object;
            continue;
        }

        m_objects.append(QPointer<IDOSDataObject>(object));
    }

    if (m_objects.isEmpty())
    {
        error = QObject::tr("The import file did not contain supported objects.");
        return false;
    }

    m_loaded = true;
    return true;
}

void IDOSImportGridCommand::captureTargetCaseReferences()
{
    m_caseReferences.clear();
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_targetCaseId.isEmpty())
    {
        return;
    }

    IDOSCaseObject* caseObject =
        qobject_cast<IDOSCaseObject*>(targetProject->object(m_targetCaseId));
    if (caseObject != nullptr)
    {
        m_caseReferences.insert(caseObject->objectId(), caseObject->itemRefs());
    }
}

void IDOSImportGridCommand::appendTargetCaseReferences()
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_targetCaseId.isEmpty())
    {
        return;
    }

    IDOSCaseObject* caseObject =
        qobject_cast<IDOSCaseObject*>(targetProject->object(m_targetCaseId));
    if (caseObject == nullptr)
    {
        return;
    }

    QList<IDOSCaseItemRef> refs = caseObject->itemRefs();
    if (m_replaceExistingGrid)
    {
        QSet<QString> replacedObjectIds;
        for (const QPointer<IDOSDataObject>& object : m_replacedObjects)
        {
            if (!object.isNull())
            {
                replacedObjectIds.insert(object->objectId());
            }
        }

        for (int index = refs.size() - 1; index >= 0; --index)
        {
            if (replacedObjectIds.contains(refs.at(index).objectId()))
            {
                refs.removeAt(index);
            }
        }
    }

    QSet<QString> seen;
    for (const IDOSCaseItemRef& ref : refs)
    {
        seen.insert(ref.role() + QLatin1Char(':') + ref.objectId());
    }

    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (object.isNull())
        {
            continue;
        }

        QString role;
        if (qobject_cast<IDOSGridObject*>(object.data()) != nullptr)
        {
            role = QStringLiteral("case.grid");
        }
        else if (qobject_cast<IDOSGridPropertyObject*>(object.data()) != nullptr)
        {
            role = QStringLiteral("case.gridProperty");
        }
        else if (qobject_cast<IDOSSimulationSummaryResultsObject*>(object.data()) != nullptr)
        {
            role = QStringLiteral("case.summaryResults");
        }

        const QString key = role + QLatin1Char(':') + object->objectId();
        if (!role.isEmpty() && !seen.contains(key))
        {
            refs.append(IDOSCaseItemRef(role, object->objectId()));
            seen.insert(key);
        }
    }
    caseObject->setItemRefs(refs);
}

void IDOSImportGridCommand::restoreTargetCaseReferences()
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        return;
    }

    QHash<QString, QList<IDOSCaseItemRef>>::const_iterator iterator =
        m_caseReferences.constBegin();
    while (iterator != m_caseReferences.constEnd())
    {
        IDOSCaseObject* caseObject =
            qobject_cast<IDOSCaseObject*>(targetProject->object(iterator.key()));
        if (caseObject != nullptr)
        {
            caseObject->setItemRefs(iterator.value());
        }
        ++iterator;
    }
}

QString IDOSImportGridCommand::referencedGridId() const
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_targetCaseId.isEmpty())
    {
        return QString();
    }

    IDOSCaseObject* caseObject =
        qobject_cast<IDOSCaseObject*>(targetProject->object(m_targetCaseId));
    if (caseObject == nullptr)
    {
        return QString();
    }

    const QList<IDOSCaseItemRef> refs = caseObject->itemRefs();
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.grid"))
        {
            return ref.objectId();
        }
    }
    return QString();
}

QStringList IDOSImportGridCommand::collectGridPropertyIds(const QString& gridId) const
{
    QStringList propertyIds;
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || gridId.isEmpty())
    {
        return propertyIds;
    }

    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            propertyIds.append(property->objectId());
        }
    }
    return propertyIds;
}

void IDOSImportGridCommand::captureReplacementObjects()
{
    if (!m_replacedObjects.isEmpty())
    {
        return;
    }

    IDOSProject* targetProject = project();
    const QString gridId = referencedGridId();
    if (targetProject == nullptr || gridId.isEmpty())
    {
        return;
    }

    IDOSDataObject* grid = targetProject->object(gridId);
    if (grid != nullptr)
    {
        m_replacedObjects.append(QPointer<IDOSDataObject>(grid));
    }

    const QStringList propertyIds = collectGridPropertyIds(gridId);
    for (const QString& propertyId : propertyIds)
    {
        IDOSDataObject* property = targetProject->object(propertyId);
        if (property != nullptr)
        {
            m_replacedObjects.append(QPointer<IDOSDataObject>(property));
        }
    }
}

void IDOSImportGridCommand::detachReplacementObjects()
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        return;
    }

    for (const QPointer<IDOSDataObject>& object : m_replacedObjects)
    {
        if (!object.isNull() && object->parent() != nullptr)
        {
            targetProject->take(object->objectId());
        }
    }
}

void IDOSImportGridCommand::restoreReplacementObjects()
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        return;
    }

    for (const QPointer<IDOSDataObject>& object : m_replacedObjects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            targetProject->add(object.data());
        }
    }
}

IDOSImportGridCommandMetadata::IDOSImportGridCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("grid.import"),
                          QObject::tr("Import grid"),
                          QObject::tr("Import a grid file into a simulation case."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportGridCommandMetadata::~IDOSImportGridCommandMetadata() = default;

QJsonObject IDOSImportGridCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), stringPropertySchema(QObject::tr("Path of the file to import.")));
    properties.insert(QStringLiteral("targetCaseId"),
                      stringPropertySchema(
                          QObject::tr("Object ID of the target case.")));
    QJsonObject replaceProperty;
    replaceProperty.insert(QStringLiteral("type"), QStringLiteral("boolean"));
    replaceProperty.insert(QStringLiteral("description"),
                           QObject::tr("Replace the existing grid in the target case."));
    properties.insert(QStringLiteral("replaceExistingGrid"), replaceProperty);
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("targetCaseId"));
    return schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportGridCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    return new IDOSImportGridCommand(project, arguments);
}
