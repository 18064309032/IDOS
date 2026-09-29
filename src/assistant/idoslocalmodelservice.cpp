#include "idoslocalmodelservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

IDOSLocalModelService::IDOSLocalModelService(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_baseUrl(QStringLiteral("http://127.0.0.1:18080"))
{
}

IDOSLocalModelService::~IDOSLocalModelService()
{
}

void IDOSLocalModelService::setBaseUrl(const QUrl& baseUrl)
{
    m_baseUrl = baseUrl;
}

QUrl IDOSLocalModelService::baseUrl() const
{
    return m_baseUrl;
}

QNetworkReply* IDOSLocalModelService::chat(const QString& message,
                                           const QString& systemMessage)
{
    QJsonArray messages;
    if (!systemMessage.isEmpty())
    {
        QJsonObject system;
        system.insert(QStringLiteral("role"), QStringLiteral("system"));
        system.insert(QStringLiteral("content"), systemMessage);
        messages.append(system);
    }

    QJsonObject user;
    user.insert(QStringLiteral("role"), QStringLiteral("user"));
    user.insert(QStringLiteral("content"), message);
    messages.append(user);

    QJsonObject requestObject;
    requestObject.insert(QStringLiteral("messages"), messages);
    requestObject.insert(QStringLiteral("stream"), false);

    const QUrl requestUrl = m_baseUrl.resolved(QUrl(QStringLiteral("/v1/chat/completions")));
    QNetworkReply* reply = m_networkManager->post(
        makeRequest(requestUrl),
        QJsonDocument(requestObject).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished,
            this, &IDOSLocalModelService::onReplyFinished);
    return reply;
}

void IDOSLocalModelService::cancel(QNetworkReply* reply)
{
    if (reply != nullptr)
    {
        reply->abort();
    }
}

void IDOSLocalModelService::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply == nullptr)
    {
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        emit requestFailed(reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        emit requestFailed(QStringLiteral("Invalid model response: %1").arg(parseError.errorString()));
        reply->deleteLater();
        return;
    }

    emit responseReady(document.object());
    reply->deleteLater();
}

QNetworkRequest IDOSLocalModelService::makeRequest(const QUrl& url) const
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    return request;
}
