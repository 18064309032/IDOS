#include <memory>

#include <QUndoStack>

#include "command/idoscommand.h"
#include "command/idoscommandregistry.h"
#include "idosproject.h"
#include "log/idoslogger.h"

#include "command/idoscommandmanager.h"

IDOSCommandManager::IDOSCommandManager(
    IDOSProject* project,
    QObject* parent)
    : QObject(parent)
    , m_project(project)
    , m_stack(project != nullptr ? project->undoStack() : nullptr)
{
    connect(m_stack,
            &QUndoStack::canUndoChanged,
            this,
            &IDOSCommandManager::onStackChanged);
    connect(m_stack,
            &QUndoStack::canRedoChanged,
            this,
            &IDOSCommandManager::onStackChanged);
    connect(m_stack,
            &QUndoStack::cleanChanged,
            this,
            &IDOSCommandManager::onStackChanged);
    connect(m_stack,
            &QUndoStack::indexChanged,
            this,
            &IDOSCommandManager::onStackChanged);
    connect(m_stack,
            &QUndoStack::undoTextChanged,
            this,
            &IDOSCommandManager::onStackChanged);
    connect(m_stack,
            &QUndoStack::redoTextChanged,
            this,
            &IDOSCommandManager::onStackChanged);
}

IDOSCommandManager::~IDOSCommandManager() = default;

IDOSProject* IDOSCommandManager::project() const
{
    return m_project.data();
}

QUndoStack* IDOSCommandManager::stack() const
{
    return m_stack.data();
}

bool IDOSCommandManager::execute(IDOSCommand* command)
{
    if (command == nullptr)
    {
        IDOS_WARN(tr("Cannot execute a null command."));
        return false;
    }

    const QString name = command->name();
    IDOS_DEBUG(tr("Executing project command: %1").arg(name));

    if (!m_project || !m_stack || command->project() != m_project)
    {
        IDOS_ERROR(tr("Command execution rejected because the command targets a different project: %1")
                       .arg(name));
        emit commandFailed(name,
                           QStringLiteral("project_mismatch"),
                           tr("The command target project does not match the manager project."));
        delete command;
        return false;
    }

    if (command->type() == IDOSCommand::Type::Query)
    {
        command->redo();

        if (command->isSuccessful())
        {
            IDOS_DEBUG(tr("Query command completed: %1").arg(name));
            emit commandExecuted(name);
        }
        else
        {
            IDOS_ERROR(tr("Query command failed: %1; %2")
                           .arg(name, command->errorString()));
            emit commandFailed(name,
                               command->errorCode(),
                               command->errorString());
        }

        const bool successful = command->isSuccessful();
        delete command;
        return successful;
    }

    m_stack->push(command);

    if (!command->isSuccessful())
    {
        IDOS_ERROR(tr("Project command failed: %1; %2")
                       .arg(name, command->errorString()));
        emit commandFailed(name,
                           command->errorCode(),
                           command->errorString());
        return false;
    }

    IDOS_INFO(tr("Project command completed: %1").arg(name));
    emit commandExecuted(name);
    return true;
}

QJsonObject IDOSCommandManager::execute(const IDOSCommandRegistry* registry,
                                        const QString& name,
                                        const QJsonObject& arguments)
{
    QJsonObject response;
    response.insert(QStringLiteral("command"), name);

    if (registry == nullptr)
    {
        response.insert(QStringLiteral("success"), false);
        response.insert(QStringLiteral("errorCode"), QStringLiteral("registry_unavailable"));
        response.insert(QStringLiteral("error"), tr("The command registry is unavailable."));
        return response;
    }

    if (!m_project)
    {
        response.insert(QStringLiteral("success"), false);
        response.insert(QStringLiteral("errorCode"), QStringLiteral("project_unavailable"));
        response.insert(QStringLiteral("error"), tr("The target project is unavailable."));
        return response;
    }

    std::unique_ptr<IDOSCommand> command = registry->create(name, arguments, m_project);
    if (!command)
    {
        response.insert(QStringLiteral("success"), false);
        response.insert(QStringLiteral("errorCode"), QStringLiteral("command_unavailable"));
        response.insert(QStringLiteral("error"), tr("The requested command is unavailable."));
        return response;
    }

    IDOSCommand* commandPointer = command.release();
    const IDOSCommand::Type commandType = commandPointer->type();
    const bool successful = execute(commandPointer);

    response.insert(QStringLiteral("success"), successful);
    if (successful)
    {
        if (commandType == IDOSCommand::Type::Action)
        {
            response.insert(QStringLiteral("result"), commandPointer->result());
        }
        return response;
    }

    if (commandType == IDOSCommand::Type::Action)
    {
        response.insert(QStringLiteral("errorCode"), commandPointer->errorCode());
        response.insert(QStringLiteral("error"), commandPointer->errorString());
    }
    else
    {
        response.insert(QStringLiteral("errorCode"), QStringLiteral("command_failed"));
        response.insert(QStringLiteral("error"), tr("The command failed."));
    }
    return response;
}

void IDOSCommandManager::undo()
{
    if (m_stack && m_stack->canUndo())
    {
        IDOS_INFO(tr("Undo command: %1").arg(m_stack->undoText()));
        m_stack->undo();
    }
}

void IDOSCommandManager::redo()
{
    if (m_stack && m_stack->canRedo())
    {
        IDOS_INFO(tr("Redo command: %1").arg(m_stack->redoText()));
        m_stack->redo();
    }
}

void IDOSCommandManager::clear()
{
    if (m_stack)
    {
        m_stack->clear();
    }
}

void IDOSCommandManager::setClean()
{
    if (m_stack)
    {
        m_stack->setClean();
    }
}

bool IDOSCommandManager::canUndo() const
{
    return m_stack && m_stack->canUndo();
}

bool IDOSCommandManager::canRedo() const
{
    return m_stack && m_stack->canRedo();
}

bool IDOSCommandManager::isClean() const
{
    return m_stack && m_stack->isClean();
}

QString IDOSCommandManager::undoText() const
{
    return m_stack ? m_stack->undoText() : QString();
}

QString IDOSCommandManager::redoText() const
{
    return m_stack ? m_stack->redoText() : QString();
}

void IDOSCommandManager::onStackChanged()
{
    emit stateChanged();
}
