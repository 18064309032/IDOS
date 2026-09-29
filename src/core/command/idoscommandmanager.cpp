#include "command/idoscommandmanager.h"

#include <QUndoStack>

#include "command/idoscommand.h"
#include "idosproject.h"

IDOSCommandManager::IDOSCommandManager(
    IDOSProject& project,
    QObject* parent)
    : QObject(parent)
    , m_project(&project)
    , m_stack(project.undoStack())
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
        return false;
    }

    const QString name = command->name();

    if (!m_project || !m_stack || command->project() != m_project)
    {
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
            emit commandExecuted(name);
        }
        else
        {
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
        emit commandFailed(name,
                           command->errorCode(),
                           command->errorString());
        return false;
    }

    emit commandExecuted(name);
    return true;
}

void IDOSCommandManager::undo()
{
    if (m_stack && m_stack->canUndo())
    {
        m_stack->undo();
    }
}

void IDOSCommandManager::redo()
{
    if (m_stack && m_stack->canRedo())
    {
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
