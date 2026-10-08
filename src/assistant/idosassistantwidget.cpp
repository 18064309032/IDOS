#include <QCoreApplication>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkRequest>
#include <QPushButton>
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>

#include "command/idoscommandmanager.h"
#include "command/idoscommandregistry.h"
#include "idoslocalassistantservice.h"
#include "idosproject.h"

#include "idosassistantwidget.h"

IDOSAssistantWidget::IDOSAssistantWidget(QWidget* parent)
    : QWidget(parent)
    , m_service(new IDOSLocalAssistantService(this))
    , m_commandRegistry(new IDOSCommandRegistry())
    , m_project(nullptr)
    , m_transcript(new QTextEdit(this))
    , m_input(new QLineEdit(this))
    , m_startButton(new QPushButton(tr("Start"), this))
    , m_stopButton(new QPushButton(tr("Stop"), this))
    , m_sendButton(new QPushButton(tr("Send"), this))
    , m_statusLabel(new QLabel(tr("Stopped"), this))
    , m_activeReply(nullptr)
{
    m_transcript->setReadOnly(true);
    m_input->setPlaceholderText(tr("Enter a message"));

    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_stopButton);
    controlLayout->addWidget(m_statusLabel, 1);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_input, 1);
    inputLayout->addWidget(m_sendButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(controlLayout);
    layout->addWidget(m_transcript, 1);
    layout->addLayout(inputLayout);

    connect(m_startButton,
            &QPushButton::clicked,
            this,
            &IDOSAssistantWidget::onStartClicked);
    connect(m_stopButton,
            &QPushButton::clicked,
            this,
            &IDOSAssistantWidget::onStopClicked);
    connect(m_sendButton,
            &QPushButton::clicked,
            this,
            &IDOSAssistantWidget::onSendClicked);
    connect(m_input,
            &QLineEdit::returnPressed,
            this,
            &IDOSAssistantWidget::onSendClicked);
    connect(m_service,
            &IDOSLocalAssistantService::ready,
            this,
            &IDOSAssistantWidget::onAssistantReady);
    connect(m_service,
            &IDOSLocalAssistantService::stopped,
            this,
            &IDOSAssistantWidget::onAssistantStopped);
    connect(m_service,
            &IDOSLocalAssistantService::responseReady,
            this,
            &IDOSAssistantWidget::onAssistantResponse);
    connect(m_service,
            &IDOSLocalAssistantService::errorOccurred,
            this,
            &IDOSAssistantWidget::onAssistantError);
    connect(m_service,
            &IDOSLocalAssistantService::requestFailed,
            this,
            &IDOSAssistantWidget::onRequestFailed);

    updateControls();
}

IDOSAssistantWidget::~IDOSAssistantWidget()
{
}

void IDOSAssistantWidget::setProject(IDOSProject* project)
{
    m_project = project;
}

IDOSCommandRegistry* IDOSAssistantWidget::commandRegistry() const
{
    return m_commandRegistry.get();
}

void IDOSAssistantWidget::onStartClicked()
{
    const QString assistantRoot = QCoreApplication::applicationDirPath()
        + QStringLiteral("/assistant");
    QString error;
    if (!m_service->start(assistantRoot, error))
    {
        onAssistantError(error);
        return;
    }

    m_statusLabel->setText(tr("Starting"));
    updateControls();
}

void IDOSAssistantWidget::onStopClicked()
{
    m_service->stop();
    updateControls();
}

void IDOSAssistantWidget::onSendClicked()
{
    const QString message = m_input->text().trimmed();
    if (message.isEmpty() || !m_service->isReady())
    {
        return;
    }

    appendMessage(tr("You"), message);
    m_input->clear();
    m_sendButton->setEnabled(false);
    QJsonArray tools;
    if (m_commandRegistry)
    {
        const QJsonArray projectTools = m_commandRegistry->toToolJson();
        for (QJsonArray::const_iterator iterator = projectTools.constBegin();
             iterator != projectTools.constEnd();
             ++iterator)
        {
            tools.append(*iterator);
        }
    }

    m_activeReply = m_service->chat(message, commandSystemMessage(tools), tools);
}

void IDOSAssistantWidget::onAssistantReady()
{
    m_statusLabel->setText(tr("Ready"));
    emit statusChanged(m_statusLabel->text());
    updateControls();
}

void IDOSAssistantWidget::onAssistantStopped()
{
    m_statusLabel->setText(tr("Stopped"));
    emit statusChanged(m_statusLabel->text());
    updateControls();
}

void IDOSAssistantWidget::onAssistantResponse(const QJsonObject& response)
{
    const bool handledToolCall = handleToolCalls(response);
    const QString text = responseText(response);
    const bool handledTextCommandCall = !handledToolCall && handleTextCommandCall(text);
    if (!text.isEmpty() && !handledTextCommandCall)
    {
        appendMessage(tr("Assistant"), text);
    }
    else if (!handledToolCall && !handledTextCommandCall)
    {
        appendMessage(tr("Assistant"), tr("No response content."));
    }

    m_activeReply.clear();
    updateControls();
}

void IDOSAssistantWidget::onAssistantError(const QString& error)
{
    m_statusLabel->setText(tr("Error"));
    appendMessage(tr("System"), error);
    emit statusChanged(m_statusLabel->text());
    updateControls();
}

void IDOSAssistantWidget::onRequestFailed(const QString& error)
{
    appendMessage(tr("System"), error);
    m_activeReply.clear();
    updateControls();
}

void IDOSAssistantWidget::appendMessage(const QString& role, const QString& message)
{
    m_transcript->append(QStringLiteral("<b>%1:</b> %2")
                             .arg(role.toHtmlEscaped(), message.toHtmlEscaped()));
}

bool IDOSAssistantWidget::handleToolCalls(const QJsonObject& response)
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

    bool handled = false;
    for (QJsonArray::const_iterator iterator = toolCalls.constBegin();
         iterator != toolCalls.constEnd();
         ++iterator)
    {
        const QJsonObject toolCall = iterator->toObject();
        if (executeToolCall(toolCall))
        {
            handled = true;
        }
    }
    return handled;
}

bool IDOSAssistantWidget::handleTextCommandCall(const QString& text)
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

    return executeToolCall(createToolCall(commandName, argumentsText));
}

bool IDOSAssistantWidget::executeToolCall(const QJsonObject& toolCall)
{
    const QJsonObject functionObject = toolCall.value(QStringLiteral("function")).toObject();
    const QString name = functionObject.value(QStringLiteral("name")).toString();
    const QString argumentsText = functionObject.value(QStringLiteral("arguments")).toString();
    if (name.trimmed().isEmpty())
    {
        appendMessage(tr("System"), tr("The assistant requested an unnamed command."));
        return false;
    }

    bool argumentsOk = false;
    const QJsonObject arguments = parseArguments(argumentsText, argumentsOk);
    if (!argumentsOk)
    {
        appendMessage(tr("System"), tr("The assistant provided invalid command arguments."));
        return false;
    }

    if (m_project == nullptr)
    {
        appendMessage(tr("System"), tr("No project is available for command execution."));
        return false;
    }

    if (!m_commandRegistry)
    {
        appendMessage(tr("System"), tr("No command registry is available."));
        return false;
    }

    IDOSCommandManager* commandManager = m_project->commandManager();
    if (commandManager == nullptr)
    {
        appendMessage(tr("System"), tr("No command manager is available."));
        return false;
    }

    const QJsonObject result =
        commandManager->execute(m_commandRegistry.get(), name, arguments);
    const QJsonDocument document(result);
    appendMessage(tr("Tool"), QString::fromUtf8(document.toJson(QJsonDocument::Compact)));
    return result.value(QStringLiteral("success")).toBool();
}

QJsonObject IDOSAssistantWidget::createToolCall(const QString& commandName,
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

QJsonObject IDOSAssistantWidget::parseArguments(const QString& argumentsText,
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

QString IDOSAssistantWidget::commandSystemMessage(const QJsonArray& tools) const
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

QString IDOSAssistantWidget::responseText(const QJsonObject& response) const
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

void IDOSAssistantWidget::updateControls()
{
    const bool running = m_service->isRunning();
    const bool ready = m_service->isReady();
    m_startButton->setEnabled(!running);
    m_stopButton->setEnabled(running);
    m_sendButton->setEnabled(ready && m_activeReply.isNull());
}
