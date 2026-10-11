#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "idosassistantservice.h"

#include "idosassistantwidget.h"

IDOSAssistantWidget::IDOSAssistantWidget(QWidget* parent)
    :
    QWidget(parent)
    , m_service(new IDOSAssistantService(this))
    , m_transcript(new QTextEdit(this))
    , m_input(new QLineEdit(this))
    , m_startButton(new QPushButton(tr("Start"), this))
    , m_stopButton(new QPushButton(tr("Stop"), this))
    , m_sendButton(new QPushButton(tr("Send"), this))
    , m_statusLabel(new QLabel(tr("Stopped"), this))
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

void IDOSAssistantWidget::setProject(IDOSProject* project)
{
    m_service->setProject(project);
}

IDOSAssistantService* IDOSAssistantWidget::service() const
{
    return m_service;
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
    onControlsChanged();
}

void IDOSAssistantWidget::onStopClicked()
{
    m_service->stop();
    onControlsChanged();
}

void IDOSAssistantWidget::onSendClicked()
{
    const QString message = m_input->text().trimmed();
    if (m_service->send(message))
    {
        onMessage(tr("You"), message);
        m_input->clear();
    }
    onControlsChanged();
}
void IDOSAssistantWidget::onAssistantReady()
{
    m_statusLabel->setText(tr("Ready"));
    emit statusChanged(m_statusLabel->text());
    onControlsChanged();
}

void IDOSAssistantWidget::onAssistantStopped()
{
    m_statusLabel->setText(tr("Stopped"));
    emit statusChanged(m_statusLabel->text());
    onControlsChanged();
}

void IDOSAssistantWidget::onAssistantError(const QString& error)
{
    m_statusLabel->setText(tr("Error"));
    onMessage(tr("System"), error);
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
    m_transcript->append(QStringLiteral("<b>%1:</b> %2")
                             .arg(role.toHtmlEscaped(), message.toHtmlEscaped()));
}

void IDOSAssistantWidget::onControlsChanged()
{
    const bool running = m_service->isRunning();
    const bool ready = m_service->isReady();
    m_startButton->setEnabled(!running);
    m_stopButton->setEnabled(running);
    m_sendButton->setEnabled(ready && !m_service->isBusy());
}
