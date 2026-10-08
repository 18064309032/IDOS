#include <memory>
#include <utility>

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>

#include "idoscaseitemref.h"
#include "idoscaseobject.h"
#include "idosdataprovider.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idosprovidermetadata.h"
#include "idosproviderregistry.h"
#include "idossimulationcaseobject.h"
#include "idoswell.h"

#include "command/idosimportcommands.h"

class IDOSImportCommandJsonHelpers
{
public:
    static QJsonObject filePathSchema();
    static QJsonObject stringPropertySchema(const QString& description);
    static QJsonObject schemaWithProperties(const QJsonObject& properties,
                                            const QJsonArray& required);
};

IDOSImportObjectsCommand::IDOSImportObjectsCommand(IDOSProject* project,
                                                   QString name,
                                                   QString filePath,
                                                   Mode mode,
                                                   QString targetGridId,
                                                   QString targetCaseId,
                                                   QString caseNameOverride,
                                                   bool replaceExistingGrid,
                                                   QUndoCommand* parent)
    : IDOSCommand(std::move(name),
                  Type::Action,
                  project,
                  QObject::tr("Import data"),
                  parent)
    , m_filePath(std::move(filePath))
    , m_mode(mode)
    , m_targetGridId(std::move(targetGridId))
    , m_targetCaseId(std::move(targetCaseId))
    , m_caseNameOverride(std::move(caseNameOverride))
    , m_replaceExistingGrid(replaceExistingGrid)
    , m_loaded(false)
{
}

IDOSImportObjectsCommand::~IDOSImportObjectsCommand()
{
    deleteDetachedObjects();
}

bool IDOSImportObjectsCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_filePath.trimmed().isEmpty() || !QFileInfo::exists(m_filePath))
    {
        error = QObject::tr("The import file is unavailable.");
        return false;
    }

    if ((m_mode == Mode::Grid || m_mode == Mode::Property) && m_targetCaseId.isEmpty())
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    if (m_mode == Mode::Property && m_targetGridId.isEmpty())
    {
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    IDOSProject* targetProject = project();
    if (m_mode == Mode::Grid || m_mode == Mode::Property)
    {
        IDOSCaseObject* targetCase =
            qobject_cast<IDOSCaseObject*>(targetProject->objectById(m_targetCaseId));
        if (targetCase == nullptr)
        {
            error = QObject::tr("The target case is unavailable.");
            return false;
        }

        if (m_mode == Mode::Grid)
        {
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
        }
    }

    if (m_mode == Mode::Property && targetProject->objectById(m_targetGridId) == nullptr)
    {
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    return true;
}

QJsonObject IDOSImportObjectsCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("targetGridId"), m_targetGridId);
    preview.insert(QStringLiteral("targetCaseId"), m_targetCaseId);
    preview.insert(QStringLiteral("caseName"), m_caseNameOverride);
    preview.insert(QStringLiteral("replaceExistingGrid"), m_replaceExistingGrid);
    return preview;
}

bool IDOSImportObjectsCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (!ensureLoaded(error))
    {
        return false;
    }

    if (m_mode == Mode::Grid && m_replaceExistingGrid)
    {
        captureReplacementObjects();
    }

    if (m_mode == Mode::Case)
    {
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
    }

    if (m_mode == Mode::WellData && hasDuplicateWell(error))
    {
        return false;
    }

    captureTargetCaseReferences();
    IDOSProjectUpdateGuard updateGuard(targetProject);
    if (m_mode == Mode::Grid && m_replaceExistingGrid)
    {
        detachReplacementObjects();
    }

    QStringList objectIds;
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            targetProject->addObject(object.data());
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
    return true;
}

bool IDOSImportObjectsCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    IDOSProjectUpdateGuard updateGuard(targetProject);
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() != nullptr)
        {
            targetProject->takeObject(object->objectId());
        }
    }
    restoreTargetCaseReferences();
    restoreReplacementObjects();
    return true;
}

bool IDOSImportObjectsCommand::ensureLoaded(QString& error)
{
    if (m_loaded)
    {
        return true;
    }

    return loadObjects(error);
}

bool IDOSImportObjectsCommand::loadObjects(QString& error)
{
    QList<IDOSProviderMetadata*> metadataList =
        IDOSProviderRegistry::instance().metadataForFile(m_filePath);
    if (metadataList.isEmpty())
    {
        error = QObject::tr("No provider matches file: %1").arg(m_filePath);
        return false;
    }

    std::unique_ptr<IDOSDataProvider> provider = metadataList.first()->createProvider();
    if (provider == nullptr)
    {
        error = QObject::tr("Failed to create provider for: %1").arg(m_filePath);
        return false;
    }

    QList<IDOSDataObject*> objects = provider->read(m_filePath);
    if (objects.isEmpty())
    {
        error = provider->lastError();
        return false;
    }

    for (IDOSDataObject* object : objects)
    {
        if (object == nullptr)
        {
            continue;
        }

        if (!acceptObject(object))
        {
            delete object;
            continue;
        }

        prepareObject(object);
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

bool IDOSImportObjectsCommand::acceptObject(IDOSDataObject* object) const
{
    if (m_mode == Mode::Case)
    {
        return object != nullptr;
    }

    if (m_mode == Mode::Grid)
    {
        return qobject_cast<IDOSGrid*>(object) != nullptr
            || qobject_cast<IDOSGridProperty*>(object) != nullptr;
    }

    if (m_mode == Mode::Property)
    {
        return qobject_cast<IDOSGridProperty*>(object) != nullptr;
    }

    if (m_mode == Mode::WellData)
    {
        return qobject_cast<IDOSWell*>(object) != nullptr;
    }

    return false;
}

void IDOSImportObjectsCommand::prepareObject(IDOSDataObject* object)
{
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(object);
    if (m_mode == Mode::Case && caseObject != nullptr && !m_caseNameOverride.trimmed().isEmpty())
    {
        caseObject->setName(m_caseNameOverride.trimmed());
    }

    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
    if (m_mode == Mode::Property && property != nullptr)
    {
        property->setGridId(m_targetGridId);
    }
}

bool IDOSImportObjectsCommand::hasDuplicateWell(QString& error) const
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return true;
    }

    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        IDOSWell* importedWell = qobject_cast<IDOSWell*>(object.data());
        if (importedWell == nullptr)
        {
            continue;
        }

        for (IDOSDataObject* existingObject : targetProject->objects())
        {
            IDOSWell* existingWell = qobject_cast<IDOSWell*>(existingObject);
            if (existingWell != nullptr
                && existingWell->name().trimmed().compare(importedWell->name().trimmed(),
                                                          Qt::CaseInsensitive) == 0)
            {
                error = QObject::tr("A well named \"%1\" already exists. Well data merge is not undoable yet.")
                            .arg(importedWell->name());
                return true;
            }
        }
    }

    return false;
}

void IDOSImportObjectsCommand::captureTargetCaseReferences()
{
    m_caseReferences.clear();
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_targetCaseId.isEmpty())
    {
        return;
    }

    IDOSCaseObject* caseObject =
        qobject_cast<IDOSCaseObject*>(targetProject->objectById(m_targetCaseId));
    if (caseObject != nullptr)
    {
        m_caseReferences.insert(caseObject->objectId(), caseObject->itemRefs());
    }
}

void IDOSImportObjectsCommand::appendTargetCaseReferences()
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_targetCaseId.isEmpty())
    {
        return;
    }

    IDOSCaseObject* caseObject =
        qobject_cast<IDOSCaseObject*>(targetProject->objectById(m_targetCaseId));
    if (caseObject == nullptr)
    {
        return;
    }

    QList<IDOSCaseItemRef> refs = caseObject->itemRefs();
    if (m_mode == Mode::Grid && m_replaceExistingGrid)
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
        if (qobject_cast<IDOSGrid*>(object.data()) != nullptr)
        {
            role = QStringLiteral("case.grid");
        }
        else if (qobject_cast<IDOSGridProperty*>(object.data()) != nullptr)
        {
            role = QStringLiteral("case.gridProperty");
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

void IDOSImportObjectsCommand::restoreTargetCaseReferences()
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
            qobject_cast<IDOSCaseObject*>(targetProject->objectById(iterator.key()));
        if (caseObject != nullptr)
        {
            caseObject->setItemRefs(iterator.value());
        }
        ++iterator;
    }
}

QString IDOSImportObjectsCommand::referencedGridId() const
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_targetCaseId.isEmpty())
    {
        return QString();
    }

    IDOSCaseObject* caseObject =
        qobject_cast<IDOSCaseObject*>(targetProject->objectById(m_targetCaseId));
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

QStringList IDOSImportObjectsCommand::collectGridPropertyIds(const QString& gridId) const
{
    QStringList propertyIds;
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || gridId.isEmpty())
    {
        return propertyIds;
    }

    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            propertyIds.append(property->objectId());
        }
    }
    return propertyIds;
}

void IDOSImportObjectsCommand::captureReplacementObjects()
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

    IDOSDataObject* grid = targetProject->objectById(gridId);
    if (grid != nullptr)
    {
        m_replacedObjects.append(QPointer<IDOSDataObject>(grid));
    }

    const QStringList propertyIds = collectGridPropertyIds(gridId);
    for (const QString& propertyId : propertyIds)
    {
        IDOSDataObject* property = targetProject->objectById(propertyId);
        if (property != nullptr)
        {
            m_replacedObjects.append(QPointer<IDOSDataObject>(property));
        }
    }
}

void IDOSImportObjectsCommand::detachReplacementObjects()
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
            targetProject->takeObject(object->objectId());
        }
    }
}

void IDOSImportObjectsCommand::restoreReplacementObjects()
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
            targetProject->addObject(object.data());
        }
    }
}

void IDOSImportObjectsCommand::deleteDetachedObjects()
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

QJsonObject IDOSImportCommandJsonHelpers::filePathSchema()
{
    QJsonObject property;
    property.insert(QStringLiteral("type"), QStringLiteral("string"));
    property.insert(QStringLiteral("description"), QObject::tr("Path of the file to import."));
    return property;
}

QJsonObject IDOSImportCommandJsonHelpers::stringPropertySchema(const QString& description)
{
    QJsonObject property;
    property.insert(QStringLiteral("type"), QStringLiteral("string"));
    property.insert(QStringLiteral("description"), description);
    return property;
}

QJsonObject IDOSImportCommandJsonHelpers::schemaWithProperties(const QJsonObject& properties,
                                                               const QJsonArray& required)
{
    QJsonObject schemaObject;
    schemaObject.insert(QStringLiteral("type"), QStringLiteral("object"));
    schemaObject.insert(QStringLiteral("properties"), properties);
    schemaObject.insert(QStringLiteral("required"), required);
    return schemaObject;
}

IDOSImportCaseCommandMetadata::IDOSImportCaseCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("case.import"),
                          QObject::tr("Import case"),
                          QObject::tr("Import a simulation case file into the current project."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportCaseCommandMetadata::~IDOSImportCaseCommandMetadata() = default;

QJsonObject IDOSImportCaseCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), IDOSImportCommandJsonHelpers::filePathSchema());
    properties.insert(QStringLiteral("caseName"),
                      IDOSImportCommandJsonHelpers::stringPropertySchema(
                          QObject::tr("Optional imported case name.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    return IDOSImportCommandJsonHelpers::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportCaseCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project,
                                        QStringLiteral("case.import"),
                                        arguments.value(QStringLiteral("filePath")).toString(),
                                        IDOSImportObjectsCommand::Mode::Case,
                                        QString(),
                                        QString(),
                                        arguments.value(QStringLiteral("caseName")).toString());
}

IDOSImportGridCommandMetadata::IDOSImportGridCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("grid.import"),
                          QObject::tr("Import grid"),
                          QObject::tr("Import a grid file into a simulation case."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportGridCommandMetadata::~IDOSImportGridCommandMetadata() = default;

QJsonObject IDOSImportGridCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), IDOSImportCommandJsonHelpers::filePathSchema());
    properties.insert(QStringLiteral("targetCaseId"),
                      IDOSImportCommandJsonHelpers::stringPropertySchema(
                          QObject::tr("Object ID of the target case.")));
    QJsonObject replaceProperty;
    replaceProperty.insert(QStringLiteral("type"), QStringLiteral("boolean"));
    replaceProperty.insert(QStringLiteral("description"),
                           QObject::tr("Replace the existing grid in the target case."));
    properties.insert(QStringLiteral("replaceExistingGrid"), replaceProperty);
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("targetCaseId"));
    return IDOSImportCommandJsonHelpers::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportGridCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project,
                                        QStringLiteral("grid.import"),
                                        arguments.value(QStringLiteral("filePath")).toString(),
                                        IDOSImportObjectsCommand::Mode::Grid,
                                        QString(),
                                        arguments.value(QStringLiteral("targetCaseId")).toString(),
                                        QString(),
                                        arguments.value(QStringLiteral("replaceExistingGrid")).toBool());
}

IDOSImportPropertyCommandMetadata::IDOSImportPropertyCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("property.import"),
                          QObject::tr("Import property"),
                          QObject::tr("Import grid properties into a grid."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportPropertyCommandMetadata::~IDOSImportPropertyCommandMetadata() = default;

QJsonObject IDOSImportPropertyCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), IDOSImportCommandJsonHelpers::filePathSchema());
    properties.insert(QStringLiteral("targetGridId"),
                      IDOSImportCommandJsonHelpers::stringPropertySchema(
                          QObject::tr("Object ID of the target grid.")));
    properties.insert(QStringLiteral("targetCaseId"),
                      IDOSImportCommandJsonHelpers::stringPropertySchema(
                          QObject::tr("Object ID of the target case.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("targetGridId"));
    required.append(QStringLiteral("targetCaseId"));
    return IDOSImportCommandJsonHelpers::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportPropertyCommandMetadata::create(const QJsonObject& arguments,
                                                       IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project,
                                        QStringLiteral("property.import"),
                                        arguments.value(QStringLiteral("filePath")).toString(),
                                        IDOSImportObjectsCommand::Mode::Property,
                                        arguments.value(QStringLiteral("targetGridId")).toString(),
                                        arguments.value(QStringLiteral("targetCaseId")).toString());
}

IDOSImportWellDataCommandMetadata::IDOSImportWellDataCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("wellData.import"),
                          QObject::tr("Import well data"),
                          QObject::tr("Import well data as new wells."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportWellDataCommandMetadata::~IDOSImportWellDataCommandMetadata() = default;

QJsonObject IDOSImportWellDataCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), IDOSImportCommandJsonHelpers::filePathSchema());
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    return IDOSImportCommandJsonHelpers::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportWellDataCommandMetadata::create(const QJsonObject& arguments,
                                                       IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project,
                                        QStringLiteral("wellData.import"),
                                        arguments.value(QStringLiteral("filePath")).toString(),
                                        IDOSImportObjectsCommand::Mode::WellData,
                                        QString(),
                                        QString());
}
