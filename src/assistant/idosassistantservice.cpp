#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>

#include "idoscommandmanager.h"
#include "idoscommandregistry.h"
#include "idoslocalmodelprocess.h"
#include "idoslocalmodelservice.h"
#include "idosproject.h"

#include "idosassistantservice.h"

IDOSAssistantService::IDOSAssistantService(QObject* parent)
    :
    QObject(parent)
    , m_commandRegistry(new IDOSCommandRegistry())
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
            &IDOSAssistantService::onProcessStarted);
    connect(m_process,
            &IDOSLocalModelProcess::stopped,
            this,
            &IDOSAssistantService::onProcessStopped);
    connect(m_process,
            &IDOSLocalModelProcess::errorOccurred,
            this,
            &IDOSAssistantService::onProcessError);
    connect(m_healthTimer,
            &QTimer::timeout,
            this,
            &IDOSAssistantService::onHealthCheckTimeout);
    connect(m_modelService,
            &IDOSLocalModelService::responseReady,
            this,
            &IDOSAssistantService::onModelResponse);
    connect(m_modelService,
            &IDOSLocalModelService::requestFailed,
            this,
            &IDOSAssistantService::onModelRequestFailed);
}

IDOSAssistantService::~IDOSAssistantService()
{
    setProject(nullptr);
    stop();
}

bool IDOSAssistantService::start(const QString& assistantRoot, QString& error)
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

void IDOSAssistantService::stop()
{
    cancel();
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

bool IDOSAssistantService::isRunning() const
{
    return m_process->isRunning();
}

bool IDOSAssistantService::isReady() const
{
    return m_ready;
}

void IDOSAssistantService::setProject(IDOSProject* project)
{
    if (m_project == project)
    {
        return;
    }
    cancel();
    if (m_project != nullptr)
    {
        disconnect(m_project, nullptr, this, nullptr);
        m_project->commandManager()->setRegistry(nullptr);
    }
    m_project = project;
    if (project != nullptr)
    {
        project->commandManager()->setRegistry(m_commandRegistry.get());
        connect(project, &QObject::destroyed, this, &IDOSAssistantService::onProjectDestroyed);
    }
}

IDOSCommandRegistry* IDOSAssistantService::commandRegistry() const
{
    return m_commandRegistry.get();
}

bool IDOSAssistantService::send(const QString& text)
{
    if (!m_ready || isBusy() || text.trimmed().isEmpty())
    {
        return false;
    }
    m_requestProject = m_project;
    const QJsonArray tools = m_project != nullptr ? m_commandRegistry->toToolJson() : QJsonArray();
    m_activeReply = m_modelService->chat(text, commandSystemMessage(tools), tools);
    emit busyChanged(isBusy());
    return m_activeReply != nullptr;
}

bool IDOSAssistantService::isBusy() const
{
    return m_activeReply != nullptr;
}

void IDOSAssistantService::cancel()
{
    QNetworkReply* reply = m_activeReply.data();
    m_activeReply.clear();
    m_requestProject.clear();
    m_modelService->cancel(reply);
    emit busyChanged(false);
}

void IDOSAssistantService::onProjectDestroyed()
{
    m_project.clear();
    cancel();
}

void IDOSAssistantService::publish(const QString& role, const QString& text)
{
    emit message(role, text);
}

void IDOSAssistantService::process(const QJsonObject& response, IDOSProject* project)
{
    if (project != m_project)
    {
        return;
    }
    QPointer<IDOSProject> target(project);
    const bool handledToolCall = handleToolCalls(response, target.data());
    const QString text = responseText(response);
    const bool handledTextCommandCall = !handledToolCall && handleTextCommandCall(text, target.data());
    if (!text.isEmpty() && !handledTextCommandCall)
    {
        publish(tr("Assistant"), text);
    }
    else if (!handledToolCall && !handledTextCommandCall)
    {
        publish(tr("Assistant"), tr("No response content."));
    }
}
void IDOSAssistantService::onProcessStarted()
{
    beginHealthCheck();
}

void IDOSAssistantService::onProcessStopped()
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

void IDOSAssistantService::onProcessError(const QString& error)
{
    finishStartupWithError(error);
}

void IDOSAssistantService::onHealthCheckTimeout()
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
            &IDOSAssistantService::onHealthCheckFinished);
}

void IDOSAssistantService::onHealthCheckFinished()
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

void IDOSAssistantService::onModelResponse(QNetworkReply* reply, const QJsonObject& response)
{
    if (reply == nullptr || reply != m_activeReply)
    {
        return;
    }
    QPointer<IDOSProject> target = m_requestProject;
    m_activeReply.clear();
    m_requestProject.clear();
    process(response, target.data());
    emit busyChanged(isBusy());
}

void IDOSAssistantService::onModelRequestFailed(QNetworkReply* reply, const QString& error)
{
    if (reply == nullptr || reply != m_activeReply)
    {
        return;
    }
    m_activeReply.clear();
    m_requestProject.clear();
    emit requestFailed(error);
    emit busyChanged(false);
}
void IDOSAssistantService::beginHealthCheck()
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

void IDOSAssistantService::finishStartupWithError(const QString& error)
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

bool IDOSAssistantService::handleToolCalls(const QJsonObject& response, IDOSProject* project)
{
    const QJsonArray choices = response.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty())
    {
        return false;
    }

    const QJsonObject choice = choices.first().toObject();
    const QJsonObject message = choice.value(QStringLiteral("message")).toObject();
    const QJsonArray toolCalls = message.value(QStringLiteral("tool_calls")).toArray();
    if (toolCalls.isEmpty())
    {
        return false;
    }

    QPointer<IDOSProject> target(project);
    bool handled = false;
    for (QJsonArray::const_iterator iterator = toolCalls.constBegin();
         iterator != toolCalls.constEnd();
         ++iterator)
    {
        const QJsonObject toolCall = iterator->toObject();
        if (executeToolCall(toolCall, target.data()))
        {
            handled = true;
        }
    }
    return handled;
}

bool IDOSAssistantService::handleTextCommandCall(const QString& text, IDOSProject* project)
{
    QString commandText = text.trimmed();
    if (commandText.isEmpty())
    {
        return false;
    }

    if (commandText.startsWith(QStringLiteral("```")))
    {
        commandText.remove(QStringLiteral("```"));
        commandText = commandText.trimmed();
    }

    if (commandText.startsWith(QStringLiteral("json")))
    {
        commandText = commandText.mid(QStringLiteral("json").length()).trimmed();
    }

    const int openIndex = commandText.indexOf(QStringLiteral("("));
    const int closeIndex = commandText.lastIndexOf(QStringLiteral(")"));
    if (openIndex <= 0 || closeIndex <= openIndex)
    {
        return false;
    }

    const QString commandName = commandText.left(openIndex).trimmed();
    const QString argumentsText =
        commandText.mid(openIndex + 1, closeIndex - openIndex - 1).trimmed();
    const bool isProjectCommand =
        m_commandRegistry != nullptr && m_commandRegistry->find(commandName) != nullptr;
    if (!isProjectCommand)
    {
        return false;
    }

    return executeToolCall(createToolCall(commandName, argumentsText), project);
}

bool IDOSAssistantService::executeToolCall(const QJsonObject& toolCall, IDOSProject* project)
{
    if (project != m_project)
    {
        return false;
    }
    const QJsonObject functionObject = toolCall.value(QStringLiteral("function")).toObject();
    const QString name = functionObject.value(QStringLiteral("name")).toString();
    const QString argumentsText = functionObject.value(QStringLiteral("arguments")).toString();
    if (name.trimmed().isEmpty())
    {
        publish(tr("System"), tr("The assistant requested an unnamed command."));
        return false;
    }

    bool argumentsOk = false;
    const QJsonObject arguments = parseArguments(argumentsText, argumentsOk);
    if (!argumentsOk)
    {
        publish(tr("System"), tr("The assistant provided invalid command arguments."));
        return false;
    }

    if (project == nullptr)
    {
        publish(tr("System"), tr("No project is available for command execution."));
        return false;
    }

    if (!m_commandRegistry)
    {
        publish(tr("System"), tr("No command registry is available."));
        return false;
    }

    IDOSCommandManager* commandManager = project->commandManager();
    if (commandManager == nullptr)
    {
        publish(tr("System"), tr("No command manager is available."));
        return false;
    }

    const QJsonObject result =
        commandManager->execute(name, arguments);
    const QJsonDocument document(result);
    publish(tr("Tool"), QString::fromUtf8(document.toJson(QJsonDocument::Compact)));
    return result.value(QStringLiteral("success")).toBool();
}

QJsonObject IDOSAssistantService::createToolCall(const QString& commandName,
                                                const QString& argumentsText) const
{
    QJsonObject functionObject;
    functionObject.insert(QStringLiteral("name"), commandName);
    functionObject.insert(QStringLiteral("arguments"), argumentsText);

    QJsonObject toolCall;
    toolCall.insert(QStringLiteral("type"), QStringLiteral("function"));
    toolCall.insert(QStringLiteral("function"), functionObject);
    return toolCall;
}

QJsonObject IDOSAssistantService::parseArguments(const QString& argumentsText,
                                                bool& ok) const
{
    ok = false;
    const QString trimmedArguments = argumentsText.trimmed();
    if (trimmedArguments.isEmpty())
    {
        ok = true;
        return QJsonObject();
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(trimmedArguments.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        return QJsonObject();
    }

    ok = true;
    return document.object();
}

QString IDOSAssistantService::commandSystemMessage(const QJsonArray& tools) const
{
    if (tools.isEmpty())
    {
        return QStringLiteral(
            "You are the IDOS assistant. No project command tools are currently available. "
            "If the user asks to create, delete, import, undo, redo, or modify project data, "
            "ask them to create or open a project first.");
    }

    QStringList commandNames;
    for (QJsonArray::const_iterator iterator = tools.constBegin();
         iterator != tools.constEnd();
         ++iterator)
    {
        const QJsonObject toolObject = iterator->toObject();
        const QJsonObject functionObject = toolObject.value(QStringLiteral("function")).toObject();
        const QString commandName = functionObject.value(QStringLiteral("name")).toString();
        if (!commandName.isEmpty())
        {
            commandNames.append(commandName);
        }
    }

    return QStringLiteral(
        "You are the IDOS assistant. The available IDOS project commands are: %1. "
        "When the user asks to create, delete, import, or otherwise change project data, "
        "call the matching tool instead of only explaining the action. "
        "For example, if the user asks to create a well named A10, call well.create with "
        "{\"name\":\"A10\"}. "
        "If the user asks what commands are available, answer from the available command list.")
        .arg(commandNames.join(QStringLiteral(", ")));
}

QString IDOSAssistantService::responseText(const QJsonObject& response) const
{
    const QJsonArray choices = response.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty())
    {
        return QString();
    }

    const QJsonObject choice = choices.first().toObject();
    const QJsonObject message = choice.value(QStringLiteral("message")).toObject();
    return message.value(QStringLiteral("content")).toString();
}

