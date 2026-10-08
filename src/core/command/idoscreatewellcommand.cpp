#include <utility>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "idosproject.h"
#include "idoswell.h"

#include "command/idoscreatewellcommand.h"

IDOSCreateWellCommand::IDOSCreateWellCommand(
    IDOSProject* project,
    QString name,
    QUndoCommand* parent)
    : IDOSCommand(QStringLiteral("well.create"),
                  Type::Action,
                  project,
                  QObject::tr("Create well"),
                  parent)
    , m_wellName(std::move(name))
    , m_well(nullptr)
{
}

IDOSCreateWellCommand::~IDOSCreateWellCommand()
{
    if (!m_well.isNull() && m_well->parent() == nullptr)
    {
        delete m_well.data();
    }
}

bool IDOSCreateWellCommand::validateCommand(QString& error) const
{
    const QString name = m_wellName.trimmed();
    if (name.isEmpty())
    {
        error = QObject::tr("Please enter a well name.");
        return false;
    }

    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    for (IDOSDataObject* object : targetProject->objects())
    {
        IDOSWell* existingWell = qobject_cast<IDOSWell*>(object);
        if (existingWell != nullptr
            && existingWell != m_well
            && existingWell->name().compare(name, Qt::CaseInsensitive) == 0)
        {
            error = QObject::tr("A well named \"%1\" already exists. Please enter a different name.").arg(name);
            return false;
        }
    }

    return true;
}

QJsonObject IDOSCreateWellCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("name"), m_wellName.trimmed());
    preview.insert(QStringLiteral("type"), QStringLiteral("idos.well"));
    return preview;
}

bool IDOSCreateWellCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_well.isNull())
    {
        IDOSWell* well = new IDOSWell();
        well->setName(m_wellName.trimmed());
        m_well = well;
    }

    if (m_well->parent() != nullptr)
    {
        error = QObject::tr("The well is already attached to a project.");
        return false;
    }

    targetProject->addObject(m_well.data());

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("objectId"), m_well->objectId());
    commandResult.insert(QStringLiteral("name"), m_well->name());
    setResult(commandResult);
    return true;
}

bool IDOSCreateWellCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_well.isNull())
    {
        error = QObject::tr("The target well is unavailable.");
        return false;
    }

    IDOSDataObject* detachedObject = targetProject->takeObject(m_well->objectId());
    if (detachedObject != m_well)
    {
        error = QObject::tr("The target well is unavailable.");
        return false;
    }

    return true;
}

IDOSCreateWellCommandMetadata::IDOSCreateWellCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("well.create"),
                          QObject::tr("Create well"),
                          QObject::tr("Create a new well in the current project."),
                          IDOSCommand::Type::Action)
{
}

IDOSCreateWellCommandMetadata::~IDOSCreateWellCommandMetadata() = default;

QJsonObject IDOSCreateWellCommandMetadata::schema() const
{
    QJsonObject nameProperty;
    nameProperty.insert(QStringLiteral("type"), QStringLiteral("string"));
    nameProperty.insert(QStringLiteral("description"),
                        QObject::tr("Name of the new well."));

    QJsonObject properties;
    properties.insert(QStringLiteral("name"), nameProperty);

    QJsonArray required;
    required.append(QStringLiteral("name"));

    QJsonObject schemaObject;
    schemaObject.insert(QStringLiteral("type"), QStringLiteral("object"));
    schemaObject.insert(QStringLiteral("properties"), properties);
    schemaObject.insert(QStringLiteral("required"), required);
    return schemaObject;
}

IDOSCommand* IDOSCreateWellCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    const QString name = arguments.value(QStringLiteral("name")).toString();
    return new IDOSCreateWellCommand(project, name);
}
