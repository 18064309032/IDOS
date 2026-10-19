#include <memory>

#include <QDebug>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonArray>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVariantList>

#include "data/case/idoscaseitemref.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/fluidrock/idosfluidrocktableobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataprovidermetadata.h"
#include "providers/idosdataproviderregistry.h"

#include "idosimportcasecommand.h"

IDOSImportCaseCommand::IDOSImportCaseCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("case.import"), Type::Action, project, QObject::tr("Import case"))
    , m_filePath(arguments.value(QStringLiteral("filePath")).toString())
    , m_loaded(false)
    , m_caseNameOverride(arguments.value(QStringLiteral("caseName")).toString())
{
}

IDOSImportCaseCommand::~IDOSImportCaseCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
}

bool IDOSImportCaseCommand::validateCommand(QString& error) const
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

    return true;
}

QJsonObject IDOSImportCaseCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("caseName"), m_caseNameOverride);
    return preview;
}

bool IDOSImportCaseCommand::apply(QString& error)
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
    targetProject = project();
    if (targetProject == nullptr || !validateCommand(error))
    {
        error = error.isEmpty() ? QObject::tr("The target project is unavailable.") : error;
        return false;
    }

    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        IDOSCaseObject* importedCase = qobject_cast<IDOSCaseObject*>(object.data());
        if (importedCase == nullptr)
        {
            continue;
        }
        for (IDOSDataObject* existingObject : targetProject->objects())
        {
            IDOSCaseObject* existingCase = qobject_cast<IDOSCaseObject*>(existingObject);
            if (existingCase != nullptr
                && existingCase->name().trimmed().compare(importedCase->name().trimmed(),
                                                          Qt::CaseInsensitive) == 0)
            {
                error = QObject::tr("A case with this name already exists.");
                return false;
            }
        }
    }

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

    QElapsedTimer commitTimer;
    commitTimer.start();
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
    if (qEnvironmentVariableIsSet("IDOS_IMPORT_PROFILE"))
    {
        qInfo("IMPORT_PROFILE phase=project-commit elapsed_ms=%.3f", commitTimer.nsecsElapsed() / 1000000.0);
    }
    return true;
}

bool IDOSImportCaseCommand::revert(QString& error)
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

    targetProject->beginUpdate();
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() != nullptr)
        {
            targetProject->take(object->objectId());
        }
    }
    targetProject->endUpdate();
    return true;
}

bool IDOSImportCaseCommand::loadObjects(QString& error)
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

    QList<IDOSDataObject*> objects = provider->readInBackground(m_filePath);
    if (project() == nullptr || objects.isEmpty() || !provider->lastError().isEmpty())
    {
        error = provider->lastError().isEmpty()
            ? QObject::tr("The import file did not contain supported objects.")
            : provider->lastError();
        qDeleteAll(objects);
        return false;
    }

    if (!adoptParsedObjects(objects))
    {
        qDeleteAll(objects);
        error = QObject::tr("The import file did not contain supported objects.");
        return false;
    }
    return true;
}

bool IDOSImportCaseCommand::adoptParsedObjects(QList<IDOSDataObject*>& objects)
{
    if (m_loaded || project() == nullptr || objects.isEmpty())
    {
        return false;
    }
    QSet<IDOSDataObject*> unique;
    for (IDOSDataObject* object : objects)
    {
        if (object == nullptr || object->parent() != nullptr || object->thread() != project()->thread() || unique.contains(object))
        {
            return false;
        }
        unique.insert(object);
    }
    for (IDOSDataObject* object : objects)
    {
        prepareObject(object);
        m_objects.append(QPointer<IDOSDataObject>(object));
    }
    objects.clear();
    m_loaded = true;
    return true;
}

void IDOSImportCaseCommand::prepareObject(IDOSDataObject* object)
{
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(object);
    if (caseObject != nullptr && !m_caseNameOverride.trimmed().isEmpty())
    {
        caseObject->setName(m_caseNameOverride.trimmed());
    }

    IDOSSimulationCaseObject* simulationCase =
        qobject_cast<IDOSSimulationCaseObject*>(caseObject);
    if (simulationCase != nullptr)
    {
        const QVariantList curveTables = simulationCase->curveTables();
        QMap<QString, int> tableIndices;
        for (IDOSDataObject* existingObject : project()->objects())
        {
            IDOSFluidRockTableObject* existingTable = qobject_cast<IDOSFluidRockTableObject*>(existingObject);
            if (existingTable != nullptr)
            {
                const QVariantMap tableData = existingTable->tableData();
                const QString keyword = tableData.value(QStringLiteral("keyword")).toString().toUpper();
                tableIndices.insert(keyword, qMax(tableIndices.value(keyword),
                                                  tableData.value(QStringLiteral("tableIndex")).toInt()));
            }
        }
        QList<IDOSCaseItemRef> refs = simulationCase->itemRefs();
        int createdTableCount = 0;
        for (const QVariant& tableValue : curveTables)
        {
            QVariantMap tableData = tableValue.toMap();
            const QString keyword = tableData.value(QStringLiteral("keyword")).toString().toUpper();
            if (keyword.isEmpty())
            {
                continue;
            }
            const int tableIndex = tableIndices.value(keyword) + 1;
            tableIndices.insert(keyword, tableIndex);
            tableData.insert(QStringLiteral("keyword"), keyword);
            tableData.insert(QStringLiteral("tableIndex"), tableIndex);
            tableData.insert(QStringLiteral("originCaseId"), simulationCase->objectId());
            const IDOSFluidRockTableObject::Category category = isFluidKeyword(keyword)
                ? IDOSFluidRockTableObject::Category::Fluid : IDOSFluidRockTableObject::Category::RockPhysics;
            IDOSFluidRockTableObject* table = new IDOSFluidRockTableObject(category);
            table->setTableData(tableData);
            table->setSourceFile(m_filePath);
            table->setName(QObject::tr("%1 — Table %2").arg(keyword).arg(tableIndex));
            m_objects.append(QPointer<IDOSDataObject>(table));
            refs.append(IDOSCaseItemRef(QStringLiteral("case.fluidRockTable"), table->objectId()));
            ++createdTableCount;
        }
        if (createdTableCount > 0)
        {
            simulationCase->setCurveTables(QVariantList());
            simulationCase->setItemRefs(refs);
        }
    }

}

bool IDOSImportCaseCommand::isFluidKeyword(const QString& keyword) const
{
    const QStringList fluidKeywords = {
        QStringLiteral("DENSITY"), QStringLiteral("PVTO"), QStringLiteral("PVDO"),
        QStringLiteral("PVDG"), QStringLiteral("PVTG"), QStringLiteral("PVZG"),
        QStringLiteral("PVTW")
    };
    return fluidKeywords.contains(keyword.toUpper());
}

IDOSImportCaseCommandMetadata::IDOSImportCaseCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("case.import"),
                          QObject::tr("Import case"),
                          QObject::tr("Import a simulation case file into the current project."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportCaseCommandMetadata::~IDOSImportCaseCommandMetadata() = default;

QJsonObject IDOSImportCaseCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), stringPropertySchema(QObject::tr("Path of the file to import.")));
    properties.insert(QStringLiteral("caseName"),
                      stringPropertySchema(
                          QObject::tr("Optional imported case name.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    return schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportCaseCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    return new IDOSImportCaseCommand(project, arguments);
}
