#include <cmath>

#include <QJsonArray>
#include <QObject>
#include <QStringList>

#include "idosproject.h"

#include "idosupdateprojectmetadatacommand.h"

IDOSUpdateProjectMetadataCommand::IDOSUpdateProjectMetadataCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("project.settings.update"), Type::Action, project, QObject::tr("Update project settings"))
    , m_captured(false)
{
    QVariantMap data = arguments.value(QStringLiteral("metadata")).toObject().toVariantMap();
    const QStringList enumerationFields{QStringLiteral("unitSystem"), QStringLiteral("coordinateType"),
        QStringLiteral("lengthUnit"), QStringLiteral("zDirection")};
    for (const QString& field : enumerationFields)
    {
        if (data.contains(field))
        {
            const QJsonValue value = arguments.value(QStringLiteral("metadata")).toObject().value(field);
            if (!value.isDouble() || !std::isfinite(value.toDouble()) || value.toDouble() < 0.0 ||
                value.toDouble() > 3.0 || value.toDouble() != std::floor(value.toDouble()))
            {
                m_inputError = QObject::tr("Invalid coordinate setting: %1.").arg(field);
                return;
            }
            data[field] = value.toInt();
        }
    }
    const QStringList numbers{QStringLiteral("originX"), QStringLiteral("originY"), QStringLiteral("originZ"),
        QStringLiteral("rotationDegrees"), QStringLiteral("horizontalScale")};
    for (const QString& field : numbers)
    {
        if (data.contains(field))
        {
            const QJsonValue value = arguments.value(QStringLiteral("metadata")).toObject().value(field);
            if (!value.isDouble())
            {
                m_inputError = QObject::tr("Invalid coordinate setting: %1.").arg(field);
                return;
            }
            data[field] = value.toDouble();
        }
    }
    m_after.restoreProjectData(data, m_inputError);
}

bool IDOSUpdateProjectMetadataCommand::validateCommand(QString& error) const
{
    if (project() == nullptr || !m_inputError.isEmpty() || m_after.name().trimmed().isEmpty())
    {
        error = !m_inputError.isEmpty() ? m_inputError : QObject::tr("The project or project name is unavailable.");
        return false;
    }
    return m_after.validateCoordinates(error);
}

QJsonObject IDOSUpdateProjectMetadataCommand::buildPreview() const
{
    QJsonObject result;
    result.insert(QStringLiteral("metadata"), QJsonObject::fromVariantMap(m_after.projectData()));
    return result;
}

bool IDOSUpdateProjectMetadataCommand::apply(QString& error)
{
    if (!validateCommand(error))
    {
        return false;
    }
    if (!m_captured)
    {
        m_before = project()->metadata();
        m_captured = true;
    }
    if (!project()->setMetadata(m_after))
    {
        error = QObject::tr("The project settings could not be applied.");
        return false;
    }
    setResult(buildPreview());
    return true;
}

bool IDOSUpdateProjectMetadataCommand::revert(QString& error)
{
    if (project() == nullptr || !m_captured || !project()->setMetadata(m_before))
    {
        error = QObject::tr("The project settings could not be restored.");
        return false;
    }
    return true;
}

IDOSUpdateProjectMetadataCommandMetadata::IDOSUpdateProjectMetadataCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("project.settings.update"), QObject::tr("Update project settings"),
        QObject::tr("Update project metadata and coordinate settings without moving existing data."), IDOSCommand::Type::Action)
{
}

QJsonObject IDOSUpdateProjectMetadataCommandMetadata::schema() const
{
    QJsonObject metadata;
    metadata.insert(QStringLiteral("type"), QStringLiteral("object"));
    QJsonObject properties;
    properties.insert(QStringLiteral("metadata"), metadata);
    QJsonObject result;
    result.insert(QStringLiteral("type"), QStringLiteral("object"));
    result.insert(QStringLiteral("properties"), properties);
    result.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("metadata")});
    return result;
}

IDOSCommand* IDOSUpdateProjectMetadataCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSUpdateProjectMetadataCommand(project, arguments);
}
