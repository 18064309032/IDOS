#include <memory>

#include "idoscommand.h"
#include "idoscommandregistry.h"
#include "idosproject.h"

#include "idoscommandmanager.h"

IDOSCommandManager::IDOSCommandManager(IDOSProject* project, QObject* parent)
    :
    QObject(parent)
    , m_project(project)
    , m_registry(nullptr)
    , m_index(0)
    , m_cleanIndex(0)
    , m_running(false)
{
}

IDOSCommandManager::~IDOSCommandManager()
{
    qDeleteAll(m_history);
}

IDOSProject* IDOSCommandManager::project() const
{
    return m_project.data();
}

void IDOSCommandManager::setRegistry(const IDOSCommandRegistry* registry)
{
    if (!m_running)
    {
        m_registry = registry;
    }
}

bool IDOSCommandManager::execute(IDOSCommand* command)
{
    if (command == nullptr)
    {
        m_result = QJsonObject();
        m_errorCode = QStringLiteral("null_command");
        m_error = tr("Cannot execute a null command.");
        return false;
    }
    if (m_running)
    {
        delete command;
        return false;
    }
    if (!m_project || command->project() != m_project)
    {
        m_result = QJsonObject();
        m_errorCode = QStringLiteral("project_mismatch");
        m_error = tr("The command target project does not match the manager project.");
        delete command;
        return false;
    }
    m_running = true;
    command->execute();
    saveResult(command);
    const QString name = command->name();
    const bool successful = command->isSuccessful();
    if (successful && command->type() == IDOSCommand::Type::Action)
    {
        if (m_cleanIndex > m_index)
        {
            m_cleanIndex = -1;
        }
        while (m_history.size() > m_index)
        {
            delete m_history.takeLast();
        }
        m_history.append(command);
        ++m_index;
    }
    else
    {
        delete command;
    }
    emit stateChanged();
    if (successful)
    {
        emit commandExecuted(name);
    }
    else
    {
        emit commandFailed(name, m_errorCode, m_error);
    }
    m_running = false;
    return successful;
}

QJsonObject IDOSCommandManager::execute(const QString& name,
                                       const QJsonObject& arguments)
{
    QJsonObject response;
    response.insert(QStringLiteral("command"), name);
    if (m_registry == nullptr || !m_project)
    {
        m_result = QJsonObject();
        m_errorCode = m_registry == nullptr ? QStringLiteral("registry_unavailable")
                                         :
                                         QStringLiteral("project_unavailable");
        m_error = m_registry == nullptr ? tr("The command registry is unavailable.")
                                     : tr("The target project is unavailable.");
        response.insert(QStringLiteral("success"), false);
        response.insert(QStringLiteral("errorCode"), m_errorCode);
        response.insert(QStringLiteral("error"), m_error);
        return response;
    }
    std::unique_ptr<IDOSCommand> command = m_registry->create(name, arguments, m_project);
    if (!command)
    {
        m_result = QJsonObject();
        m_errorCode = QStringLiteral("command_unavailable");
        m_error = tr("The requested command is unavailable.");
        response.insert(QStringLiteral("success"), false);
        response.insert(QStringLiteral("errorCode"), m_errorCode);
        response.insert(QStringLiteral("error"), m_error);
        return response;
    }
    const bool successful = execute(command.release());
    response.insert(QStringLiteral("success"), successful);
    if (successful)
    {
        response.insert(QStringLiteral("result"), m_result);
    }
    else
    {
        response.insert(QStringLiteral("errorCode"), m_errorCode);
        response.insert(QStringLiteral("error"), m_error);
    }
    return response;
}

void IDOSCommandManager::saveResult(const IDOSCommand* command)
{
    m_result = command->isSuccessful() ? command->result() : QJsonObject();
    m_error = command->errorString();
    m_errorCode = command->errorCode();
}

void IDOSCommandManager::undo()
{
    if (!canUndo() || m_running)
    {
        return;
    }
    m_running = true;
    IDOSCommand* command = m_history.at(m_index - 1);
    command->undo();
    saveResult(command);
    const QString name = command->name();
    const bool successful = command->isSuccessful();
    if (successful)
    {
        --m_index;
    }
    emit stateChanged();
    if (!successful)
    {
        emit commandFailed(name, m_errorCode, m_error);
    }
    m_running = false;
}

void IDOSCommandManager::redo()
{
    if (!canRedo() || m_running)
    {
        return;
    }
    m_running = true;
    IDOSCommand* command = m_history.at(m_index);
    command->execute();
    saveResult(command);
    const QString name = command->name();
    const bool successful = command->isSuccessful();
    if (successful)
    {
        ++m_index;
    }
    emit stateChanged();
    if (!successful)
    {
        emit commandFailed(name, m_errorCode, m_error);
    }
    m_running = false;
}

void IDOSCommandManager::clear()
{
    if (m_running)
    {
        return;
    }
    qDeleteAll(m_history);
    m_history.clear();
    m_index = 0;
    m_cleanIndex = 0;
    m_result = QJsonObject();
    m_error.clear();
    m_errorCode.clear();
    emit stateChanged();
}

void IDOSCommandManager::setClean()
{
    if (!m_running)
    {
        m_cleanIndex = m_index;
        emit stateChanged();
    }
}

bool IDOSCommandManager::canUndo() const
{
    return m_project && m_index > 0;
}

bool IDOSCommandManager::canRedo() const
{
    return m_project && m_index < m_history.size();
}

bool IDOSCommandManager::isClean() const
{
    return m_index == m_cleanIndex;
}

QString IDOSCommandManager::undoText() const
{
    return canUndo() ? m_history.at(m_index - 1)->text() : QString();
}

QString IDOSCommandManager::redoText() const
{
    return canRedo() ? m_history.at(m_index)->text() : QString();
}

QJsonObject IDOSCommandManager::result() const
{
    return m_result;
}

QString IDOSCommandManager::error() const
{
    return m_error;
}

bool IDOSCommandManager::contains(const QString& name) const
{
    for (const IDOSCommand* command : m_history)
    {
        if (command->name() == name)
        {
            return true;
        }
    }
    return false;
}
