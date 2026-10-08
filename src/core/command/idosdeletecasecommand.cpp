#include <utility>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "idoscaseitemref.h"
#include "idoscaseobject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"

#include "command/idosdeletecasecommand.h"

IDOSDeleteCaseCommand::IDOSDeleteCaseCommand(
    IDOSProject* project,
    QString caseId,
    QUndoCommand* parent)
    : IDOSCommand(QStringLiteral("case.delete"),
                  Type::Action,
                  project,
                  QObject::tr("Delete case"),
                  parent)
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

    IDOSDataObject* object = targetProject->objectById(m_caseId);
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
    preview.insert(QStringLiteral("type"), QStringLiteral("idos.case"));
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
    m_grids.clear();
    m_properties.clear();

    for (const QString& gridId : gridIds)
    {
        const QStringList propertyIds = collectGridPropertyIds(gridId);
        for (const QString& propertyId : propertyIds)
        {
            IDOSDataObject* detachedObject = targetProject->takeObject(propertyId);
            IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(detachedObject);
            if (property != nullptr)
            {
                m_properties.append(QPointer<IDOSGridProperty>(property));
            }
            else if (detachedObject != nullptr)
            {
                targetProject->addObject(detachedObject);
            }
        }

        IDOSDataObject* detachedGridObject = targetProject->takeObject(gridId);
        IDOSGrid* grid = qobject_cast<IDOSGrid*>(detachedGridObject);
        if (grid != nullptr)
        {
            m_grids.append(QPointer<IDOSGrid>(grid));
        }
        else if (detachedGridObject != nullptr)
        {
            targetProject->addObject(detachedGridObject);
        }
    }

    IDOSDataObject* detachedCaseObject = targetProject->takeObject(m_caseId);
    IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(detachedCaseObject);
    if (caseObject == nullptr)
    {
        if (detachedCaseObject != nullptr)
        {
            targetProject->addObject(detachedCaseObject);
        }
        for (const QPointer<IDOSGrid>& grid : m_grids)
        {
            if (!grid.isNull() && grid->parent() == nullptr)
            {
                targetProject->addObject(grid.data());
            }
        }
        for (const QPointer<IDOSGridProperty>& property : m_properties)
        {
            if (!property.isNull() && property->parent() == nullptr)
            {
                targetProject->addObject(property.data());
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
            targetProject->addObject(grid.data());
        }
    }

    for (const QPointer<IDOSGridProperty>& property : m_properties)
    {
        if (!property.isNull() && property->parent() == nullptr)
        {
            targetProject->addObject(property.data());
        }
    }

    if (m_caseObject->parent() != nullptr)
    {
        error = QObject::tr("The target case is already attached to a project.");
        return false;
    }

    targetProject->addObject(m_caseObject.data());
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
        qobject_cast<IDOSCaseObject*>(targetProject->objectById(m_caseId));
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
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            propertyIds.append(property->objectId());
        }
    }
    return propertyIds;
}

void IDOSDeleteCaseCommand::clearDetachedObjects()
{
    for (const QPointer<IDOSGridProperty>& property : m_properties)
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
                          QObject::tr("Delete a simulation case and its private grid data."),
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
