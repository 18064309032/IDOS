#ifndef IDOS_ASSISTANT_SERVICE_H
#define IDOS_ASSISTANT_SERVICE_H

#include <memory>

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
    explicit IDOSAssistantService(QObject* parent = nullptr);
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
    void requestFailed(const QString& error);

private slots:
    void onProcessStarted();
    void onProcessStopped();
    void onProcessError(const QString& error);
    void onHealthCheckTimeout();
    void onHealthCheckFinished();
    void onModelResponse(QNetworkReply* reply, const QJsonObject& response);
    void onModelRequestFailed(QNetworkReply* reply, const QString& error);
    void onProjectDestroyed();

private:
    void publish(const QString& role, const QString& text);
    bool handleToolCalls(const QJsonObject& response, IDOSProject* project);
    bool handleTextCommandCall(const QString& text, IDOSProject* project);
    bool executeToolCall(const QJsonObject& toolCall, IDOSProject* project);
    QJsonObject createToolCall(const QString& name, const QString& arguments) const;
    QJsonObject parseArguments(const QString& text, bool& ok) const;
    QString commandSystemMessage(const QJsonArray& tools) const;
    QString responseText(const QJsonObject& response) const;
    void beginHealthCheck();
    void finishStartupWithError(const QString& error);

    std::unique_ptr<IDOSCommandRegistry> m_commandRegistry;
    QPointer<IDOSProject> m_project;
    QPointer<IDOSProject> m_requestProject;
    QPointer<QNetworkReply> m_activeReply;
    IDOSLocalModelProcess* m_process;
    IDOSLocalModelService* m_modelService;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_healthTimer;
    QPointer<QNetworkReply> m_healthReply;
    int m_healthAttempts;
    bool m_starting;
    bool m_ready;
};

#endif // IDOS_ASSISTANT_SERVICE_H
