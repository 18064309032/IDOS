#ifndef IDOS_COMMAND_MANAGER_H
#define IDOS_COMMAND_MANAGER_H

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>

#include "idos_core.h"

class IDOSCommand;
class IDOSCommandRegistry;
class IDOSProject;

class CORE_EXPORT IDOSCommandManager : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSCommandManager(IDOSProject* project, QObject* parent = nullptr);
    ~IDOSCommandManager() override;
    IDOSProject* project() const;
    void setRegistry(const IDOSCommandRegistry* registry);
    bool execute(IDOSCommand* command);
    QJsonObject execute(const QString& name, const QJsonObject& arguments);
    void undo();
    void redo();
    void clear();
    void setClean();
    bool canUndo() const;
    bool canRedo() const;
    bool isClean() const;
    bool contains(const QString& name) const;
    QString undoText() const;
    QString redoText() const;
    QJsonObject result() const;
    QString error() const;

  signals:
    void stateChanged();
    void commandExecuted(const QString& name);
    void commandFailed(const QString& name, const QString& errorCode,
                       const QString& errorString);

  private:
    Q_DISABLE_COPY(IDOSCommandManager)

    void saveResult(const IDOSCommand* command);

    QPointer<IDOSProject> m_project;
    const IDOSCommandRegistry* m_registry;
    QList<IDOSCommand*> m_history;
    int m_index;
    int m_cleanIndex;
    bool m_running;
    QJsonObject m_result;
    QString m_error;
    QString m_errorCode;
};

#endif
