#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QVBoxLayout>

#include "log/idoslogger.h"

#include "idosdebuginfowidget.h"

IDOSDebugInfoWidget::IDOSDebugInfoWidget(QWidget* parent)
    : QWidget(parent)
    , m_textEdit(new QPlainTextEdit(this))
{
    QPushButton* clearButton = new QPushButton(tr("Clear"), this);
    QPushButton* copyButton = new QPushButton(tr("Copy"), this);

    QHBoxLayout* toolLayout = new QHBoxLayout();
    toolLayout->setContentsMargins(0, 0, 0, 0);
    toolLayout->addWidget(clearButton);
    toolLayout->addWidget(copyButton);
    toolLayout->addStretch();

    m_textEdit->setReadOnly(true);
    m_textEdit->setMaximumBlockCount(5000);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addLayout(toolLayout);
    layout->addWidget(m_textEdit, 1);

    connect(clearButton, &QPushButton::clicked, this, &IDOSDebugInfoWidget::onClearClicked);
    connect(copyButton, &QPushButton::clicked, this, &IDOSDebugInfoWidget::onCopyClicked);

    IDOSLogger& logger = IDOSLogger::instance();
    connect(&logger,
            &IDOSLogger::recordAppended,
            this,
            &IDOSDebugInfoWidget::onRecordAppended);
    connect(&logger,
            &IDOSLogger::recordsCleared,
            this,
            &IDOSDebugInfoWidget::onRecordsCleared);

    rebuild();
}

IDOSDebugInfoWidget::~IDOSDebugInfoWidget()
{
}

void IDOSDebugInfoWidget::onClearClicked()
{
    IDOSLogger::instance().clear();
}

void IDOSDebugInfoWidget::onCopyClicked()
{
    const QString text = m_textEdit->textCursor().hasSelection()
                             ? m_textEdit->textCursor().selectedText()
                             : m_textEdit->toPlainText();
    QApplication::clipboard()->setText(text);
}

void IDOSDebugInfoWidget::onRecordAppended(const IDOSLogRecord& record)
{
    appendRecord(record);

    QScrollBar* scrollBar = m_textEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void IDOSDebugInfoWidget::onRecordsCleared()
{
    m_textEdit->clear();
}

void IDOSDebugInfoWidget::rebuild()
{
    m_textEdit->clear();
    const QList<IDOSLogRecord> records = IDOSLogger::instance().records();
    int index = 0;
    while (index < records.count())
    {
        appendRecord(records.at(index));
        ++index;
    }
}

void IDOSDebugInfoWidget::appendRecord(const IDOSLogRecord& record)
{
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);

    if (!m_textEdit->document()->isEmpty())
    {
        cursor.insertBlock();
    }

    QString text = record.message();
    if (!record.file().isEmpty())
    {
        text += QStringLiteral(" (%1:%2)").arg(record.file()).arg(record.line());
    }

    QTextCharFormat format;
    format.setForeground(levelColor(record.level()));

    const QString line =
        QStringLiteral("[%1] [%2] %3")
            .arg(record.time().toString(QStringLiteral("HH:mm:ss.zzz")),
                 idosLogLevelToString(record.level()),
                 text);
    cursor.insertText(line, format);
}

QColor IDOSDebugInfoWidget::levelColor(IDOSLogLevel level) const
{
    if (level == IDOSLogLevel::Trace || level == IDOSLogLevel::Debug)
    {
        return QColor(Qt::gray);
    }
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
