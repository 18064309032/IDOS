#include "idoslocalassistantservice.h"

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "idoslocalmodelprocess.h"
#include "idoslocalmodelservice.h"

IDOSLocalAssistantService::IDOSLocalAssistantService(QObject* parent)
    : QObject(parent)
    , m_process(new IDOSLocalModelProcess(this))
    , m_modelService(new IDOSLocalModelService(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_healthTimer(new QTimer(this))
    , m_healthReply(nullptr)
    , m_healthAttempts(0)
    , m_starting(false)
    , m_ready(false)
{
    m_healthTimer->setInterval(500);
    m_healthTimer->setSingleShot(false);

    connect(m_process,
            &IDOSLocalModelProcess::started,
            this,
            &IDOSLocalAssistantService::onProcessStarted);
    connect(m_process,
            &IDOSLocalModelProcess::stopped,
            this,
            &IDOSLocalAssistantService::onProcessStopped);
    connect(m_process,
            &IDOSLocalModelProcess::errorOccurred,
            this,
            &IDOSLocalAssistantService::onProcessError);
    connect(m_healthTimer,
            &QTimer::timeout,
            this,
            &IDOSLocalAssistantService::onHealthCheckTimeout);
    connect(m_modelService,
            &IDOSLocalModelService::responseReady,
            this,
            &IDOSLocalAssistantService::onModelResponse);
    connect(m_modelService,
            &IDOSLocalModelService::requestFailed,
            this,
            &IDOSLocalAssistantService::onModelRequestFailed);
}

IDOSLocalAssistantService::~IDOSLocalAssistantService()
{
    stop();
}

bool IDOSLocalAssistantService::start(const QString& assistantRoot, QString& error)
{
    if (isReady())
    {
        return true;
    }

    if (m_starting)
    {
        error = QStringLiteral("The local assistant is already starting.");
        return false;
    }

    m_starting = true;
    m_ready = false;
    m_healthAttempts = 0;
    emit starting();

    if (!m_process->start(assistantRoot, error))
    {
        m_starting = false;
        emit errorOccurred(error);
        return false;
    }

    m_modelService->setBaseUrl(QUrl(m_process->baseUrl()));
    beginHealthCheck();
    return true;
}

void IDOSLocalAssistantService::stop()
{
    m_healthTimer->stop();
    if (m_healthReply != nullptr)
    {
        m_healthReply->abort();
        m_healthReply->deleteLater();
        m_healthReply.clear();
    }

    const bool wasActive = m_starting || m_ready || m_process->isRunning();
    m_starting = false;
    m_ready = false;
    m_process->stop();
    if (wasActive)
    {
        emit stopped();
    }
}

bool IDOSLocalAssistantService::isRunning() const
{
    return m_process->isRunning();
}

bool IDOSLocalAssistantService::isReady() const
{
    return m_ready;
}

QNetworkReply* IDOSLocalAssistantService::chat(const QString& message,
                                               const QString& systemMessage,
                                               const QJsonArray& tools)
{
    if (!m_ready)
    {
        emit requestFailed(QStringLiteral("The local assistant is not ready."));
        return nullptr;
    }

    return m_modelService->chat(message, systemMessage, tools);
}

void IDOSLocalAssistantService::cancel(QNetworkReply* reply)
{
    m_modelService->cancel(reply);
}

void IDOSLocalAssistantService::onProcessStarted()
{
    beginHealthCheck();
}

void IDOSLocalAssistantService::onProcessStopped()
{
    m_healthTimer->stop();
    m_healthReply.clear();
    const bool wasActive = m_starting || m_ready;
    m_starting = false;
    m_ready = false;
    if (wasActive)
    {
        emit stopped();
    }
}

void IDOSLocalAssistantService::onProcessError(const QString& error)
{
    finishStartupWithError(error);
}

void IDOSLocalAssistantService::onHealthCheckTimeout()
{
    if (!m_starting || m_healthReply != nullptr)
    {
        return;
    }

    ++m_healthAttempts;
    if (m_healthAttempts > 60)
    {
        finishStartupWithError(QStringLiteral("The local model service did not become ready."));
        return;
    }

    const QUrl healthUrl = QUrl(m_process->baseUrl()).resolved(QUrl(QStringLiteral("/health")));
    m_healthReply = m_networkManager->get(QNetworkRequest(healthUrl));
    connect(m_healthReply,
            &QNetworkReply::finished,
            this,
            &IDOSLocalAssistantService::onHealthCheckFinished);
}

void IDOSLocalAssistantService::onHealthCheckFinished()
{
    QNetworkReply* reply = m_healthReply.data();
    m_healthReply.clear();
    if (reply == nullptr)
    {
        return;
    }

    const bool healthy = reply->error() == QNetworkReply::NoError
        && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200;
    reply->deleteLater();
    if (healthy)
    {
        m_healthTimer->stop();
        m_starting = false;
        m_ready = true;
        emit ready();
    }
}

void IDOSLocalAssistantService::onModelResponse(const QJsonObject& response)
{
    emit responseReady(response);
}

void IDOSLocalAssistantService::onModelRequestFailed(const QString& error)
{
    emit requestFailed(error);
}

void IDOSLocalAssistantService::beginHealthCheck()
{
    if (!m_starting)
    {
        return;
    }

    if (!m_healthTimer->isActive())
    {
        m_healthTimer->start();
    }
    onHealthCheckTimeout();
}

void IDOSLocalAssistantService::finishStartupWithError(const QString& error)
{
    if (!m_starting)
    {
        emit errorOccurred(error);
        return;
    }

    m_healthTimer->stop();
    m_starting = false;
    m_ready = false;
    m_process->stop();
    emit errorOccurred(error);
}
