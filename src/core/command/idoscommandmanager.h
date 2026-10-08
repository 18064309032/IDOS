#ifndef IDOS_COMMAND_MANAGER_H
#define IDOS_COMMAND_MANAGER_H

#include <QObject>
#include <QPointer>
#include <QJsonObject>

#include "idos_core.h"

class IDOSCommand;
class IDOSCommandRegistry;
class IDOSProject;
class QUndoStack;

/**
 * @brief Executes project commands through the project undo stack.
 */
class CORE_EXPORT IDOSCommandManager : public QObject
{
    Q_OBJECT

public:
    explicit IDOSCommandManager(IDOSProject* project,
                                QObject* parent = nullptr);
    ~IDOSCommandManager() override;

    IDOSCommandManager(const IDOSCommandManager&) = delete;
    IDOSCommandManager& operator=(const IDOSCommandManager&) = delete;

    IDOSProject* project() const;
    QUndoStack* stack() const;

    bool execute(IDOSCommand* command);
    QJsonObject execute(const IDOSCommandRegistry* registry,
                        const QString& name,
                        const QJsonObject& arguments);
    void undo();
    void redo();
    void clear();
    void setClean();

    bool canUndo() const;
    bool canRedo() const;
    bool isClean() const;
    QString undoText() const;
    QString redoText() const;

signals:
    void stateChanged();
    void commandExecuted(const QString& name);
    void commandFailed(const QString& name,
                       const QString& errorCode,
                       const QString& errorString);

private slots:
    void onStackChanged();

private:
    QPointer<IDOSProject> m_project;
    QPointer<QUndoStack> m_stack;
};

#endif // IDOS_COMMAND_MANAGER_H
