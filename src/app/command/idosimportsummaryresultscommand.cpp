#include <memory>

#include <QFileInfo>
#include <QJsonArray>
#include <QObject>
#include <QSet>
#include <QStringList>

#include "data/case/idoscaseitemref.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/case/idossimulationsummaryresultsobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataprovidermetadata.h"
#include "providers/idosdataproviderregistry.h"

#include "idosimportsummaryresultscommand.h"

IDOSImportSummaryResultsCommand::IDOSImportSummaryResultsCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("case.summary.import"), Type::Action, project, QObject::tr("Import summary results"))
    , m_filePath(arguments.value(QStringLiteral("filePath")).toString())
    , m_loaded(false)
    , m_targetCaseId(arguments.value(QStringLiteral("targetCaseId")).toString())
{
}

IDOSImportSummaryResultsCommand::~IDOSImportSummaryResultsCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
}

bool IDOSImportSummaryResultsCommand::validateCommand(QString& error) const
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
    if (qobject_cast<IDOSSimulationCaseObject*>(targetCase) == nullptr)
    {
        error = QObject::tr("The target simulation case is unavailable.");
        return false;
    }
    for (const IDOSCaseItemRef& ref : targetCase->itemRefs())
    {
        if (ref.role() == QStringLiteral("case.summaryResults"))
        {
            error = QObject::tr("Summary results have already been imported for this case.");
            return false;
        }
    }

    return true;
}

QJsonObject IDOSImportSummaryResultsCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("targetCaseId"), m_targetCaseId);
    return preview;
}

bool IDOSImportSummaryResultsCommand::apply(QString& error)
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

    targetProject->beginUpdate();

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

bool IDOSImportSummaryResultsCommand::revert(QString& error)
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

    targetProject->beginUpdate();
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() != nullptr)
        {
            targetProject->take(object->objectId());
        }
    }
    restoreTargetCaseReferences();
    targetProject->endUpdate();
    return true;
}

bool IDOSImportSummaryResultsCommand::loadObjects(QString& error)
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

    for (IDOSDataObject* object : objects)
    {
        if (object == nullptr)
        {
            continue;
        }

        if (qobject_cast<IDOSSimulationSummaryResultsObject*>(object) == nullptr)
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

void IDOSImportSummaryResultsCommand::captureTargetCaseReferences()
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

void IDOSImportSummaryResultsCommand::appendTargetCaseReferences()
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

        const QString role = QStringLiteral("case.summaryResults");

        const QString key = role + QLatin1Char(':') + object->objectId();
        if (!role.isEmpty() && !seen.contains(key))
        {
            refs.append(IDOSCaseItemRef(role, object->objectId()));
            seen.insert(key);
        }
    }
    caseObject->setItemRefs(refs);
}

void IDOSImportSummaryResultsCommand::restoreTargetCaseReferences()
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

IDOSImportSummaryResultsCommandMetadata::IDOSImportSummaryResultsCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("case.summary.import"),
                          QObject::tr("Import summary results"),
                          QObject::tr("Import ECLIPSE summary vectors into a simulation case."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportSummaryResultsCommandMetadata::~IDOSImportSummaryResultsCommandMetadata() = default;

QJsonObject IDOSImportSummaryResultsCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), stringPropertySchema(QObject::tr("Path of the file to import.")));
    properties.insert(QStringLiteral("targetCaseId"),
                      stringPropertySchema(
                          QObject::tr("Object ID of the target simulation case.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("targetCaseId"));
    return schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportSummaryResultsCommandMetadata::create(const QJsonObject& arguments,
                                                               IDOSProject* project) const
{
    return new IDOSImportSummaryResultsCommand(project, arguments);
}
