#include "command/idosdeletepropertycommand.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <utility>

#include "idoscaseobject.h"
#include "idosgridproperty.h"
#include "idosproject.h"

IDOSDeletePropertyCommand::IDOSDeletePropertyCommand(
    IDOSProject* project,
    QString propertyId,
    QUndoCommand* parent)
    : IDOSCommand(QStringLiteral("property.delete"),
                  Type::Action,
                  project,
                  QObject::tr("Delete property"),
                  parent)
    , m_propertyId(std::move(propertyId))
    , m_property(nullptr)
{
}

IDOSDeletePropertyCommand::~IDOSDeletePropertyCommand()
{
    if (!m_property.isNull() && m_property->parent() == nullptr)
    {
        delete m_property.data();
    }
}

bool IDOSDeletePropertyCommand::validateCommand(QString& error) const
{
    if (m_propertyId.isEmpty())
    {
        error = QObject::tr("The target property is unavailable.");
        return false;
    }

    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    IDOSDataObject* object = targetProject->objectById(m_propertyId);
    if (object == nullptr && m_property.isNull())
    {
        error = QObject::tr("The target property is unavailable.");
        return false;
    }

    if (object != nullptr && qobject_cast<IDOSGridProperty*>(object) == nullptr)
    {
        error = QObject::tr("The target object is not a property.");
        return false;
    }

    return true;
}

QJsonObject IDOSDeletePropertyCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("objectId"), m_propertyId);
    preview.insert(QStringLiteral("type"), QStringLiteral("idos.grid.property"));
    return preview;
}

bool IDOSDeletePropertyCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    captureCaseReferences();
    removeCaseReferences();

    IDOSDataObject* detachedObject = targetProject->takeObject(m_propertyId);
    IDOSGridProperty* detachedProperty = qobject_cast<IDOSGridProperty*>(detachedObject);
    if (detachedProperty == nullptr)
    {
        if (detachedObject != nullptr)
        {
            targetProject->addObject(detachedObject);
        }
        restoreCaseReferences();
        error = QObject::tr("The target property is unavailable.");
        return false;
    }

    m_property = detachedProperty;

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("objectId"), m_propertyId);
    commandResult.insert(QStringLiteral("name"), detachedProperty->name());
    setResult(commandResult);
    return true;
}

bool IDOSDeletePropertyCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_property.isNull())
    {
        error = QObject::tr("The target property is unavailable.");
        return false;
    }

    if (m_property->parent() != nullptr)
    {
        error = QObject::tr("The target property is already attached to a project.");
        return false;
    }

    targetProject->addObject(m_property.data());
    restoreCaseReferences();
    return true;
}

void IDOSDeletePropertyCommand::captureCaseReferences()
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

void IDOSDeletePropertyCommand::removeCaseReferences()
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
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
            if (refs.at(i).objectId() == m_propertyId)
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

void IDOSDeletePropertyCommand::restoreCaseReferences()
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

IDOSDeletePropertyCommandMetadata::IDOSDeletePropertyCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("property.delete"),
                          QObject::tr("Delete property"),
                          QObject::tr("Delete a grid property from the current project."),
                          IDOSCommand::Type::Action)
{
}

IDOSDeletePropertyCommandMetadata::~IDOSDeletePropertyCommandMetadata() = default;

QJsonObject IDOSDeletePropertyCommandMetadata::schema() const
{
    QJsonObject propertyIdProperty;
    propertyIdProperty.insert(QStringLiteral("type"), QStringLiteral("string"));
    propertyIdProperty.insert(QStringLiteral("description"),
                              QObject::tr("Object ID of the property to delete."));

    QJsonObject properties;
    properties.insert(QStringLiteral("propertyId"), propertyIdProperty);

    QJsonArray required;
    required.append(QStringLiteral("propertyId"));

    QJsonObject schemaObject;
    schemaObject.insert(QStringLiteral("type"), QStringLiteral("object"));
    schemaObject.insert(QStringLiteral("properties"), properties);
    schemaObject.insert(QStringLiteral("required"), required);
    return schemaObject;
}

IDOSCommand* IDOSDeletePropertyCommandMetadata::create(const QJsonObject& arguments,
                                                       IDOSProject* project) const
{
    const QString propertyId = arguments.value(QStringLiteral("propertyId")).toString();
    return new IDOSDeletePropertyCommand(project, propertyId);
}
