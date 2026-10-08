#include "command/idosrenameobjectcommand.h"

#include <utility>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "idosdataobject.h"
#include "idosproject.h"

class IDOSRenameObjectCommandJsonHelpers
{
public:
    static QJsonObject stringPropertySchema(const QString& description);
};

IDOSRenameObjectCommand::IDOSRenameObjectCommand(IDOSProject* project,
                                                 QString objectId,
                                                 QString newName,
                                                 QUndoCommand* parent)
    : IDOSCommand(QStringLiteral("object.rename"),
                  Type::Action,
                  project,
                  QObject::tr("Rename object"),
                  parent)
    , m_objectId(std::move(objectId))
    , m_newName(std::move(newName))
    , m_oldName()
    , m_oldNameCaptured(false)
{
}

IDOSRenameObjectCommand::~IDOSRenameObjectCommand() = default;

bool IDOSRenameObjectCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_objectId.trimmed().isEmpty())
    {
        error = QObject::tr("The target object is unavailable.");
        return false;
    }

    if (m_newName.trimmed().isEmpty())
    {
        error = QObject::tr("Please enter an object name.");
        return false;
    }

    if (project()->objectById(m_objectId) == nullptr)
    {
        error = QObject::tr("The target object is unavailable.");
        return false;
    }

    return true;
}

QJsonObject IDOSRenameObjectCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("objectId"), m_objectId);
    preview.insert(QStringLiteral("name"), m_newName);
    return preview;
}

bool IDOSRenameObjectCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    IDOSDataObject* object = targetProject->objectById(m_objectId);
    if (object == nullptr)
    {
        error = QObject::tr("The target object is unavailable.");
        return false;
    }

    if (!m_oldNameCaptured)
    {
        m_oldName = object->name();
        m_oldNameCaptured = true;
    }

    object->setName(m_newName.trimmed());

    QJsonObject result;
    result.insert(QStringLiteral("objectId"), m_objectId);
    result.insert(QStringLiteral("oldName"), m_oldName);
    result.insert(QStringLiteral("name"), object->name());
    setResult(result);
    return true;
}

bool IDOSRenameObjectCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    IDOSDataObject* object = targetProject->objectById(m_objectId);
    if (object == nullptr)
    {
        error = QObject::tr("The target object is unavailable.");
        return false;
    }

    object->setName(m_oldName);
    return true;
}

IDOSRenameObjectCommandMetadata::IDOSRenameObjectCommandMetadata()
    : IDOSCommandMetadata(QStringLiteral("object.rename"),
                          QObject::tr("Rename object"),
                          QObject::tr("Rename an object in the current project."),
                          IDOSCommand::Type::Action)
{
}

IDOSRenameObjectCommandMetadata::~IDOSRenameObjectCommandMetadata() = default;

QJsonObject IDOSRenameObjectCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("objectId"),
                      IDOSRenameObjectCommandJsonHelpers::stringPropertySchema(
                          QObject::tr("Object ID of the object to rename.")));
    properties.insert(QStringLiteral("name"),
                      IDOSRenameObjectCommandJsonHelpers::stringPropertySchema(
                          QObject::tr("New object name.")));

    QJsonArray required;
    required.append(QStringLiteral("objectId"));
    required.append(QStringLiteral("name"));

    QJsonObject schemaObject;
    schemaObject.insert(QStringLiteral("type"), QStringLiteral("object"));
    schemaObject.insert(QStringLiteral("properties"), properties);
    schemaObject.insert(QStringLiteral("required"), required);
    return schemaObject;
}

IDOSCommand* IDOSRenameObjectCommandMetadata::create(const QJsonObject& arguments,
                                                     IDOSProject* project) const
{
    return new IDOSRenameObjectCommand(project,
                                       arguments.value(QStringLiteral("objectId")).toString(),
                                       arguments.value(QStringLiteral("name")).toString());
}

QJsonObject IDOSRenameObjectCommandJsonHelpers::stringPropertySchema(const QString& description)
{
    QJsonObject property;
    property.insert(QStringLiteral("type"), QStringLiteral("string"));
    property.insert(QStringLiteral("description"), description);
    return property;
}
