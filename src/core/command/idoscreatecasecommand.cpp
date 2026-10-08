#include "command/idoscreatecasecommand.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <utility>

#include "idossimulationcaseobject.h"
#include "idosproject.h"

IDOSCreateCaseCommand::IDOSCreateCaseCommand(
    IDOSProject* project,
    QString name,
    QUndoCommand* parent)
    : IDOSCommand(QStringLiteral("case.create"),
                  Type::Action,
                  project,
                  QObject::tr("Create case"),
                  parent)
    , m_caseName(std::move(name))
    , m_caseObject(nullptr)
{
}

IDOSCreateCaseCommand::~IDOSCreateCaseCommand()
{
    if (!m_caseObject.isNull() && m_caseObject->parent() == nullptr)
    {
        delete m_caseObject.data();
    }
}

bool IDOSCreateCaseCommand::validateCommand(QString& error) const
{
    const QString name = m_caseName.trimmed();
    if (name.isEmpty())
    {
        error = QObject::tr("Please enter a case name.");
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
        IDOSSimulationCaseObject* existingCase = qobject_cast<IDOSSimulationCaseObject*>(object);
        if (existingCase != nullptr
            && existingCase != m_caseObject
            && existingCase->name().trimmed().compare(name, Qt::CaseInsensitive) == 0)
        {
            error = QObject::tr("A case with this name already exists.");
            return false;
        }
    }

    return true;
}

QJsonObject IDOSCreateCaseCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("name"), m_caseName.trimmed());
    preview.insert(QStringLiteral("type"), QStringLiteral("idos.case.simulation"));
    return preview;
}

bool IDOSCreateCaseCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_caseObject.isNull())
    {
        IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
        caseObject->setName(m_caseName.trimmed());
        m_caseObject = caseObject;
    }

    if (m_caseObject->parent() != nullptr)
    {
        error = QObject::tr("The case is already attached to a project.");
        return false;
    }

    targetProject->addObject(m_caseObject.data());

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("objectId"), m_caseObject->objectId());
    commandResult.insert(QStringLiteral("name"), m_caseObject->name());
    commandResult.insert(QStringLiteral("type"), m_caseObject->caseTypeId());
    setResult(commandResult);
    return true;
}

bool IDOSCreateCaseCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr || m_caseObject.isNull())
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    IDOSDataObject* detachedObject = targetProject->takeObject(m_caseObject->objectId());
    if (detachedObject != m_caseObject)
    {
        error = QObject::tr("The target case is unavailable.");
        return false;
    }

    return true;
}

IDOSCreateCaseCommandMetadata::IDOSCreateCaseCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("case.create"),
                          QObject::tr("Create case"),
                          QObject::tr("Create a new simulation case in the current project."),
                          IDOSCommand::Type::Action)
{
}

IDOSCreateCaseCommandMetadata::~IDOSCreateCaseCommandMetadata() = default;

QJsonObject IDOSCreateCaseCommandMetadata::schema() const
{
    QJsonObject nameProperty;
    nameProperty.insert(QStringLiteral("type"), QStringLiteral("string"));
    nameProperty.insert(QStringLiteral("description"),
                        QObject::tr("Name of the new simulation case."));

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

IDOSCommand* IDOSCreateCaseCommandMetadata::create(const QJsonObject& arguments,
                                                   IDOSProject* project) const
{
    const QString name = arguments.value(QStringLiteral("name")).toString();
    return new IDOSCreateCaseCommand(project, name);
}
