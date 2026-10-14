#include <utility>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "data/case/idoscaseobject.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"
#include "idosproject.h"

#include "idosdeletegridcommand.h"

IDOSDeleteGridCommand::IDOSDeleteGridCommand(
    IDOSProject* project,
    QString gridId)
    : IDOSCommand(QStringLiteral("grid.delete"),
                  Type::Action,
                  project,
                  QObject::tr("Delete grid"))
    , m_gridId(std::move(gridId))
    , m_grid(nullptr)
{
}

IDOSDeleteGridCommand::~IDOSDeleteGridCommand()
{
    clearDetachedObjects();
}

bool IDOSDeleteGridCommand::validateCommand(QString& error) const
{
    if (m_gridId.isEmpty())
    {
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    IDOSDataObject* object = targetProject->object(m_gridId);
    if (object == nullptr && m_grid.isNull())
    {
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    if (object != nullptr && qobject_cast<IDOSGrid*>(object) == nullptr)
    {
        error = QObject::tr("The target object is not a grid.");
        return false;
    }

    return true;
}

QJsonObject IDOSDeleteGridCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("objectId"), m_gridId);
    preview.insert(QStringLiteral("type"), QStringLiteral("idos.grid"));
    return preview;
}

bool IDOSDeleteGridCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    captureCaseReferences();
    m_filters.clear();
    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(object);
        if (filter != nullptr && filter->gridId() == m_gridId)
        {
            m_filters.append(filter);
            targetProject->take(filter->objectId());
        }
    }
    const QStringList propertyIds = collectPropertyIds();
    m_properties.clear();

    for (const QString& propertyId : propertyIds)
    {
        removeCaseReferencesForObject(propertyId);
        IDOSDataObject* detachedObject = targetProject->take(propertyId);
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(detachedObject);
        if (property != nullptr)
        {
            m_properties.append(QPointer<IDOSGridProperty>(property));
        }
        else if (detachedObject != nullptr)
        {
            targetProject->add(detachedObject);
        }
    }

    removeCaseReferencesForObject(m_gridId);
    IDOSDataObject* detachedGridObject = targetProject->take(m_gridId);
    IDOSGrid* detachedGrid = qobject_cast<IDOSGrid*>(detachedGridObject);
    if (detachedGrid == nullptr)
    {
        if (detachedGridObject != nullptr)
        {
            targetProject->add(detachedGridObject);
        }
        for (const QPointer<IDOSGridProperty>& property : m_properties)
        {
            if (!property.isNull() && property->parent() == nullptr)
            {
                targetProject->add(property.data());
            }
        }
        for (const QPointer<IDOSGridFilter>& filter : m_filters)
        {
            if (filter != nullptr && filter->parent() == nullptr)
            {
                targetProject->add(filter.data());
            }
        }
        restoreCaseReferences();
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    m_grid = detachedGrid;

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("objectId"), m_gridId);
    commandResult.insert(QStringLiteral("name"), detachedGrid->name());
    commandResult.insert(QStringLiteral("propertyCount"), m_properties.size());
    setResult(commandResult);
    return true;
}

bool IDOSDeleteGridCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_grid.isNull())
    {
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    if (m_grid->parent() != nullptr)
    {
        error = QObject::tr("The target grid is already attached to a project.");
        return false;
    }

    targetProject->add(m_grid.data());
    for (const QPointer<IDOSGridProperty>& property : m_properties)
    {
        if (!property.isNull() && property->parent() == nullptr)
        {
            targetProject->add(property.data());
        }
    }
    for (const QPointer<IDOSGridFilter>& filter : m_filters)
    {
        if (filter != nullptr && filter->parent() == nullptr)
        {
            targetProject->add(filter.data());
        }
    }
    restoreCaseReferences();
    return true;
}

void IDOSDeleteGridCommand::captureCaseReferences()
{
    m_caseReferences.clear();

    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        return;
    }

    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(object);
        if (caseObject != nullptr)
        {
            m_caseReferences.insert(caseObject->objectId(), caseObject->itemRefs());
        }
    }
}

void IDOSDeleteGridCommand::removeCaseReferencesForObject(const QString& objectId)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || objectId.isEmpty())
    {
        return;
    }

    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(object);
        if (caseObject == nullptr)
        {
            continue;
        }

        QList<IDOSCaseItemRef> refs = caseObject->itemRefs();
        bool changed = false;
        for (int i = refs.size() - 1; i >= 0; --i)
        {
            if (refs.at(i).objectId() == objectId)
            {
                refs.removeAt(i);
                changed = true;
            }
        }

        if (changed)
        {
            caseObject->setItemRefs(refs);
        }
    }
}

void IDOSDeleteGridCommand::restoreCaseReferences()
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

QStringList IDOSDeleteGridCommand::collectPropertyIds() const
{
    QStringList propertyIds;
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        return propertyIds;
    }

    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr && property->gridId() == m_gridId)
        {
            propertyIds.append(property->objectId());
        }
    }
    return propertyIds;
}

void IDOSDeleteGridCommand::clearDetachedObjects()
{
    for (const QPointer<IDOSGridFilter>& filter : m_filters)
    {
        if (filter != nullptr && filter->parent() == nullptr)
        {
            delete filter.data();
        }
    }
    for (const QPointer<IDOSGridProperty>& property : m_properties)
    {
        if (!property.isNull() && property->parent() == nullptr)
        {
            delete property.data();
        }
    }

    if (!m_grid.isNull() && m_grid->parent() == nullptr)
    {
        delete m_grid.data();
    }
}

IDOSDeleteGridCommandMetadata::IDOSDeleteGridCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("grid.delete"),
                          QObject::tr("Delete grid"),
                          QObject::tr("Delete a grid and its properties from the current project."),
                          IDOSCommand::Type::Action)
{
}

IDOSDeleteGridCommandMetadata::~IDOSDeleteGridCommandMetadata() = default;

QJsonObject IDOSDeleteGridCommandMetadata::schema() const
{
    QJsonObject gridIdProperty;
    gridIdProperty.insert(QStringLiteral("type"), QStringLiteral("string"));
    gridIdProperty.insert(QStringLiteral("description"),
                          QObject::tr("Object ID of the grid to delete."));

    QJsonObject properties;
    properties.insert(QStringLiteral("gridId"), gridIdProperty);

    QJsonArray required;
    required.append(QStringLiteral("gridId"));

    QJsonObject schemaObject;
    schemaObject.insert(QStringLiteral("type"), QStringLiteral("object"));
    schemaObject.insert(QStringLiteral("properties"), properties);
    schemaObject.insert(QStringLiteral("required"), required);
    return schemaObject;
}

IDOSCommand* IDOSDeleteGridCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    const QString gridId = arguments.value(QStringLiteral("gridId")).toString();
    return new IDOSDeleteGridCommand(project, gridId);
}
