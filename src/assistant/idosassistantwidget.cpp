#include <QCoreApplication>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
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
    const QFont interfaceFont = font();
    m_transcript->setFont(interfaceFont);
    m_transcript->document()->setDefaultFont(interfaceFont);
    m_input->setObjectName(QStringLiteral("assistantInput"));
    m_input->setPlaceholderText(tr("Enter a message (Shift+Enter for a new line, Enter to send)"));
    m_input->setFont(interfaceFont);
    m_input->setFixedHeight(44);
    m_input->installEventFilter(this);
    m_sendButton->setObjectName(QStringLiteral("assistantSendButton"));
    m_sendButton->setFixedHeight(44);
    m_statusLabel->setObjectName(QStringLiteral("assistantStatusLabel"));

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
    onControlsChanged();
}

void IDOSAssistantWidget::onRequestFailed(const QString& error)
{
    onMessage(tr("System"), error);
    onControlsChanged();
}

void IDOSAssistantWidget::onMessage(const QString& role, const QString& message)
{
    QTextCursor cursor(m_transcript->document());
    cursor.movePosition(QTextCursor::End);
    if (!m_transcript->document()->isEmpty())
    {
        cursor.insertBlock();
        cursor.insertBlock();
    }
    cursor.insertText(tr("%1: %2").arg(role, message));
    m_transcript->setTextCursor(cursor);
    m_transcript->ensureCursorVisible();
}

void IDOSAssistantWidget::onControlsChanged()
{
    m_sendButton->setEnabled(!m_service->isBusy() && !m_startupPending);
}
