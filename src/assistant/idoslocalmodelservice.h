#ifndef IDOS_LOCAL_MODEL_SERVICE_H
#define IDOS_LOCAL_MODEL_SERVICE_H

#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

#include "idos_assistant.h"

class QNetworkAccessManager;

class ASSISTANT_EXPORT IDOSLocalModelService : public QObject
{
    Q_OBJECT

public:
    explicit IDOSLocalModelService(QObject* parent = nullptr);
    ~IDOSLocalModelService() override;

    IDOSLocalModelService(const IDOSLocalModelService&) = delete;
    IDOSLocalModelService& operator=(const IDOSLocalModelService&) = delete;

    void setBaseUrl(const QUrl& baseUrl);
    QUrl baseUrl() const;

    QNetworkReply* chat(const QString& message,
                        const QString& systemMessage = QString());
    void cancel(QNetworkReply* reply);

signals:
    void responseReady(const QJsonObject& response);
    void requestFailed(const QString& error);

private slots:
    void onReplyFinished();

private:
    QNetworkRequest makeRequest(const QUrl& url) const;

    QNetworkAccessManager* m_networkManager;
    QUrl m_baseUrl;
};

#endif // IDOS_LOCAL_MODEL_SERVICE_H
