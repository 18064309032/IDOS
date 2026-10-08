#include <utility>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "idoscaseobject.h"
#include "idosproject.h"
#include "idoswell.h"

#include "command/idosdeletewellcommand.h"

IDOSDeleteWellCommand::IDOSDeleteWellCommand(
    IDOSProject* project,
    QString wellId,
    QUndoCommand* parent)
    : IDOSCommand(QStringLiteral("well.delete"),
                  Type::Action,
                  project,
                  QObject::tr("Delete well"),
                  parent)
    , m_wellId(std::move(wellId))
    , m_well(nullptr)
{
}

IDOSDeleteWellCommand::~IDOSDeleteWellCommand()
{
    if (!m_well.isNull() && m_well->parent() == nullptr)
    {
        delete m_well.data();
    }
}

bool IDOSDeleteWellCommand::validateCommand(QString& error) const
{
    if (m_wellId.isEmpty())
    {
        error = QObject::tr("The target well is unavailable.");
        return false;
    }

    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    IDOSDataObject* object = targetProject->objectById(m_wellId);
    if (object == nullptr && m_well.isNull())
    {
        error = QObject::tr("The target well is unavailable.");
        return false;
    }

    if (object != nullptr && qobject_cast<IDOSWell*>(object) == nullptr)
    {
        error = QObject::tr("The target object is not a well.");
        return false;
    }

    return true;
}

QJsonObject IDOSDeleteWellCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("objectId"), m_wellId);
    preview.insert(QStringLiteral("type"), QStringLiteral("idos.well"));
    return preview;
}

bool IDOSDeleteWellCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    captureCaseReferences();
    removeCaseReferences();

    IDOSDataObject* detachedObject = targetProject->takeObject(m_wellId);
    IDOSWell* detachedWell = qobject_cast<IDOSWell*>(detachedObject);
    if (detachedWell == nullptr)
    {
        if (detachedObject != nullptr)
        {
            targetProject->addObject(detachedObject);
        }
        restoreCaseReferences();
        error = QObject::tr("The target well is unavailable.");
        return false;
    }

    m_well = detachedWell;

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("objectId"), m_wellId);
    commandResult.insert(QStringLiteral("name"), detachedWell->name());
    setResult(commandResult);
    return true;
}

bool IDOSDeleteWellCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_well.isNull())
    {
        error = QObject::tr("The target well is unavailable.");
        return false;
    }

    if (m_well->parent() != nullptr)
    {
        error = QObject::tr("The target well is already attached to a project.");
        return false;
    }

    targetProject->addObject(m_well.data());
    restoreCaseReferences();
    return true;
}

void IDOSDeleteWellCommand::captureCaseReferences()
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

void IDOSDeleteWellCommand::removeCaseReferences()
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
            if (refs.at(i).objectId() == m_wellId)
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

void IDOSDeleteWellCommand::restoreCaseReferences()
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

IDOSDeleteWellCommandMetadata::IDOSDeleteWellCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("well.delete"),
                          QObject::tr("Delete well"),
                          QObject::tr("Delete a well from the current project."),
                          IDOSCommand::Type::Action)
{
}

IDOSDeleteWellCommandMetadata::~IDOSDeleteWellCommandMetadata() = default;

QJsonObject IDOSDeleteWellCommandMetadata::schema() const
{
    QJsonObject wellIdProperty;
    wellIdProperty.insert(QStringLiteral("type"), QStringLiteral("string"));
    wellIdProperty.insert(QStringLiteral("description"),
                          QObject::tr("Object ID of the well to delete."));

    QJsonObject properties;
    properties.insert(QStringLiteral("wellId"), wellIdProperty);

    QJsonArray required;
    required.append(QStringLiteral("wellId"));

    QJsonObject schemaObject;
    schemaObject.insert(QStringLiteral("type"), QStringLiteral("object"));
    schemaObject.insert(QStringLiteral("properties"), properties);
    schemaObject.insert(QStringLiteral("required"), required);
    return schemaObject;
}

IDOSCommand* IDOSDeleteWellCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    const QString wellId = arguments.value(QStringLiteral("wellId")).toString();
    return new IDOSDeleteWellCommand(project, wellId);
}
