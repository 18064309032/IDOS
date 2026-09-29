#include "idosassistantwidget.h"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "idoslocalassistantservice.h"

IDOSAssistantWidget::IDOSAssistantWidget(QWidget* parent)
    : QWidget(parent)
    , m_service(new IDOSLocalAssistantService(this))
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
    m_activeReply = m_service->chat(message);
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
    const QString text = responseText(response);
    if (!text.isEmpty())
    {
        appendMessage(tr("Assistant"), text);
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
