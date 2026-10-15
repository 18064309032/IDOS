#include <utility>

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "data/case/idoscaseitemref.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/fluidrock/idosfluidrocktable.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridproperty.h"
#include "idosproject.h"

#include "idosdeletecasecommand.h"

IDOSDeleteCaseCommand::IDOSDeleteCaseCommand(
    IDOSProject* project,
    QString caseId)
    : IDOSCommand(QStringLiteral("case.delete"),
                  Type::Action,
                  project,
                  QObject::tr("Delete case"))
    , m_caseId(std::move(caseId))
    , m_caseObject(nullptr)
{
}

IDOSDeleteCaseCommand::~IDOSDeleteCaseCommand()
{
    clearDetachedObjects();
}

bool IDOSDeleteCaseCommand::validateCommand(QString& error) const
{
    if (m_caseId.isEmpty())
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    IDOSDataObject* object = targetProject->object(m_caseId);
    if (object == nullptr && m_caseObject.isNull())
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    if (object != nullptr && qobject_cast<IDOSCaseObject*>(object) == nullptr)
    {
        error = QObject::tr("The target object is not a case.");
        return false;
    }

    return true;
}

QJsonObject IDOSDeleteCaseCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("objectId"), m_caseId);
    IDOSDataObject* object = project() != nullptr ? project()->object(m_caseId) : nullptr;
    preview.insert(QStringLiteral("type"), object != nullptr ? object->typeId() : QString());
    return preview;
}

bool IDOSDeleteCaseCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    const QStringList gridIds = collectCaseGridIds();
    const QStringList fluidRockTableIds = collectCaseFluidRockTableIds();
    m_grids.clear();
    m_properties.clear();
    m_fluidRockTables.clear();

    for (const QString& tableId : fluidRockTableIds)
    {
        if (isReferencedByOtherCase(tableId))
        {
            continue;
        }

        IDOSDataObject* detachedObject = targetProject->take(tableId);
        IDOSFluidRockTable* table = qobject_cast<IDOSFluidRockTable*>(detachedObject);
        if (table != nullptr)
        {
            m_fluidRockTables.append(QPointer<IDOSFluidRockTable>(table));
        }
        else if (detachedObject != nullptr)
        {
            targetProject->add(detachedObject);
        }
    }

    for (const QString& gridId : gridIds)
    {
        const QStringList propertyIds = collectGridPropertyIds(gridId);
        for (const QString& propertyId : propertyIds)
        {
            IDOSDataObject* detachedObject = targetProject->take(propertyId);
            IDOSDataObject* property = detachedObject;
            if (property != nullptr)
            {
                m_properties.append(QPointer<IDOSDataObject>(property));
            }
            else if (detachedObject != nullptr)
            {
                targetProject->add(detachedObject);
            }
        }

        IDOSDataObject* detachedGridObject = targetProject->take(gridId);
        IDOSGrid* grid = qobject_cast<IDOSGrid*>(detachedGridObject);
        if (grid != nullptr)
        {
            m_grids.append(QPointer<IDOSGrid>(grid));
        }
        else if (detachedGridObject != nullptr)
        {
            targetProject->add(detachedGridObject);
        }
    }

    IDOSDataObject* detachedCaseObject = targetProject->take(m_caseId);
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(detachedCaseObject);
    if (caseObject == nullptr)
    {
        if (detachedCaseObject != nullptr)
        {
            targetProject->add(detachedCaseObject);
        }
        for (const QPointer<IDOSGrid>& grid : m_grids)
        {
            if (!grid.isNull() && grid->parent() == nullptr)
            {
                targetProject->add(grid.data());
            }
        }
        for (const QPointer<IDOSDataObject>& property : m_properties)
        {
            if (!property.isNull() && property->parent() == nullptr)
            {
                targetProject->add(property.data());
            }
        }
        for (const QPointer<IDOSFluidRockTable>& table : m_fluidRockTables)
        {
            if (!table.isNull() && table->parent() == nullptr)
            {
                targetProject->add(table.data());
            }
        }
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    m_caseObject = caseObject;

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("objectId"), m_caseId);
    commandResult.insert(QStringLiteral("name"), caseObject->name());
    commandResult.insert(QStringLiteral("gridCount"), m_grids.size());
    commandResult.insert(QStringLiteral("propertyCount"), m_properties.size());
    setResult(commandResult);
    return true;
}

bool IDOSDeleteCaseCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_caseObject.isNull())
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    for (const QPointer<IDOSGrid>& grid : m_grids)
    {
        if (!grid.isNull() && grid->parent() == nullptr)
        {
            targetProject->add(grid.data());
        }
    }

    for (const QPointer<IDOSDataObject>& property : m_properties)
    {
        if (!property.isNull() && property->parent() == nullptr)
        {
            targetProject->add(property.data());
        }
    }

    for (const QPointer<IDOSFluidRockTable>& table : m_fluidRockTables)
    {
        if (!table.isNull() && table->parent() == nullptr)
        {
            targetProject->add(table.data());
        }
    }

    if (m_caseObject->parent() != nullptr)
    {
        error = QObject::tr("The target case is already attached to a project.");
        return false;
    }

    targetProject->add(m_caseObject.data());
    return true;
}

QStringList IDOSDeleteCaseCommand::collectCaseGridIds() const
{
    QStringList gridIds;
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        return gridIds;
    }

    IDOSCaseObject* caseObject =
        qobject_cast<IDOSCaseObject*>(targetProject->object(m_caseId));
    if (caseObject == nullptr)
    {
        return gridIds;
    }

    const QList<IDOSCaseItemRef> refs = caseObject->itemRefs();
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.grid"))
        {
            gridIds.append(ref.objectId());
        }
    }
    return gridIds;
}

QStringList IDOSDeleteCaseCommand::collectGridPropertyIds(const QString& gridId) const
{
    QStringList propertyIds;
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || gridId.isEmpty())
    {
        return propertyIds;
    }

    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSDataObject* property = object;
        if (property->containerId() == gridId)
        {
            propertyIds.append(property->objectId());
        }
    }
    return propertyIds;
}

QStringList IDOSDeleteCaseCommand::collectCaseFluidRockTableIds() const
{
    QStringList tableIds;
    IDOSProject* targetProject = project();
    IDOSCaseObject* caseObject = targetProject != nullptr
        ? qobject_cast<IDOSCaseObject*>(targetProject->object(m_caseId)) : nullptr;
    if (caseObject == nullptr)
    {
        return tableIds;
    }

    IDOSSimulationCaseObject* simulationCase = qobject_cast<IDOSSimulationCaseObject*>(caseObject);
    QString normalizedCaseSource;
    if (simulationCase != nullptr && !simulationCase->sourceFile().trimmed().isEmpty())
    {
        normalizedCaseSource = QDir::cleanPath(QFileInfo(simulationCase->sourceFile()).absoluteFilePath());
    }

    for (const IDOSCaseItemRef& reference : caseObject->itemRefs())
    {
        if (reference.role() != QStringLiteral("case.fluidRockTable"))
        {
            continue;
        }

        IDOSFluidRockTable* table = qobject_cast<IDOSFluidRockTable*>(targetProject->object(reference.objectId()));
        if (table == nullptr)
        {
            continue;
        }

        const QString originCaseId = table->tableData().value(QStringLiteral("originCaseId")).toString();
        if (!originCaseId.isEmpty())
        {
            tableIds.append(table->objectId());
            continue;
        }

        if (normalizedCaseSource.isEmpty() || table->sourceFile().trimmed().isEmpty())
        {
            continue;
        }

        const QString normalizedTableSource = QDir::cleanPath(QFileInfo(table->sourceFile()).absoluteFilePath());
        if (QString::compare(normalizedCaseSource, normalizedTableSource, Qt::CaseInsensitive) == 0)
        {
            tableIds.append(table->objectId());
        }
    }
    return tableIds;
}

bool IDOSDeleteCaseCommand::isReferencedByOtherCase(const QString& objectId) const
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || objectId.isEmpty())
    {
        return false;
    }

    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(object);
        if (caseObject == nullptr || caseObject->objectId() == m_caseId)
        {
            continue;
        }

        for (const IDOSCaseItemRef& reference : caseObject->itemRefs())
        {
            if (reference.objectId() == objectId)
            {
                return true;
            }
        }
    }
    return false;
}

void IDOSDeleteCaseCommand::clearDetachedObjects()
{
    for (const QPointer<IDOSFluidRockTable>& table : m_fluidRockTables)
    {
        if (!table.isNull() && table->parent() == nullptr)
        {
            delete table.data();
        }
    }

    for (const QPointer<IDOSDataObject>& property : m_properties)
    {
        if (!property.isNull() && property->parent() == nullptr)
        {
            delete property.data();
        }
    }

    for (const QPointer<IDOSGrid>& grid : m_grids)
    {
        if (!grid.isNull() && grid->parent() == nullptr)
        {
            delete grid.data();
        }
    }

    if (!m_caseObject.isNull() && m_caseObject->parent() == nullptr)
    {
        delete m_caseObject.data();
    }
}

IDOSDeleteCaseCommandMetadata::IDOSDeleteCaseCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("case.delete"),
                          QObject::tr("Delete case"),
                          QObject::tr("Delete a simulation case, its private grid data and unshared fluid tables imported with it."),
                          IDOSCommand::Type::Action)
{
}

IDOSDeleteCaseCommandMetadata::~IDOSDeleteCaseCommandMetadata() = default;

QJsonObject IDOSDeleteCaseCommandMetadata::schema() const
{
    QJsonObject caseIdProperty;
    caseIdProperty.insert(QStringLiteral("type"), QStringLiteral("string"));
    caseIdProperty.insert(QStringLiteral("description"),
                          QObject::tr("Object ID of the case to delete."));

    QJsonObject properties;
    properties.insert(QStringLiteral("caseId"), caseIdProperty);

    QJsonArray required;
    required.append(QStringLiteral("caseId"));

    QJsonObject schemaObject;
    schemaObject.insert(QStringLiteral("type"), QStringLiteral("object"));
    schemaObject.insert(QStringLiteral("properties"), properties);
    schemaObject.insert(QStringLiteral("required"), required);
    return schemaObject;
}

IDOSCommand* IDOSDeleteCaseCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    const QString caseId = arguments.value(QStringLiteral("caseId")).toString();
    return new IDOSDeleteCaseCommand(project, caseId);
}
