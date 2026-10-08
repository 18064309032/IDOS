#include <QDateTime>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QVBoxLayout>

#include "log/idoslogger.h"

#include "idosruntimeinfowidget.h"

IDOSRuntimeInfoWidget::IDOSRuntimeInfoWidget(QWidget* parent)
    : QWidget(parent)
    , m_textEdit(new QPlainTextEdit(this))
{
    QPushButton* clearButton = new QPushButton(tr("Clear"), this);

    QHBoxLayout* toolLayout = new QHBoxLayout();
    toolLayout->setContentsMargins(0, 0, 0, 0);
    toolLayout->addWidget(clearButton);
    toolLayout->addStretch();

    m_textEdit->setReadOnly(true);
    m_textEdit->setMaximumBlockCount(5000);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addLayout(toolLayout);
    layout->addWidget(m_textEdit, 1);

    connect(clearButton, &QPushButton::clicked, this, &IDOSRuntimeInfoWidget::onClearClicked);

    IDOSLogger& logger = IDOSLogger::instance();
    connect(&logger,
            &IDOSLogger::messagePosted,
            this,
            &IDOSRuntimeInfoWidget::onMessagePosted,
            Qt::QueuedConnection);
    connect(&logger,
            &IDOSLogger::recordsCleared,
            this,
            &IDOSRuntimeInfoWidget::onRecordsCleared,
            Qt::QueuedConnection);
}

IDOSRuntimeInfoWidget::~IDOSRuntimeInfoWidget()
{
}

void IDOSRuntimeInfoWidget::onClearClicked()
{
    IDOSLogger::instance().clear();
}

void IDOSRuntimeInfoWidget::onMessagePosted(const QString& message, IDOSLogLevel level)
{
    appendLine(level, message);

    QScrollBar* scrollBar = m_textEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void IDOSRuntimeInfoWidget::onRecordsCleared()
{
    m_textEdit->clear();
}

void IDOSRuntimeInfoWidget::appendLine(IDOSLogLevel level, const QString& text)
{
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);

    if (!m_textEdit->document()->isEmpty())
    {
        cursor.insertBlock();
    }

    QTextCharFormat format;
    format.setForeground(levelColor(level));

    const QString line =
        QStringLiteral("[%1] [%2] %3")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")),
                 idosLogLevelToString(level),
                 text);
    cursor.insertText(line, format);
}

QColor IDOSRuntimeInfoWidget::levelColor(IDOSLogLevel level) const
{
    if (level == IDOSLogLevel::Warn)
    {
        return QColor(Qt::darkYellow);
    }
    if (level == IDOSLogLevel::Error || level == IDOSLogLevel::Fatal)
    {
        return QColor(Qt::red);
    }
    return m_textEdit->palette().color(QPalette::Text);
}
