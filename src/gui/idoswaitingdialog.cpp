#include <QCloseEvent>
#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPalette>
#include <QProgressBar>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

#include "idoswaitingdialog.h"

IDOSWaitingDialog::IDOSWaitingDialog(QWidget* parent)
    :
    QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint)
    , m_card(new QFrame(this))
    , m_title(new QLabel(m_card))
    , m_message(new QLabel(m_card))
    , m_detail(new QLabel(m_card))
    , m_elapsed(new QLabel(m_card))
    , m_progress(new QProgressBar(m_card))
    , m_timer(new QTimer(this))
    , m_running(false)
{
    QFont interfaceFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
#ifdef Q_OS_WIN
    interfaceFont.setFamily(QStringLiteral("Microsoft YaHei UI"));
#endif
    interfaceFont.setPointSizeF(10.0);
    setFont(interfaceFont);
    setObjectName(QStringLiteral("waitingDialog"));
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumWidth(460);
    setMaximumWidth(560);
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(16, 12, 16, 20);
    outer->addWidget(m_card);
    m_card->setObjectName(QStringLiteral("waitingCard"));
    const QPalette colors = palette();
    const QColor accent = colors.color(QPalette::Highlight);
    const QColor foreground = colors.color(QPalette::WindowText);
    const QColor surface = colors.color(QPalette::Window);
    const QColor muted = colors.color(QPalette::Disabled, QPalette::WindowText);
    m_card->setStyleSheet(QStringLiteral(
        "QFrame#waitingCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
        "QLabel { background: transparent; border: none; color: %3; }"
        "QLabel#waitingDetail, QLabel#waitingElapsed, QLabel#waitingHint { color: %4; }"
        "QProgressBar { border: none; border-radius: 3px; background: %2; min-height: 6px; max-height: 6px; }"
        "QProgressBar::chunk { background: %5; border-radius: 3px; }")
        .arg(surface.name(), colors.color(QPalette::Midlight).name(), foreground.name(), muted.name(), accent.name()));
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(m_card);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 42));
    m_card->setGraphicsEffect(shadow);
    QVBoxLayout* layout = new QVBoxLayout(m_card);
    layout->setContentsMargins(26, 24, 26, 22);
    layout->setSpacing(14);
    QHBoxLayout* heading = new QHBoxLayout();
    heading->setSpacing(12);
    QLabel* icon = new QLabel(m_card);
    icon->setPixmap(QIcon(QStringLiteral(":/images/providers-load.svg")).pixmap(32, 32));
    icon->setFixedSize(36, 36);
    heading->addWidget(icon);
    m_title->setObjectName(QStringLiteral("waitingTitle"));
    m_title->setTextFormat(Qt::PlainText);
    m_title->setWordWrap(true);
    QFont titleFont = m_title->font();
    titleFont.setPointSizeF(titleFont.pointSizeF() + 3.0);
    titleFont.setBold(true);
    m_title->setFont(titleFont);
    heading->addWidget(m_title, 1);
    layout->addLayout(heading);
    m_detail->setObjectName(QStringLiteral("waitingDetail"));
    m_detail->setTextFormat(Qt::PlainText);
    m_detail->setMinimumWidth(0);
    m_detail->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    layout->addWidget(m_detail);
    m_message->setObjectName(QStringLiteral("waitingMessage"));
    m_message->setTextFormat(Qt::PlainText);
    m_message->setWordWrap(true);
    m_message->setAccessibleName(tr("Current operation"));
    layout->addWidget(m_message);
    m_progress->setObjectName(QStringLiteral("waitingProgress"));
    m_progress->setTextVisible(false);
    m_progress->setAccessibleName(tr("Operation progress"));
    layout->addWidget(m_progress);
    QHBoxLayout* footer = new QHBoxLayout();
    QLabel* hint = new QLabel(tr("Please wait…"), m_card);
    hint->setObjectName(QStringLiteral("waitingHint"));
    footer->addWidget(hint);
    footer->addStretch();
    m_elapsed->setObjectName(QStringLiteral("waitingElapsed"));
    footer->addWidget(m_elapsed);
    layout->addLayout(footer);
    m_timer->setInterval(250);
    connect(m_timer, &QTimer::timeout, this, &IDOSWaitingDialog::onElapsedTimeout);
    setBusy();
}

void IDOSWaitingDialog::begin(const QString& title, const QString& message, const QString& detail)
{
    setWindowTitle(title);
    m_title->setText(title);
    setMessage(message);
    setDetail(detail);
    setBusy();
    m_running = true;
    m_clock.start();
    onElapsedTimeout();
    m_timer->start();
    adjustSize();
    if (parentWidget() != nullptr)
    {
        move(parentWidget()->mapToGlobal(parentWidget()->rect().center()) - rect().center());
    }
    show();
    raise();
    repaint();
}

void IDOSWaitingDialog::setMessage(const QString& message)
{
    m_message->setText(message);
    m_progress->setAccessibleDescription(message);
}

void IDOSWaitingDialog::setDetail(const QString& detail)
{
    m_detailText = detail;
    m_detail->setToolTip(detail);
    m_detail->setVisible(!detail.isEmpty());
    updateDetail();
}

void IDOSWaitingDialog::setProgress(int value, int maximum)
{
    if (maximum <= 0)
    {
        setBusy();
        return;
    }
    m_progress->setRange(0, maximum);
    m_progress->setValue(qBound(0, value, maximum));
}

void IDOSWaitingDialog::setBusy()
{
    m_progress->setRange(0, 0);
}

void IDOSWaitingDialog::finish()
{
    m_running = false;
    m_timer->stop();
    QDialog::done(QDialog::Accepted);
}

bool IDOSWaitingDialog::isRunning() const
{
    return m_running;
}

void IDOSWaitingDialog::done(int result)
{
    if (!m_running)
    {
        QDialog::done(result);
    }
}

void IDOSWaitingDialog::closeEvent(QCloseEvent* event)
{
    if (m_running)
    {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}

void IDOSWaitingDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    updateDetail();
}

void IDOSWaitingDialog::updateDetail()
{
    m_detail->setText(m_detail->fontMetrics().elidedText(m_detailText, Qt::ElideMiddle, m_detail->width()));
}

void IDOSWaitingDialog::onElapsedTimeout()
{
    const qint64 seconds = m_clock.isValid() ? m_clock.elapsed() / 1000 : 0;
    const QString duration = seconds < 60 ? tr("%1 s").arg(seconds)
        : tr("%1 min %2 s").arg(seconds / 60).arg(seconds % 60);
    m_elapsed->setText(tr("Elapsed %1").arg(duration));
    updateDetail();
}
