#include <utility>

#include "idosproject.h"

#include "command/idoscommand.h"

IDOSCommand::IDOSCommand(QString name,
                         Type type,
                         IDOSProject* project,
                         QString text,
                         QUndoCommand* parent)
    : QUndoCommand(std::move(text), parent)
    , m_name(std::move(name))
    , m_type(type)
    , m_project(project)
{
}

IDOSCommand::~IDOSCommand() = default;

const QString& IDOSCommand::name() const
{
    return m_name;
}

IDOSCommand::Type IDOSCommand::type() const
{
    return m_type;
}

IDOSProject* IDOSCommand::project() const
{
    return m_project;
}

bool IDOSCommand::validate(QString& error) const
{
    error.clear();

    if (m_project == nullptr)
    {
        error = QStringLiteral("The command has no target project.");
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

void IDOSCommand::undo()
{
    QString error;

    if (!revert(error))
    {
        setError(QStringLiteral("revert_failed"), error);
        return;
    }

    m_errorCode.clear();
    m_errorString.clear();
}

void IDOSCommand::redo()
{
    QString error;

    if (!validate(error))
    {
        setError(QStringLiteral("validation_failed"), error);
        return;
    }

    if (!apply(error))
    {
        setError(QStringLiteral("apply_failed"), error);
        return;
    }

    m_errorCode.clear();
    m_errorString.clear();
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
