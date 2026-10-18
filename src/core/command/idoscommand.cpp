#include <utility>

#include <QObject>

#include "idosproject.h"

#include "idoscommand.h"

IDOSCommand::IDOSCommand(QString name,
                         Type type,
                         IDOSProject* project,
                         QString text)
    :
    m_name(std::move(name))
    , m_text(std::move(text))
    , m_type(type)
    , m_project(project)
{
}

IDOSCommand::~IDOSCommand() = default;

const QString& IDOSCommand::name() const
{
    return m_name;
}

QString IDOSCommand::text() const
{
    return m_text;
}

IDOSCommand::Type IDOSCommand::type() const
{
    return m_type;
}

IDOSProject* IDOSCommand::project() const
{
    return m_project.data();
}

bool IDOSCommand::validate(QString& error) const
{
    error.clear();

    if (m_project == nullptr)
    {
        error = QObject::tr("The command has no target project.");
        return false;
    }

    return validateCommand(error);
}

QJsonObject IDOSCommand::preview() const
{
    return buildPreview();
}

bool IDOSCommand::isSuccessful() const
{
    return m_errorCode.isEmpty();
}

const QString& IDOSCommand::errorCode() const
{
    return m_errorCode;
}

const QString& IDOSCommand::errorString() const
{
    return m_errorString;
}

const QJsonObject& IDOSCommand::result() const
{
    return m_result;
}

bool IDOSCommand::undo()
{
    QString error;

    if (m_project == nullptr)
    {
        setError(QStringLiteral("revert_failed"), QObject::tr("The command has no target project."));
        return false;
    }
    m_project->beginUpdate();
    const bool reverted = revert(error);
    if (m_project != nullptr)
    {
        m_project->endUpdate();
    }
    if (!reverted)
    {
        setError(QStringLiteral("revert_failed"), error);
        return false;
    }

    m_errorCode.clear();
    m_errorString.clear();
    return true;
}

bool IDOSCommand::execute()
{
    QString error;

    if (!validate(error))
    {
        setError(QStringLiteral("validation_failed"), error);
        return false;
    }

    if (m_type == Type::Action)
    {
        m_project->beginUpdate();
    }
    const bool applied = apply(error);
    if (m_type == Type::Action && m_project != nullptr)
    {
        m_project->endUpdate();
    }
    if (!applied)
    {
        setError(QStringLiteral("apply_failed"), error);
        return false;
    }

    m_errorCode.clear();
    m_errorString.clear();
    return true;
}

bool IDOSCommand::validateCommand(QString& error) const
{
    error.clear();
    return true;
}

QJsonObject IDOSCommand::buildPreview() const
{
    return QJsonObject();
}

void IDOSCommand::setResult(QJsonObject result)
{
    m_result = std::move(result);
}

void IDOSCommand::setError(QString errorCode, QString errorString)
{
    m_errorCode = std::move(errorCode);
    m_errorString = std::move(errorString);
}
