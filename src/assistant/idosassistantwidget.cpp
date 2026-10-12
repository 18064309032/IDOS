#include <QCoreApplication>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

#include "idosassistantservice.h"
#include "log/idoslogger.h"

#include "idosassistantwidget.h"

IDOSAssistantWidget::IDOSAssistantWidget(QWidget* parent)
    :
    QWidget(parent)
    , m_service(new IDOSAssistantService(this))
    , m_transcript(new QTextBrowser(this))
    , m_input(new QTextEdit(this))
    , m_sendButton(new QPushButton(tr("Send"), this))
    , m_statusLabel(new QLabel(tr("Stopped"), this))
    , m_startupPending(false)
{
    m_transcript->setReadOnly(true);
    m_transcript->setObjectName(QStringLiteral("assistantTranscript"));
    m_transcript->setOpenLinks(false);
    m_transcript->setOpenExternalLinks(false);
    m_transcript->setFrameShape(QFrame::NoFrame);
    m_input->setObjectName(QStringLiteral("assistantInput"));
    m_input->setPlaceholderText(tr("Enter a message (Shift+Enter for a new line, Enter to send)"));
    m_input->setFixedHeight(76);
    m_input->installEventFilter(this);
    m_sendButton->setObjectName(QStringLiteral("assistantSendButton"));
    m_sendButton->setMinimumHeight(36);
    m_statusLabel->setObjectName(QStringLiteral("assistantStatusLabel"));
    m_statusLabel->setStyleSheet(QStringLiteral("color: #687386; padding: 4px 2px;"));
    setStyleSheet(QStringLiteral(
        "QTextBrowser#assistantTranscript { background: #ffffff; border: 1px solid #d6dbe4; border-radius: 4px; }"
        "QTextEdit#assistantInput { background: #ffffff; border: 1px solid #c9d0da; border-radius: 4px; padding: 6px; }"));

    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->setContentsMargins(2, 0, 2, 0);
    controlLayout->addWidget(m_statusLabel, 1);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);
    inputLayout->addWidget(m_input, 1);
    inputLayout->addWidget(m_sendButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 10);
    layout->setSpacing(8);
    layout->addLayout(controlLayout);
    layout->addWidget(m_transcript, 1);
    layout->addLayout(inputLayout);

    connect(m_sendButton,
            &QPushButton::clicked,
            this,
            &IDOSAssistantWidget::onSendClicked);
    connect(m_service,
            &IDOSAssistantService::ready,
            this,
            &IDOSAssistantWidget::onAssistantReady);
    connect(m_service,
            &IDOSAssistantService::stopped,
            this,
            &IDOSAssistantWidget::onAssistantStopped);
    connect(m_service, &IDOSAssistantService::message, this, &IDOSAssistantWidget::onMessage);
    connect(m_service, &IDOSAssistantService::busyChanged, this, &IDOSAssistantWidget::onControlsChanged);
    connect(m_service,
            &IDOSAssistantService::errorOccurred,
            this,
            &IDOSAssistantWidget::onAssistantError);
    connect(m_service,
            &IDOSAssistantService::requestFailed,
            this,
            &IDOSAssistantWidget::onRequestFailed);

    onControlsChanged();
}

IDOSAssistantWidget::~IDOSAssistantWidget()
{
}

bool IDOSAssistantWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_input && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        const bool isEnterKey = keyEvent->key() == Qt::Key_Return
                                || keyEvent->key() == Qt::Key_Enter;
        const bool shouldSend = isEnterKey
                                && (keyEvent->modifiers() & Qt::ShiftModifier) == 0;
        if (shouldSend)
        {
            onSendClicked();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void IDOSAssistantWidget::setProject(IDOSProject* project)
{
    m_service->setProject(project);
}

IDOSAssistantService* IDOSAssistantWidget::service() const
{
    return m_service;
}

void IDOSAssistantWidget::startAssistant()
{
    if (m_service->isReady() || m_startupPending)
    {
        return;
    }

    const QString assistantRoot = QCoreApplication::applicationDirPath()
        + QStringLiteral("/assistant");
    QString error;
    m_startupPending = true;
    m_statusLabel->setText(tr("Starting"));
    if (!m_service->start(assistantRoot, error))
    {
        if (m_startupPending)
        {
            onAssistantError(error);
        }
        return;
    }

    onControlsChanged();
}

void IDOSAssistantWidget::onSendClicked()
{
    const QString message = m_input->toPlainText().trimmed();
    if (message.isEmpty())
    {
        return;
    }

    if (!m_service->isReady())
    {
        if (m_startupPending)
        {
            m_pendingMessage = message;
            m_statusLabel->setText(tr("Starting"));
            onMessage(tr("System"),
                      tr("AI assistant is starting. Your message will be sent when it is ready."));
            return;
        }

        m_pendingMessage = message;
        startAssistant();
        if (m_startupPending)
        {
            onMessage(tr("System"),
                      tr("AI assistant is starting. Your message will be sent when it is ready."));
        }
        onControlsChanged();
        return;
    }

    if (m_service->send(message))
    {
        onMessage(tr("You"), message);
        m_input->clear();
    }
    onControlsChanged();
}
void IDOSAssistantWidget::onAssistantReady()
{
    m_startupPending = false;
    m_statusLabel->setText(tr("Ready"));
    IDOS_MESSAGE(tr("AI assistant started successfully."), IDOSLogLevel::Info);
    emit statusChanged(m_statusLabel->text());
    if (!m_pendingMessage.isEmpty())
    {
        const QString message = m_pendingMessage;
        m_pendingMessage.clear();
        if (m_service->send(message))
        {
            onMessage(tr("You"), message);
            m_input->clear();
        }
    }
    onControlsChanged();
}

void IDOSAssistantWidget::onAssistantStopped()
{
    m_startupPending = false;
    m_statusLabel->setText(tr("Stopped"));
    emit statusChanged(m_statusLabel->text());
    onControlsChanged();
}

void IDOSAssistantWidget::onAssistantError(const QString& error)
{
    const bool startupFailure = m_startupPending;
    m_startupPending = false;
    m_pendingMessage.clear();
    m_statusLabel->setText(tr("Error"));
    onMessage(tr("System"), error);
    if (startupFailure)
    {
        IDOS_MESSAGE(tr("AI assistant failed to start: %1").arg(error),
                     IDOSLogLevel::Error);
    }
    else
    {
        IDOS_MESSAGE(tr("AI assistant encountered an error: %1").arg(error),
                     IDOSLogLevel::Error);
    }
    emit statusChanged(m_statusLabel->text());
    onControlsChanged();
}

void IDOSAssistantWidget::onRequestFailed(const QString& error)
{
    onMessage(tr("System"), error);
    onControlsChanged();
}

void IDOSAssistantWidget::onMessage(const QString& role, const QString& message)
{
    const QString escapedMessage = message.toHtmlEscaped().replace(
        QLatin1Char('\n'), QStringLiteral("<br>"));
    QString messageHtml;
    if (role == tr("You"))
    {
        messageHtml = QStringLiteral(
                          "<table align=\"right\" cellspacing=\"0\" cellpadding=\"8\">"
                          "<tr><td bgcolor=\"#e8f1ff\"><b>%1</b><br>%2</td></tr></table><br clear=\"all\">")
                          .arg(role.toHtmlEscaped(), escapedMessage);
    }
    else if (role == tr("System"))
    {
        messageHtml = QStringLiteral(
                          "<p style=\"color:#7b8494;\"><i>%1: %2</i></p>")
                          .arg(role.toHtmlEscaped(), escapedMessage);
    }
    else
    {
        messageHtml = QStringLiteral(
                          "<table align=\"left\" cellspacing=\"0\" cellpadding=\"8\">"
                          "<tr><td bgcolor=\"#f1f3f6\"><b>%1</b><br>%2</td></tr></table><br clear=\"all\">")
                          .arg(role.toHtmlEscaped(), escapedMessage);
    }

    m_transcript->append(messageHtml);
    m_transcript->moveCursor(QTextCursor::End);
}

void IDOSAssistantWidget::onControlsChanged()
{
    m_sendButton->setEnabled(!m_service->isBusy() && !m_startupPending);
}
