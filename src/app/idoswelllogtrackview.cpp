#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QResizeEvent>
#include <QStyle>
#include <QtGlobal>
#include <QToolButton>
#include <QWheelEvent>

#include "idoswell.h"
#include "idoswelllogset.h"

#include "idoswelllogtrackview.h"

IDOSWellLogTrackView::IDOSWellLogTrackView(QWidget* parent)
    : QWidget(parent)
    , m_well(nullptr)
    , m_resetDepthRangeButton(new QToolButton(this))
    , m_dragStartMinimumDepth(0.0)
    , m_dragStartMaximumDepth(0.0)
    , m_visibleMinimumDepth(0.0)
    , m_visibleMaximumDepth(0.0)
    , m_hasVisibleDepthRange(false)
    , m_isDragging(false)
{
    setMinimumSize(480, 320);
    setAutoFillBackground(true);
    setMouseTracking(true);

    m_resetDepthRangeButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    m_resetDepthRangeButton->setToolTip(tr("Fit all depths"));
    m_resetDepthRangeButton->setAutoRaise(true);
    connect(m_resetDepthRangeButton, &QToolButton::clicked,
            this, &IDOSWellLogTrackView::onResetDepthRange);
    updateResetButtonGeometry();
}

void IDOSWellLogTrackView::setWell(IDOSWell* well)
{
    if (m_well == well)
    {
        return;
    }

    if (m_well != nullptr)
    {
        disconnect(m_well, nullptr, this, nullptr);
    }

    m_well = well;
    if (m_well != nullptr)
    {
        connect(m_well, &IDOSWell::logsChanged,
                this, &IDOSWellLogTrackView::onLogsChanged);
    }
    resetDepthRange();
}

void IDOSWellLogTrackView::setSelectedChannelName(const QString& channelName)
{
    if (m_selectedChannelName == channelName)
    {
        return;
    }

    m_selectedChannelName = channelName;
    resetDepthRange();
}

void IDOSWellLogTrackView::onLogsChanged(const IDOSWellLogSet& logs)
{
    Q_UNUSED(logs)
    resetDepthRange();
}

void IDOSWellLogTrackView::onResetDepthRange()
{
    resetDepthRange();
}

void IDOSWellLogTrackView::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), palette().base());
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QList<IDOSWellLogChannel> channels = displayedChannels();
    if (channels.isEmpty())
    {
        painter.setPen(palette().text().color());
        painter.drawText(rect(), Qt::AlignCenter, tr("No well log curves are available."));
        return;
    }

    double minimumDepth = 0.0;
    double maximumDepth = 0.0;
    if (!visibleDepthRange(channels, &minimumDepth, &maximumDepth))
    {
        painter.setPen(palette().text().color());
        painter.drawText(rect(), Qt::AlignCenter, tr("The selected curves contain no valid samples."));
        return;
    }

    const int depthAxisWidth = 74;
    const int topMargin = 42;
    const int bottomMargin = 30;
    const int contentHeight = height() - topMargin - bottomMargin;
    const int contentWidth = width() - depthAxisWidth - 12;
    if (contentHeight <= 0 || contentWidth <= 0)
    {
        return;
    }

    const double trackWidth = static_cast<double>(contentWidth) / channels.size();
    const double top = static_cast<double>(topMargin);
    const double trackHeight = static_cast<double>(contentHeight);
    const QColor textColor = palette().text().color();
    QPen borderPen(palette().mid().color());
    borderPen.setWidth(1);
    painter.setPen(borderPen);
    painter.drawLine(depthAxisWidth, topMargin, depthAxisWidth, topMargin + contentHeight);
    painter.drawLine(depthAxisWidth, topMargin + contentHeight,
                     width() - 12, topMargin + contentHeight);

    const int tickCount = 5;
    painter.setPen(textColor);
    painter.drawText(4, topMargin + 5, tr("Depth"));
    for (int tickIndex = 0; tickIndex <= tickCount; ++tickIndex)
    {
        const double fraction = static_cast<double>(tickIndex) / tickCount;
        const double depth = minimumDepth + fraction * (maximumDepth - minimumDepth);
        const double y = top + fraction * trackHeight;
        painter.setPen(borderPen);
        painter.drawLine(depthAxisWidth - 5, y, width() - 12, y);
        painter.setPen(textColor);
        painter.drawText(QRectF(2.0, y - 8.0, depthAxisWidth - 8.0, 16.0),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(depth, 'f', 2));
    }

    for (int channelIndex = 0; channelIndex < channels.size(); ++channelIndex)
    {
        const IDOSWellLogChannel& channel = channels.at(channelIndex);
        const double left = depthAxisWidth + channelIndex * trackWidth;
        double minimumValue = 0.0;
        double maximumValue = 0.0;
        if (!valueRange(channel, &minimumValue, &maximumValue))
        {
            continue;
        }

        painter.setPen(borderPen);
        painter.drawRect(QRectF(left, top, trackWidth, trackHeight));

        painter.setPen(textColor);
        const QString title = channel.unit().isEmpty()
            ? channel.name()
            : QStringLiteral("%1 [%2]").arg(channel.name(), channel.unit());
        painter.drawText(QRectF(left + 3.0, 3.0, trackWidth - 6.0, 18.0),
                         Qt::AlignHCenter | Qt::AlignVCenter, title);
        painter.drawText(QRectF(left + 3.0, topMargin + contentHeight + 4.0,
                                trackWidth - 6.0, 18.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QString::number(minimumValue, 'g', 5));
        painter.drawText(QRectF(left + 3.0, topMargin + contentHeight + 4.0,
                                trackWidth - 6.0, 18.0),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(maximumValue, 'g', 5));

        const QVector<double> depths = channel.depths();
        const QVector<double> values = channel.values();
        const int sampleCount = qMin(depths.size(), values.size());
        QPen curvePen(colorForChannel(channel.name()));
        curvePen.setWidth(1);
        painter.setPen(curvePen);

        bool hasPreviousPoint = false;
        QPointF previousPoint;
        for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            const double depth = depths.at(sampleIndex);
            const double value = values.at(sampleIndex);
            if (qIsNaN(depth) || qIsNaN(value) || depth < minimumDepth || depth > maximumDepth)
            {
                hasPreviousPoint = false;
                continue;
            }

            const QPointF point(valueToX(value, minimumValue, maximumValue, left, trackWidth),
                                depthToY(depth, minimumDepth, maximumDepth, top, trackHeight));
            if (hasPreviousPoint)
            {
                painter.drawLine(previousPoint, point);
            }
            previousPoint = point;
            hasPreviousPoint = true;
        }
    }
}

void IDOSWellLogTrackView::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_isDragging)
    {
        return;
    }

    const QList<IDOSWellLogChannel> channels = displayedChannels();
    double dataMinimumDepth = 0.0;
    double dataMaximumDepth = 0.0;
    if (!depthRange(channels, &dataMinimumDepth, &dataMaximumDepth))
    {
        return;
    }

    const int topMargin = 42;
    const int bottomMargin = 30;
    const int contentHeight = height() - topMargin - bottomMargin;
    if (contentHeight <= 0)
    {
        return;
    }

    const double dragDepth = static_cast<double>(event->pos().y() - m_dragStartPosition.y())
        / contentHeight * (m_dragStartMaximumDepth - m_dragStartMinimumDepth);
    m_visibleMinimumDepth = m_dragStartMinimumDepth - dragDepth;
    m_visibleMaximumDepth = m_dragStartMaximumDepth - dragDepth;
    clampVisibleDepthRange(dataMinimumDepth, dataMaximumDepth);
    update();
}

void IDOSWellLogTrackView::mousePressEvent(QMouseEvent* event)
{
    const int depthAxisWidth = 74;
    const int topMargin = 42;
    const int bottomMargin = 30;
    if (event->button() != Qt::LeftButton || event->pos().x() < depthAxisWidth
        || event->pos().y() < topMargin || event->pos().y() > height() - bottomMargin)
    {
        return;
    }

    const QList<IDOSWellLogChannel> channels = displayedChannels();
    double minimumDepth = 0.0;
    double maximumDepth = 0.0;
    if (!visibleDepthRange(channels, &minimumDepth, &maximumDepth))
    {
        return;
    }

    m_dragStartPosition = event->pos();
    m_dragStartMinimumDepth = minimumDepth;
    m_dragStartMaximumDepth = maximumDepth;
    m_isDragging = true;
    setCursor(Qt::ClosedHandCursor);
}

void IDOSWellLogTrackView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || !m_isDragging)
    {
        return;
    }

    m_isDragging = false;
    unsetCursor();
}

void IDOSWellLogTrackView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateResetButtonGeometry();
}

void IDOSWellLogTrackView::wheelEvent(QWheelEvent* event)
{
    const int depthAxisWidth = 74;
    const int topMargin = 42;
    const int bottomMargin = 30;
    const int contentHeight = height() - topMargin - bottomMargin;
    if (event->pos().x() < depthAxisWidth || event->pos().y() < topMargin
        || event->pos().y() > height() - bottomMargin || contentHeight <= 0)
    {
        event->ignore();
        return;
    }

    const int delta = event->angleDelta().y();
    if (delta == 0)
    {
        event->ignore();
        return;
    }

    const QList<IDOSWellLogChannel> channels = displayedChannels();
    double dataMinimumDepth = 0.0;
    double dataMaximumDepth = 0.0;
    double minimumDepth = 0.0;
    double maximumDepth = 0.0;
    if (!depthRange(channels, &dataMinimumDepth, &dataMaximumDepth)
        || !visibleDepthRange(channels, &minimumDepth, &maximumDepth))
    {
        event->ignore();
        return;
    }

    const double anchorDepth = yToDepth(event->pos().y(), minimumDepth, maximumDepth,
                                        topMargin, contentHeight);
    const double zoomFactor = delta > 0 ? 0.8 : 1.25;
    const double currentRange = maximumDepth - minimumDepth;
    const double minimumRange = (dataMaximumDepth - dataMinimumDepth) / 10000.0;
    const double newRange = qMax(minimumRange, currentRange * zoomFactor);
    const double anchorFraction = (anchorDepth - minimumDepth) / currentRange;
    m_visibleMinimumDepth = anchorDepth - anchorFraction * newRange;
    m_visibleMaximumDepth = m_visibleMinimumDepth + newRange;
    m_hasVisibleDepthRange = true;
    clampVisibleDepthRange(dataMinimumDepth, dataMaximumDepth);
    update();
    event->accept();
}

QList<IDOSWellLogChannel> IDOSWellLogTrackView::displayedChannels() const
{
    if (m_well == nullptr || !m_well->hasLogs())
    {
        return QList<IDOSWellLogChannel>();
    }

    if (m_selectedChannelName.isEmpty())
    {
        return m_well->logs().channels();
    }

    const IDOSWellLogChannel* channel = m_well->logs().channel(m_selectedChannelName);
    if (channel == nullptr)
    {
        return QList<IDOSWellLogChannel>();
    }

    QList<IDOSWellLogChannel> channels;
    channels.append(*channel);
    return channels;
}

QColor IDOSWellLogTrackView::colorForChannel(const QString& channelName) const
{
    const uint value = qHash(channelName);
    const int hue = static_cast<int>(value % 360U);
    return QColor::fromHsv(hue, 180, 190);
}

bool IDOSWellLogTrackView::depthRange(const QList<IDOSWellLogChannel>& channels,
                                      double* minimumDepth,
                                      double* maximumDepth) const
{
    bool hasDepth = false;
    for (const IDOSWellLogChannel& channel : channels)
    {
        const QVector<double> depths = channel.depths();
        for (double depth : depths)
        {
            if (qIsNaN(depth))
            {
                continue;
            }

            if (!hasDepth)
            {
                *minimumDepth = depth;
                *maximumDepth = depth;
                hasDepth = true;
                continue;
            }

            *minimumDepth = qMin(*minimumDepth, depth);
            *maximumDepth = qMax(*maximumDepth, depth);
        }
    }

    if (hasDepth && qFuzzyCompare(*minimumDepth, *maximumDepth))
    {
        *minimumDepth -= 0.5;
        *maximumDepth += 0.5;
    }
    return hasDepth;
}

bool IDOSWellLogTrackView::valueRange(const IDOSWellLogChannel& channel,
                                      double* minimumValue,
                                      double* maximumValue) const
{
    const QVector<double> values = channel.values();
    bool hasValue = false;
    for (double value : values)
    {
        if (qIsNaN(value))
        {
            continue;
        }

        if (!hasValue)
        {
            *minimumValue = value;
            *maximumValue = value;
            hasValue = true;
            continue;
        }

        *minimumValue = qMin(*minimumValue, value);
        *maximumValue = qMax(*maximumValue, value);
    }

    if (hasValue && qFuzzyCompare(*minimumValue, *maximumValue))
    {
        *minimumValue -= 0.5;
        *maximumValue += 0.5;
    }
    return hasValue;
}

bool IDOSWellLogTrackView::visibleDepthRange(const QList<IDOSWellLogChannel>& channels,
                                              double* minimumDepth,
                                              double* maximumDepth) const
{
    if (m_hasVisibleDepthRange)
    {
        *minimumDepth = m_visibleMinimumDepth;
        *maximumDepth = m_visibleMaximumDepth;
        return true;
    }

    return depthRange(channels, minimumDepth, maximumDepth);
}

void IDOSWellLogTrackView::clampVisibleDepthRange(double dataMinimumDepth,
                                                  double dataMaximumDepth)
{
    const double dataRange = dataMaximumDepth - dataMinimumDepth;
    double visibleRange = m_visibleMaximumDepth - m_visibleMinimumDepth;
    if (visibleRange >= dataRange)
    {
        m_visibleMinimumDepth = dataMinimumDepth;
        m_visibleMaximumDepth = dataMaximumDepth;
        return;
    }

    if (m_visibleMinimumDepth < dataMinimumDepth)
    {
        m_visibleMinimumDepth = dataMinimumDepth;
        m_visibleMaximumDepth = m_visibleMinimumDepth + visibleRange;
    }
    if (m_visibleMaximumDepth > dataMaximumDepth)
    {
        m_visibleMaximumDepth = dataMaximumDepth;
        m_visibleMinimumDepth = m_visibleMaximumDepth - visibleRange;
    }
}

void IDOSWellLogTrackView::resetDepthRange()
{
    m_hasVisibleDepthRange = false;
    m_isDragging = false;
    unsetCursor();
    update();
}

void IDOSWellLogTrackView::updateResetButtonGeometry()
{
    const int buttonSize = 28;
    m_resetDepthRangeButton->setGeometry(width() - buttonSize - 6, 6, buttonSize, buttonSize);
}

double IDOSWellLogTrackView::depthToY(double depth, double minimumDepth, double maximumDepth,
                                      double top, double height) const
{
    return top + (depth - minimumDepth) / (maximumDepth - minimumDepth) * height;
}

double IDOSWellLogTrackView::valueToX(double value, double minimumValue, double maximumValue,
                                      double left, double width) const
{
    return left + (value - minimumValue) / (maximumValue - minimumValue) * width;
}

double IDOSWellLogTrackView::yToDepth(double y, double minimumDepth, double maximumDepth,
                                      double top, double height) const
{
    return minimumDepth + (y - top) / height * (maximumDepth - minimumDepth);
}
