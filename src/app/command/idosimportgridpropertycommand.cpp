#include <memory>

#include <QFileInfo>
#include <QJsonArray>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVector>

#include "data/case/idoscaseitemref.h"
#include "data/case/idoscaseobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataprovidermetadata.h"
#include "providers/idosdataproviderregistry.h"

#include "idosimportgridpropertycommand.h"

IDOSImportGridPropertyCommand::IDOSImportGridPropertyCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("property.import"), Type::Action, project, QObject::tr("Import property"))
    , m_filePath(arguments.value(QStringLiteral("filePath")).toString())
    , m_loaded(false)
    , m_targetCaseId(arguments.value(QStringLiteral("targetCaseId")).toString())
    , m_targetGridId(arguments.value(QStringLiteral("targetGridId")).toString())
{
}

IDOSImportGridPropertyCommand::~IDOSImportGridPropertyCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
}

bool IDOSImportGridPropertyCommand::validateCommand(QString& error) const
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

    if (m_targetGridId.isEmpty())
    {
        error = QObject::tr("The target grid is unavailable.");
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

    if (targetProject->object(m_targetGridId) == nullptr)
    {
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    return true;
}

QJsonObject IDOSImportGridPropertyCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("targetGridId"), m_targetGridId);
    preview.insert(QStringLiteral("targetCaseId"), m_targetCaseId);
    return preview;
}

bool IDOSImportGridPropertyCommand::apply(QString& error)
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

bool IDOSImportGridPropertyCommand::revert(QString& error)
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

bool IDOSImportGridPropertyCommand::loadObjects(QString& error)
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

    IDOSGridObject* targetGrid = qobject_cast<IDOSGridObject*>(project()->object(m_targetGridId));
    if (targetGrid == nullptr)
    {
        error = QObject::tr("The target grid is unavailable.");
        qDeleteAll(objects);
        return false;
    }
    for (IDOSDataObject* object : objects)
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        if (property == nullptr)
        {
            continue;
        }

        QVector<double> propertyValues = property->values();
        const bool isInitFile = QFileInfo(m_filePath).suffix().compare(QStringLiteral("INIT"), Qt::CaseInsensitive) == 0;
        if (propertyValues.size() != targetGrid->totalCellCount() && isInitFile)
        {
            const QVector<int>& actnum = targetGrid->actnum();
            int activeCellCount = 0;
            for (int cellIndex = 0; cellIndex < actnum.size(); ++cellIndex)
            {
                if (actnum.at(cellIndex) != 0)
                {
                    ++activeCellCount;
                }
            }
            if (actnum.size() == targetGrid->totalCellCount() && propertyValues.size() == activeCellCount)
            {
                QVector<double> expandedValues(targetGrid->totalCellCount(), 0.0);
                int activeIndex = 0;
                for (int cellIndex = 0; cellIndex < actnum.size(); ++cellIndex)
                {
                    if (actnum.at(cellIndex) != 0)
                    {
                        expandedValues[cellIndex] = propertyValues.at(activeIndex);
                        ++activeIndex;
                    }
                }
                property->setValues(expandedValues);
                propertyValues = expandedValues;
            }
        }

        if (propertyValues.size() != targetGrid->totalCellCount())
        {
            error = QObject::tr("Property %1 has %2 cell values; the target grid requires %3.")
                        .arg(property->keyword())
                        .arg(propertyValues.size())
                        .arg(targetGrid->totalCellCount());
            qDeleteAll(objects);
            return false;
        }

        if (isInitFile)
        {
            property->setDimensions(targetGrid->nx(), targetGrid->ny(), targetGrid->nz());
            if (!property->setValues(propertyValues))
            {
                error = QObject::tr("Property %1 could not be expanded to the target grid dimensions.")
                            .arg(property->keyword());
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

        if (qobject_cast<IDOSGridPropertyObject*>(object) == nullptr)
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

void IDOSImportGridPropertyCommand::prepareObject(IDOSDataObject* object)
{
    IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
    if (property != nullptr)
    {
        IDOSGridObject* grid = project() != nullptr
            ? qobject_cast<IDOSGridObject*>(project()->object(m_targetGridId)) : nullptr;
        if (grid != nullptr)
        {
            const QVector<double> propertyValues = property->values();
            property->setDimensions(grid->nx(), grid->ny(), grid->nz());
            property->setValues(propertyValues);
        }
        property->setGridId(m_targetGridId);
    }
}

void IDOSImportGridPropertyCommand::captureTargetCaseReferences()
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

void IDOSImportGridPropertyCommand::appendTargetCaseReferences()
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

        const QString role = QStringLiteral("case.gridProperty");

        const QString key = role + QLatin1Char(':') + object->objectId();
        if (!role.isEmpty() && !seen.contains(key))
        {
            refs.append(IDOSCaseItemRef(role, object->objectId()));
            seen.insert(key);
        }
    }
    caseObject->setItemRefs(refs);
}

void IDOSImportGridPropertyCommand::restoreTargetCaseReferences()
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

IDOSImportGridPropertyCommandMetadata::IDOSImportGridPropertyCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("property.import"),
                          QObject::tr("Import property"),
                          QObject::tr("Import grid properties into a grid."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportGridPropertyCommandMetadata::~IDOSImportGridPropertyCommandMetadata() = default;

QJsonObject IDOSImportGridPropertyCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), stringPropertySchema(QObject::tr("Path of the file to import.")));
    properties.insert(QStringLiteral("targetGridId"),
                      stringPropertySchema(
                          QObject::tr("Object ID of the target grid.")));
    properties.insert(QStringLiteral("targetCaseId"),
                      stringPropertySchema(
                          QObject::tr("Object ID of the target case.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("targetGridId"));
    required.append(QStringLiteral("targetCaseId"));
    return schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportGridPropertyCommandMetadata::create(const QJsonObject& arguments,
                                                       IDOSProject* project) const
{
    return new IDOSImportGridPropertyCommand(project, arguments);
}
