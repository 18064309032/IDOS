#include <memory>

#include <QFileInfo>
#include <QJsonArray>
#include <QObject>
#include <QSet>
#include <QStringList>

#include "idosdataobject.h"
#include "idosproject.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataprovidermetadata.h"
#include "providers/idosdataproviderregistry.h"

#include "idosimportdatacommand.h"

IDOSImportDataCommand::IDOSImportDataCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("data.import"), Type::Action, project, QObject::tr("Import project data"))
    , m_filePath(arguments.value(QStringLiteral("filePath")).toString())
    , m_loaded(false)
    , m_providerId(arguments.value(QStringLiteral("providerId")).toString())
    , m_dataTypeId(arguments.value(QStringLiteral("dataTypeId")).toString())
{
}

IDOSImportDataCommand::~IDOSImportDataCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
}

bool IDOSImportDataCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_providerId.trimmed().isEmpty() || m_dataTypeId.trimmed().isEmpty())
    {
        error = QObject::tr("The data import provider or type is unavailable.");
        return false;
    }

    if (m_filePath.trimmed().isEmpty() || (!m_loaded && !QFileInfo::exists(m_filePath)))
    {
        error = QObject::tr("The import file is unavailable.");
        return false;
    }

    return true;
}

QJsonObject IDOSImportDataCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("providerId"), m_providerId);
    preview.insert(QStringLiteral("dataTypeId"), m_dataTypeId);
    return preview;
}

bool IDOSImportDataCommand::apply(QString& error)
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

bool IDOSImportDataCommand::revert(QString& error)
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

bool IDOSImportDataCommand::loadObjects(QString& error)
{
    IDOSDataProviderMetadata* metadata = nullptr;
    std::unique_ptr<IDOSDataProvider> provider;
    metadata = IDOSDataProviderRegistry::instance().metadata(m_providerId);
    if (metadata == nullptr || !metadata->canHandle(m_filePath))
    {
        error = QObject::tr("The selected provider cannot read this file.");
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

        if (object->typeId() != m_dataTypeId)
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

IDOSImportDataCommandMetadata::IDOSImportDataCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("data.import"),
                          QObject::tr("Import project data"),
                          QObject::tr("Import a supported data file into its project data category."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportDataCommandMetadata::~IDOSImportDataCommandMetadata() = default;

QJsonObject IDOSImportDataCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), stringPropertySchema(QObject::tr("Path of the file to import.")));
    properties.insert(QStringLiteral("providerId"), stringPropertySchema(
        QObject::tr("Registered data provider ID.")));
    properties.insert(QStringLiteral("dataTypeId"), stringPropertySchema(
        QObject::tr("Expected project data type ID.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("providerId"));
    required.append(QStringLiteral("dataTypeId"));
    return schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportDataCommandMetadata::create(const QJsonObject& arguments,
                                                           IDOSProject* project) const
{
    return new IDOSImportDataCommand(project, arguments);
}
