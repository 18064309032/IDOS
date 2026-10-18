#include <utility>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "data/case/idoscaseobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosproject.h"

#include "idosdeletepropertycommand.h"

IDOSDeletePropertyCommand::IDOSDeletePropertyCommand(
    IDOSProject* project,
    QString propertyId)
    :
    IDOSCommand(QStringLiteral("property.delete"),
                  Type::Action,
                  project,
                  QObject::tr("Delete property"))
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

    IDOSDataObject* object = targetProject->object(m_propertyId);
    if (object == nullptr && m_property.isNull())
    {
        error = QObject::tr("The target property is unavailable.");
        return false;
    }

    if (object != nullptr && qobject_cast<IDOSGridPropertyObject*>(object) == nullptr)
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

    IDOSDataObject* detachedObject = targetProject->take(m_propertyId);
    IDOSGridPropertyObject* detachedProperty = qobject_cast<IDOSGridPropertyObject*>(detachedObject);
    if (detachedProperty == nullptr)
    {
        if (detachedObject != nullptr)
        {
            targetProject->add(detachedObject);
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

    if (!targetProject->add(m_property.data()))
    {
        error = QObject::tr("The target property is unavailable.");
        return false;
    }
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
            qobject_cast<IDOSCaseObject*>(targetProject->object(iterator.key()));
        if (caseObject != nullptr)
        {
            caseObject->setItemRefs(iterator.value());
        }
        ++iterator;
    }
}

IDOSDeletePropertyCommandMetadata::IDOSDeletePropertyCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("property.delete"),
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
