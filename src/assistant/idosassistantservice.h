#ifndef IDOS_ASSISTANT_SERVICE_H
#define IDOS_ASSISTANT_SERVICE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QUrl>

#include "idos_assistant.h"

class IDOSCommandRegistry;
class IDOSProject;
class IDOSLocalModelProcess;
class IDOSLocalModelService;
class QNetworkAccessManager;

class ASSISTANT_EXPORT IDOSAssistantService : public QObject
{
    Q_OBJECT

public:
    explicit IDOSAssistantService(IDOSCommandRegistry* registry, QObject* parent = nullptr);
    ~IDOSAssistantService() override;

    IDOSAssistantService(const IDOSAssistantService&) = delete;
    IDOSAssistantService& operator=(const IDOSAssistantService&) = delete;

    bool start(const QString& assistantRoot, QString& error);
    void stop();
    bool isRunning() const;
    bool isReady() const;

    void setProject(IDOSProject* project);
    IDOSCommandRegistry* commandRegistry() const;
    bool send(const QString& message);
    void cancel();
    bool isBusy() const;
    void process(const QJsonObject& response, IDOSProject* project);

signals:
    void starting();
    void ready();
    void stopped();
    void message(const QString& role, const QString& text);
    void busyChanged(bool busy);
    void errorOccurred(const QString& error);
    void conversationReset();

private slots:
    void onProcessStarted();
    void onProcessStopped();
    void onProcessError(const QString& error);
    void onHealthCheckTimeout();
    void onHealthCheckFinished();
    void onModelResponse(QNetworkReply* reply, const QJsonObject& response);
    void onModelRequestFailed(QNetworkReply* reply, const QString& error);
    void onProjectDestroyed();
    void onContinue();

private:
    void publish(const QString& role, const QString& text);
    void requestNext();
    void finish(const QString& reason = QString());
    void trimHistory();
    QJsonObject executeToolCall(const QJsonObject& toolCall, IDOSProject* project);
    QJsonObject toolError(const QString& name, const QString& code, const QString& text) const;
    QString requiredTool(const QString& text) const;
    QJsonObject parseArguments(const QString& text, bool& ok) const;
    QString commandSystemMessage(const QJsonArray& tools) const;
    void beginHealthCheck();
    void finishStartupWithError(const QString& error);

    QPointer<IDOSCommandRegistry> m_commandRegistry;
    QPointer<IDOSProject> m_project;
    QPointer<IDOSProject> m_requestProject;
    QPointer<QNetworkReply> m_activeReply;
    IDOSLocalModelProcess* m_process;
    IDOSLocalModelService* m_modelService;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_healthTimer;
    QTimer* m_stepTimer;
    QJsonArray m_messages;
    QJsonArray m_tools;
    QJsonArray m_pendingCalls;
    int m_callIndex;
    int m_callCount;
    quint64 m_generation;
    bool m_busy;
    bool m_executing;
    bool m_cancelled;
    QPointer<QNetworkReply> m_healthReply;
    int m_healthAttempts;
    bool m_starting;
    bool m_ready;
};

#endif // IDOS_ASSISTANT_SERVICE_H
