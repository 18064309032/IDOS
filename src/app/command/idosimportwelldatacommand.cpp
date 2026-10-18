#include <memory>

#include <QFileInfo>
#include <QJsonArray>
#include <QObject>
#include <QStringList>

#include "data/well/idoswellobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataprovidermetadata.h"
#include "providers/idosdataproviderregistry.h"

#include "idosimportwelldatacommand.h"

IDOSImportWellDataCommand::IDOSImportWellDataCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("wellData.import"), Type::Action, project, QObject::tr("Import well data"))
    , m_filePath(arguments.value(QStringLiteral("filePath")).toString())
    , m_loaded(false)
    , m_targetWellId(arguments.value(QStringLiteral("targetWellId")).toString())
    , m_wellsPrepared(false)
{
}

IDOSImportWellDataCommand::~IDOSImportWellDataCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
    qDeleteAll(m_wellBefore);
    qDeleteAll(m_wellAfter);
}

bool IDOSImportWellDataCommand::validateCommand(QString& error) const
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

    IDOSProject* targetProject = project();

    if (!m_targetWellId.isEmpty() && qobject_cast<IDOSWellObject*>(targetProject->object(m_targetWellId)) == nullptr)
    {
        error = QObject::tr("The target well is unavailable.");
        return false;
    }
    return true;
}

QJsonObject IDOSImportWellDataCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("targetWellId"), m_targetWellId);
    return preview;
}

bool IDOSImportWellDataCommand::apply(QString& error)
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

    if (!prepareWells(error) || !validateWells(false, error))
    {
        return false;
    }
    project()->beginUpdate();
    QJsonArray ids;
    QStringList orderedIds = m_wellTargets.keys();
    orderedIds.sort();
    for (const QString& id : orderedIds)
    {
        IDOSWellObject* target = m_wellTargets.value(id).data();
        target->copyInputData(m_wellAfter.value(id));
        if (m_createdWellIds.contains(id))
        {
            project()->add(target);
        }
        ids.append(id);
    }
    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("filePath"), m_filePath);
    commandResult.insert(QStringLiteral("objectCount"), ids.size());
    commandResult.insert(QStringLiteral("objectIds"), ids);
    commandResult.insert(QStringLiteral("createdCount"), m_createdWellIds.size());
    commandResult.insert(QStringLiteral("mergedCount"), m_wellBefore.size());
    setResult(commandResult);
    project()->endUpdate();
    return true;
}

bool IDOSImportWellDataCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (!validateWells(true, error))
    {
        return false;
    }
    project()->beginUpdate();
    for (const QString& id : m_wellBefore.keys())
    {
        m_wellTargets.value(id)->copyInputData(m_wellBefore.value(id));
    }
    for (const QString& id : m_createdWellIds)
    {
        project()->take(id);
    }
    project()->endUpdate();
    return true;
}

bool IDOSImportWellDataCommand::loadObjects(QString& error)
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

        if (qobject_cast<IDOSWellObject*>(object) == nullptr)
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

bool IDOSImportWellDataCommand::prepareWells(QString& error)
{
    if (m_wellsPrepared)
    {
        return true;
    }
    QList<IDOSWellObject*> candidates;
    for (IDOSDataObject* data : project()->objects())
    {
        IDOSWellObject* well = qobject_cast<IDOSWellObject*>(data);
        if (well != nullptr)
        {
            candidates.append(well);
        }
    }
    if (!m_targetWellId.isEmpty() && m_objects.size() != 1)
    {
        error = QObject::tr("A targeted well import must contain exactly one well.");
        return false;
    }
    for (const QPointer<IDOSDataObject>& data : m_objects)
    {
        IDOSWellObject* source = qobject_cast<IDOSWellObject*>(data.data());
        if (source == nullptr)
        {
            error = QObject::tr("The imported well is unavailable.");
            return false;
        }
        IDOSWellObject* target = qobject_cast<IDOSWellObject*>(project()->object(m_targetWellId));
        if (m_targetWellId.isEmpty())
        {
            for (IDOSWellObject* candidate : candidates)
            {
                if (candidate->name().trimmed().compare(source->name().trimmed(), Qt::CaseInsensitive) == 0)
                {
                    target = candidate;
                    break;
                }
            }
        }
        if (target == nullptr)
        {
            target = source;
            candidates.append(target);
            m_createdWellIds.append(target->objectId());
        }
        const QString id = target->objectId();
        if (!m_wellTargets.contains(id))
        {
            m_wellTargets.insert(id, target);
            IDOSWellObject* after = new IDOSWellObject();
            after->copyInputData(target);
            m_wellAfter.insert(id, after);
            if (!m_createdWellIds.contains(id))
            {
                IDOSWellObject* before = new IDOSWellObject();
                before->copyInputData(target);
                m_wellBefore.insert(id, before);
            }
        }
        IDOSWellObject* after = m_wellAfter.value(id);
        // Group imports preserve an existing well head; explicit targeting allows replacement.
        if (source->hasWellHead() && after->hasWellHead() && m_targetWellId.isEmpty())
        {
            continue;
        }
        after->mergeFrom(source);
    }
    m_wellsPrepared = true;
    return true;
}

bool IDOSImportWellDataCommand::validateWells(bool attached, QString& error) const
{
    for (const QString& id : m_wellTargets.keys())
    {
        IDOSWellObject* target = m_wellTargets.value(id).data();
        const bool created = m_createdWellIds.contains(id);
        if (target == nullptr || ((attached || !created) && project()->object(id) != target) ||
            (!attached && created && (target->parent() != nullptr || project()->object(id) != nullptr)))
        {
            error = QObject::tr("The import target changed or is unavailable.");
            return false;
        }
    }
    return true;
}

IDOSImportWellDataCommandMetadata::IDOSImportWellDataCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("wellData.import"),
                          QObject::tr("Import well data"),
                          QObject::tr("Import well data, merging wells with matching names."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportWellDataCommandMetadata::~IDOSImportWellDataCommandMetadata() = default;

QJsonObject IDOSImportWellDataCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), stringPropertySchema(QObject::tr("Path of the file to import.")));
    properties.insert(QStringLiteral("targetWellId"),
                      stringPropertySchema(
                          QObject::tr("Optional object ID of the well receiving the imported data.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    return schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportWellDataCommandMetadata::create(const QJsonObject& arguments,
                                                       IDOSProject* project) const
{
    return new IDOSImportWellDataCommand(project, arguments);
}
