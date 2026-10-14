#include <memory>
#include <utility>

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QVector>

#include "fluidrock/idosfluidrocktextprovider.h"
#include "idoscaseitemref.h"
#include "idoscaseobject.h"
#include "idosdataprovider.h"
#include "idosdataprovidermetadata.h"
#include "idosdataproviderregistry.h"
#include "idosfluidrocktable.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idossimulationcaseobject.h"
#include "idoswell.h"

#include "idosimportcommands.h"

IDOSImportObjectsCommand::IDOSImportObjectsCommand(IDOSProject* project, const QString& name,
                                                   const QJsonObject& arguments)
    :
    IDOSCommand(name, Type::Action, project, QObject::tr("Import data"))
    , m_filePath(arguments.value(QStringLiteral("filePath")).toString())
    , m_fluidRockCategory(arguments.value(QStringLiteral("category")).toString())
    , m_providerId(arguments.value(QStringLiteral("providerId")).toString())
    , m_dataTypeId(arguments.value(QStringLiteral("dataTypeId")).toString())
    , m_mode(Mode::WellData)
    , m_targetGridId(arguments.value(QStringLiteral("targetGridId")).toString())
    , m_targetCaseId(arguments.value(QStringLiteral("targetCaseId")).toString())
    , m_targetTableId(arguments.value(QStringLiteral("tableId")).toString())
    , m_caseNameOverride(arguments.value(QStringLiteral("caseName")).toString())
    , m_replaceExistingGrid(arguments.value(QStringLiteral("replaceExistingGrid")).toBool())
    , m_loaded(false)
    , m_targetWellId(arguments.value(QStringLiteral("targetWellId")).toString())
    , m_wellsPrepared(false)
{
    if (name == QStringLiteral("case.import"))
    {
        m_mode = Mode::Case;
    }
    else if (name == QStringLiteral("grid.import"))
    {
        m_mode = Mode::Grid;
    }
    else if (name == QStringLiteral("property.import"))
    {
        m_mode = Mode::Property;
    }
    else if (name == QStringLiteral("data.fluidRock.import"))
    {
        m_mode = Mode::FluidRock;
    }
    else if (name == QStringLiteral("case.fluidRock.attach"))
    {
        m_mode = Mode::FluidRockAttach;
    }
    else if (name == QStringLiteral("data.import"))
    {
        m_mode = Mode::ProjectData;
    }
}
IDOSImportObjectsCommand::~IDOSImportObjectsCommand()
{
    deleteDetachedObjects();
    qDeleteAll(m_wellBefore);
    qDeleteAll(m_wellAfter);
}

bool IDOSImportObjectsCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_mode == Mode::FluidRock &&
        m_fluidRockCategory != QStringLiteral("fluid") &&
        m_fluidRockCategory != QStringLiteral("rockPhysics"))
    {
        error = QObject::tr("The fluid or rock table category is invalid.");
        return false;
    }

    if (m_mode == Mode::ProjectData &&
        (m_providerId.trimmed().isEmpty() || m_dataTypeId.trimmed().isEmpty()))
    {
        error = QObject::tr("The data import provider or type is unavailable.");
        return false;
    }

    if (m_mode != Mode::FluidRockAttach &&
        (m_filePath.trimmed().isEmpty() || (!m_loaded && !QFileInfo::exists(m_filePath))))
    {
        error = QObject::tr("The import file is unavailable.");
        return false;
    }

    if ((m_mode == Mode::Grid || m_mode == Mode::Property || m_mode == Mode::FluidRockAttach) &&
        m_targetCaseId.isEmpty())
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
    if (m_mode == Mode::Grid || m_mode == Mode::Property || m_mode == Mode::FluidRockAttach)
    {
        IDOSCaseObject* targetCase =
            qobject_cast<IDOSCaseObject*>(targetProject->object(m_targetCaseId));
        if (targetCase == nullptr)
        {
            error = QObject::tr("The target case is unavailable.");
            return false;
        }
        if (m_mode == Mode::FluidRockAttach &&
            qobject_cast<IDOSSimulationCaseObject*>(targetCase) == nullptr)
        {
            error = QObject::tr("The target simulation case is unavailable.");
            return false;
        }

        if (m_mode == Mode::FluidRockAttach &&
            qobject_cast<IDOSFluidRockTable*>(targetProject->object(m_targetTableId)) == nullptr)
        {
            error = QObject::tr("The target fluid or rock table is unavailable.");
            return false;
        }
        if (m_mode == Mode::FluidRockAttach)
        {
            for (const IDOSCaseItemRef& ref : targetCase->itemRefs())
            {
                if (ref.role() == QStringLiteral("case.fluidRockTable") &&
                    ref.objectId() == m_targetTableId)
                {
                    error = QObject::tr("The fluid or rock table is already attached to this case.");
                    return false;
                }
            }
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

    if (m_mode == Mode::Property && targetProject->object(m_targetGridId) == nullptr)
    {
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    if (m_mode == Mode::WellData && !m_targetWellId.isEmpty() &&
        qobject_cast<IDOSWell*>(targetProject->object(m_targetWellId)) == nullptr)
    {
        error = QObject::tr("The target well is unavailable.");
        return false;
    }
    return true;
}

QJsonObject IDOSImportObjectsCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("providerId"), m_providerId);
    preview.insert(QStringLiteral("dataTypeId"), m_dataTypeId);
    preview.insert(QStringLiteral("targetWellId"), m_targetWellId);
    preview.insert(QStringLiteral("targetGridId"), m_targetGridId);
    preview.insert(QStringLiteral("targetCaseId"), m_targetCaseId);
    preview.insert(QStringLiteral("tableId"), m_targetTableId);
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

    if (m_mode == Mode::FluidRockAttach)
    {
        IDOSSimulationCaseObject* targetCase =
            qobject_cast<IDOSSimulationCaseObject*>(targetProject->object(m_targetCaseId));
        if (targetCase == nullptr)
        {
            error = QObject::tr("The target simulation case is unavailable.");
            return false;
        }
        captureTargetCaseReferences();
        targetProject->beginUpdate();
        QList<IDOSCaseItemRef> refs = targetCase->itemRefs();
        bool alreadyAttached = false;
        for (const IDOSCaseItemRef& ref : refs)
        {
            if (ref.role() == QStringLiteral("case.fluidRockTable") &&
                ref.objectId() == m_targetTableId)
            {
                alreadyAttached = true;
                break;
            }
        }
        if (!alreadyAttached)
        {
            refs.append(IDOSCaseItemRef(QStringLiteral("case.fluidRockTable"), m_targetTableId));
            targetCase->setItemRefs(refs);
        }
        QJsonObject commandResult;
        commandResult.insert(QStringLiteral("targetCaseId"), m_targetCaseId);
        commandResult.insert(QStringLiteral("tableId"), m_targetTableId);
        setResult(commandResult);
        targetProject->endUpdate();
        return true;
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

    if (m_mode == Mode::WellData)
    {
        return applyWells(error);
    }

    captureTargetCaseReferences();
    targetProject->beginUpdate();
    if (m_mode == Mode::Grid && m_replaceExistingGrid)
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

bool IDOSImportObjectsCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_mode == Mode::WellData)
    {
        return revertWells(error);
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
    IDOSDataProviderMetadata* metadata = nullptr;
    std::unique_ptr<IDOSDataProvider> provider;
    if (m_mode == Mode::FluidRock)
    {
        provider = std::make_unique<IDOSFluidRockTextProvider>();
    }
    else if (m_mode == Mode::ProjectData)
    {
        metadata = IDOSDataProviderRegistry::instance().metadata(m_providerId);
        if (metadata == nullptr || !metadata->canHandle(m_filePath))
        {
            error = QObject::tr("The selected provider cannot read this file.");
            return false;
        }
        provider = metadata->create();
    }
    else
    {
        const QList<IDOSDataProviderMetadata*> metadataList =
            IDOSDataProviderRegistry::instance().providers(m_filePath);
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
    }
    if (provider == nullptr)
    {
        error = QObject::tr("Failed to create provider for: %1").arg(m_filePath);
        return false;
    }

    QList<IDOSDataObject*> objects = provider->read(m_filePath);
    if (objects.isEmpty() || !provider->lastError().isEmpty())
    {
        error = provider->lastError();
        qDeleteAll(objects);
        return false;
    }

    if (m_mode == Mode::FluidRock)
    {
        IDOSSimulationCaseObject* importedTables = nullptr;
        for (IDOSDataObject* object : objects)
        {
            importedTables = qobject_cast<IDOSSimulationCaseObject*>(object);
            if (importedTables != nullptr)
            {
                break;
            }
        }
        if (importedTables == nullptr || importedTables->curveTables().isEmpty())
        {
            error = QObject::tr("The file contains no supported fluid or rock tables.");
            qDeleteAll(objects);
            return false;
        }
        m_importedCurveTables = importedTables->curveTables();
        QMap<QString, int> tableIndices;
        for (IDOSDataObject* object : project()->objects())
        {
            IDOSFluidRockTable* existingTable = qobject_cast<IDOSFluidRockTable*>(object);
            if (existingTable != nullptr)
            {
                const QVariantMap table = existingTable->tableData();
                const QString keyword = table.value(QStringLiteral("keyword")).toString().toUpper();
                tableIndices.insert(keyword, qMax(tableIndices.value(keyword),
                                                  table.value(QStringLiteral("tableIndex")).toInt()));
            }
        }
        for (const QVariant& tableValue : m_importedCurveTables)
        {
            QVariantMap tableData = tableValue.toMap();
            const QString keyword = tableData.value(QStringLiteral("keyword")).toString().toUpper();
            const bool isFluidTable = isFluidKeyword(keyword);
            if ((m_fluidRockCategory == QStringLiteral("fluid") && !isFluidTable) ||
                (m_fluidRockCategory == QStringLiteral("rockPhysics") && isFluidTable))
            {
                continue;
            }
            const int tableIndex = tableIndices.value(keyword) + 1;
            tableIndices.insert(keyword, tableIndex);
            tableData.insert(QStringLiteral("keyword"), keyword);
            tableData.insert(QStringLiteral("tableIndex"), tableIndex);
            const IDOSFluidRockTable::Category category = isFluidTable
                ? IDOSFluidRockTable::Category::Fluid : IDOSFluidRockTable::Category::RockPhysics;
            IDOSFluidRockTable* table = new IDOSFluidRockTable(category);
            table->setTableData(tableData);
            table->setSourceFile(m_filePath);
            table->setName(QObject::tr("%1 — Table %2").arg(keyword).arg(tableIndex));
            m_objects.append(QPointer<IDOSDataObject>(table));
        }
        qDeleteAll(objects);
        if (m_objects.isEmpty())
        {
            error = m_fluidRockCategory == QStringLiteral("fluid")
                ? QObject::tr("The file contains no supported fluid tables.")
                : QObject::tr("The file contains no supported rock physics tables.");
            return false;
        }
        m_loaded = true;
        return true;
    }

    if (m_mode == Mode::Property)
    {
        IDOSGrid* targetGrid = qobject_cast<IDOSGrid*>(project()->object(m_targetGridId));
        if (targetGrid == nullptr)
        {
            error = QObject::tr("The target grid is unavailable.");
            qDeleteAll(objects);
            return false;
        }
        for (IDOSDataObject* object : objects)
        {
            IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
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

    if (m_mode == Mode::ProjectData)
    {
        return object != nullptr && object->typeId() == m_dataTypeId;
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

    IDOSSimulationCaseObject* simulationCase =
        m_mode == Mode::Case ? qobject_cast<IDOSSimulationCaseObject*>(caseObject) : nullptr;
    if (simulationCase != nullptr)
    {
        const QVariantList curveTables = simulationCase->curveTables();
        QMap<QString, int> tableIndices;
        for (IDOSDataObject* existingObject : project()->objects())
        {
            IDOSFluidRockTable* existingTable = qobject_cast<IDOSFluidRockTable*>(existingObject);
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
            const IDOSFluidRockTable::Category category = isFluidKeyword(keyword)
                ? IDOSFluidRockTable::Category::Fluid : IDOSFluidRockTable::Category::RockPhysics;
            IDOSFluidRockTable* table = new IDOSFluidRockTable(category);
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

    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
    if (m_mode == Mode::Property && property != nullptr)
    {
        IDOSGrid* grid = project() != nullptr
            ? qobject_cast<IDOSGrid*>(project()->object(m_targetGridId)) : nullptr;
        if (grid != nullptr)
        {
            const QVector<double> propertyValues = property->values();
            property->setDimensions(grid->nx(), grid->ny(), grid->nz());
            property->setValues(propertyValues);
        }
        property->setGridId(m_targetGridId);
    }
}

bool IDOSImportObjectsCommand::isFluidKeyword(const QString& keyword) const
{
    const QStringList fluidKeywords = {
        QStringLiteral("DENSITY"), QStringLiteral("PVTO"), QStringLiteral("PVDO"),
        QStringLiteral("PVDG"), QStringLiteral("PVTG"), QStringLiteral("PVZG"),
        QStringLiteral("PVTW")
    };
    return fluidKeywords.contains(keyword.toUpper());
}

bool IDOSImportObjectsCommand::prepareWells(QString& error)
{
    if (m_wellsPrepared)
    {
        return true;
    }
    QList<IDOSWell*> candidates;
    for (IDOSDataObject* data : project()->objects())
    {
        IDOSWell* well = qobject_cast<IDOSWell*>(data);
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
        IDOSWell* source = qobject_cast<IDOSWell*>(data.data());
        if (source == nullptr)
        {
            error = QObject::tr("The imported well is unavailable.");
            return false;
        }
        IDOSWell* target = qobject_cast<IDOSWell*>(project()->object(m_targetWellId));
        if (m_targetWellId.isEmpty())
        {
            for (IDOSWell* candidate : candidates)
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
            target->setVisible(false);
            candidates.append(target);
            m_createdWellIds.append(target->objectId());
        }
        const QString id = target->objectId();
        if (!m_wellTargets.contains(id))
        {
            m_wellTargets.insert(id, target);
            IDOSWell* after = new IDOSWell();
            after->copyInputData(target);
            m_wellAfter.insert(id, after);
            if (!m_createdWellIds.contains(id))
            {
                IDOSWell* before = new IDOSWell();
                before->copyInputData(target);
                m_wellBefore.insert(id, before);
            }
        }
        IDOSWell* after = m_wellAfter.value(id);
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

bool IDOSImportObjectsCommand::validateWells(bool attached, QString& error) const
{
    for (const QString& id : m_wellTargets.keys())
    {
        IDOSWell* target = m_wellTargets.value(id).data();
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

bool IDOSImportObjectsCommand::applyWells(QString& error)
{
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
        IDOSWell* target = m_wellTargets.value(id).data();
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

bool IDOSImportObjectsCommand::revertWells(QString& error)
{
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

void IDOSImportObjectsCommand::captureTargetCaseReferences()
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

void IDOSImportObjectsCommand::appendTargetCaseReferences()
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
        else if (qobject_cast<IDOSFluidRockTable*>(object.data()) != nullptr)
        {
            role = QStringLiteral("case.fluidRockTable");
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
            qobject_cast<IDOSCaseObject*>(targetProject->object(iterator.key()));
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
            targetProject->take(object->objectId());
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
            targetProject->add(object.data());
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

QJsonObject IDOSImportObjectsCommand::filePathSchema()
{
    QJsonObject property;
    property.insert(QStringLiteral("type"), QStringLiteral("string"));
    property.insert(QStringLiteral("description"), QObject::tr("Path of the file to import."));
    return property;
}

QJsonObject IDOSImportObjectsCommand::stringPropertySchema(const QString& description)
{
    QJsonObject property;
    property.insert(QStringLiteral("type"), QStringLiteral("string"));
    property.insert(QStringLiteral("description"), description);
    return property;
}

QJsonObject IDOSImportObjectsCommand::schemaWithProperties(const QJsonObject& properties,
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
    properties.insert(QStringLiteral("filePath"), IDOSImportObjectsCommand::filePathSchema());
    properties.insert(QStringLiteral("caseName"),
                      IDOSImportObjectsCommand::stringPropertySchema(
                          QObject::tr("Optional imported case name.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    return IDOSImportObjectsCommand::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportCaseCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project, name(), arguments);
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
    properties.insert(QStringLiteral("filePath"), IDOSImportObjectsCommand::filePathSchema());
    properties.insert(QStringLiteral("targetCaseId"),
                      IDOSImportObjectsCommand::stringPropertySchema(
                          QObject::tr("Object ID of the target case.")));
    QJsonObject replaceProperty;
    replaceProperty.insert(QStringLiteral("type"), QStringLiteral("boolean"));
    replaceProperty.insert(QStringLiteral("description"),
                           QObject::tr("Replace the existing grid in the target case."));
    properties.insert(QStringLiteral("replaceExistingGrid"), replaceProperty);
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("targetCaseId"));
    return IDOSImportObjectsCommand::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportGridCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project, name(), arguments);
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
    properties.insert(QStringLiteral("filePath"), IDOSImportObjectsCommand::filePathSchema());
    properties.insert(QStringLiteral("targetGridId"),
                      IDOSImportObjectsCommand::stringPropertySchema(
                          QObject::tr("Object ID of the target grid.")));
    properties.insert(QStringLiteral("targetCaseId"),
                      IDOSImportObjectsCommand::stringPropertySchema(
                          QObject::tr("Object ID of the target case.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("targetGridId"));
    required.append(QStringLiteral("targetCaseId"));
    return IDOSImportObjectsCommand::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportPropertyCommandMetadata::create(const QJsonObject& arguments,
                                                       IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project, name(), arguments);
}

IDOSImportWellDataCommandMetadata::IDOSImportWellDataCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("wellData.import"),
                          QObject::tr("Import well data"),
                          QObject::tr("Import well data, merging wells with matching names."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportWellDataCommandMetadata::~IDOSImportWellDataCommandMetadata() = default;

QJsonObject IDOSImportWellDataCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), IDOSImportObjectsCommand::filePathSchema());
    properties.insert(QStringLiteral("targetWellId"),
                      IDOSImportObjectsCommand::stringPropertySchema(
                          QObject::tr("Optional object ID of the well receiving the imported data.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    return IDOSImportObjectsCommand::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportWellDataCommandMetadata::create(const QJsonObject& arguments,
                                                       IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project, name(), arguments);
}

IDOSImportFluidRockTablesCommandMetadata::IDOSImportFluidRockTablesCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("data.fluidRock.import"),
                          QObject::tr("Import fluid and rock tables"),
                          QObject::tr("Import ECLIPSE fluid and rock tables into project data."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportFluidRockTablesCommandMetadata::~IDOSImportFluidRockTablesCommandMetadata() = default;

QJsonObject IDOSImportFluidRockTablesCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), IDOSImportObjectsCommand::filePathSchema());
    QJsonObject categorySchema = IDOSImportObjectsCommand::stringPropertySchema(
        QObject::tr("Table category to import: fluid or rockPhysics."));
    QJsonArray categories;
    categories.append(QStringLiteral("fluid"));
    categories.append(QStringLiteral("rockPhysics"));
    categorySchema.insert(QStringLiteral("enum"), categories);
    properties.insert(QStringLiteral("category"), categorySchema);
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("category"));
    return IDOSImportObjectsCommand::schemaWithProperties(properties, required);
}

IDOSAttachFluidRockTableCommandMetadata::IDOSAttachFluidRockTableCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("case.fluidRock.attach"),
                          QObject::tr("Add fluid or rock table to case"),
                          QObject::tr("Add a project fluid or rock table reference to a simulation case."),
                          IDOSCommand::Type::Action)
{
}

IDOSAttachFluidRockTableCommandMetadata::~IDOSAttachFluidRockTableCommandMetadata() = default;

QJsonObject IDOSAttachFluidRockTableCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("targetCaseId"),
                      IDOSImportObjectsCommand::stringPropertySchema(
                          QObject::tr("Object ID of the target simulation case.")));
    properties.insert(QStringLiteral("tableId"),
                      IDOSImportObjectsCommand::stringPropertySchema(
                          QObject::tr("Object ID of the fluid or rock table.")));
    QJsonArray required;
    required.append(QStringLiteral("targetCaseId"));
    required.append(QStringLiteral("tableId"));
    return IDOSImportObjectsCommand::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSAttachFluidRockTableCommandMetadata::create(const QJsonObject& arguments,
                                                              IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project, name(), arguments);
}

IDOSCommand* IDOSImportFluidRockTablesCommandMetadata::create(const QJsonObject& arguments,
                                                               IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project, name(), arguments);
}

IDOSImportProjectDataCommandMetadata::IDOSImportProjectDataCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("data.import"),
                          QObject::tr("Import project data"),
                          QObject::tr("Import a supported data file into its project data category."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportProjectDataCommandMetadata::~IDOSImportProjectDataCommandMetadata() = default;

QJsonObject IDOSImportProjectDataCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), IDOSImportObjectsCommand::filePathSchema());
    properties.insert(QStringLiteral("providerId"), IDOSImportObjectsCommand::stringPropertySchema(
        QObject::tr("Registered data provider ID.")));
    properties.insert(QStringLiteral("dataTypeId"), IDOSImportObjectsCommand::stringPropertySchema(
        QObject::tr("Expected project data type ID.")));
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("providerId"));
    required.append(QStringLiteral("dataTypeId"));
    return IDOSImportObjectsCommand::schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportProjectDataCommandMetadata::create(const QJsonObject& arguments,
                                                           IDOSProject* project) const
{
    return new IDOSImportObjectsCommand(project, name(), arguments);
}
