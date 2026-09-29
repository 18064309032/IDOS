#ifndef IDOS_LOCAL_ASSISTANT_SERVICE_H
#define IDOS_LOCAL_ASSISTANT_SERVICE_H

#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QUrl>

#include "idos_assistant.h"

class IDOSLocalModelProcess;
class IDOSLocalModelService;
class QNetworkAccessManager;

class ASSISTANT_EXPORT IDOSLocalAssistantService : public QObject
{
    Q_OBJECT

public:
    explicit IDOSLocalAssistantService(QObject* parent = nullptr);
    ~IDOSLocalAssistantService() override;

    IDOSLocalAssistantService(const IDOSLocalAssistantService&) = delete;
    IDOSLocalAssistantService& operator=(const IDOSLocalAssistantService&) = delete;

    bool start(const QString& assistantRoot, QString& error);
    void stop();
    bool isRunning() const;
    bool isReady() const;

    QNetworkReply* chat(const QString& message,
                        const QString& systemMessage = QString());
    void cancel(QNetworkReply* reply);

signals:
    void starting();
    void ready();
    void stopped();
    void responseReady(const QJsonObject& response);
    void errorOccurred(const QString& error);
    void requestFailed(const QString& error);

private slots:
    void onProcessStarted();
    void onProcessStopped();
    void onProcessError(const QString& error);
    void onHealthCheckTimeout();
    void onHealthCheckFinished();
    void onModelResponse(const QJsonObject& response);
    void onModelRequestFailed(const QString& error);

private:
    void beginHealthCheck();
    void finishStartupWithError(const QString& error);

    IDOSLocalModelProcess* m_process;
    IDOSLocalModelService* m_modelService;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_healthTimer;
    QPointer<QNetworkReply> m_healthReply;
    int m_healthAttempts;
    bool m_starting;
    bool m_ready;
};

#endif // IDOS_LOCAL_ASSISTANT_SERVICE_H
