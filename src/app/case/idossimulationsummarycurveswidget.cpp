#include <cmath>

#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QVBoxLayout>
#include <QVariant>
#include <QtGlobal>

#include "data/case/idossimulationsummaryresultsobject.h"

#include "idossimulationsummarycurveswidget.h"

IDOSSimulationSummaryCurvesWidget::IDOSSimulationSummaryCurvesWidget(
    IDOSSimulationSummaryResultsObject* results,
    QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_timeValues(results != nullptr ? results->timeValues() : QVector<qint64>())
    , m_series(results != nullptr ? results->series() : QList<QVariantMap>())
    , m_seriesSelector(nullptr)
    , m_plotLabel(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationSummaryCurvesWindow"));
    setWindowTitle(tr("Summary Curves — %1").arg(results != nullptr ? results->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_seriesSelector = new QComboBox(this);
    m_plotLabel = new QLabel(this);
    m_plotLabel->setMinimumSize(720, 460);
    m_plotLabel->setAlignment(Qt::AlignCenter);
    m_plotLabel->setScaledContents(true);
    layout->addWidget(m_seriesSelector);
    layout->addWidget(m_plotLabel, 1);

    for (int index = 0; index < m_series.size(); ++index)
    {
        const QVariantMap& item = m_series.at(index);
        const QString keyword = item.value(QStringLiteral("keyword")).toString();
        const QString entity = item.value(QStringLiteral("entity")).toString();
        const QString label = entity.isEmpty() ? keyword : QStringLiteral("%1 — %2").arg(keyword, entity);
        m_seriesSelector->addItem(label, index);
    }
    connect(m_seriesSelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &IDOSSimulationSummaryCurvesWidget::onSeriesSelectionChanged);
    resize(900, 620);
    renderPlot();
}

void IDOSSimulationSummaryCurvesWidget::onSeriesSelectionChanged(int index)
{
    Q_UNUSED(index);
    renderPlot();
}

void IDOSSimulationSummaryCurvesWidget::renderPlot()
{
    const int imageWidth = 1100;
    const int imageHeight = 700;
    QPixmap image(imageWidth, imageHeight);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int seriesIndex = m_seriesSelector->currentData().toInt();
    if (seriesIndex < 0 || seriesIndex >= m_series.size() || m_timeValues.isEmpty())
    {
        painter.drawText(image.rect(), Qt::AlignCenter, tr("No summary series are available."));
        m_plotLabel->setPixmap(image);
        return;
    }

    const QVariantMap item = m_series.at(seriesIndex);
    const QVector<double> values = item.value(QStringLiteral("values")).value<QVector<double>>();
    const int pointCount = qMin(m_timeValues.size(), values.size());
    double yMinimum = 0.0;
    double yMaximum = 0.0;
    bool hasValue = false;
    for (int index = 0; index < pointCount; ++index)
    {
        const double value = values.at(index);
        if (!std::isfinite(value))
        {
            continue;
        }
        if (!hasValue)
        {
            yMinimum = value;
            yMaximum = value;
            hasValue = true;
        }
        else
        {
            yMinimum = qMin(yMinimum, value);
            yMaximum = qMax(yMaximum, value);
        }
    }
    if (!hasValue)
    {
        painter.drawText(image.rect(), Qt::AlignCenter, tr("The selected series has no finite values."));
        m_plotLabel->setPixmap(image);
        return;
    }
    if (qFuzzyCompare(yMinimum + 1.0, yMaximum + 1.0))
    {
        yMinimum -= 0.5;
        yMaximum += 0.5;
    }
    const double yPadding = (yMaximum - yMinimum) * 0.08;
    yMinimum -= yPadding;
    yMaximum += yPadding;

    const QRectF plotRect(110.0, 65.0, 900.0, 520.0);
    painter.setPen(QPen(QColor(225, 225, 225), 1.0));
    for (int tick = 0; tick <= 5; ++tick)
    {
        const double fraction = static_cast<double>(tick) / 5.0;
        const qreal x = plotRect.left() + fraction * plotRect.width();
        const qreal y = plotRect.bottom() - fraction * plotRect.height();
        painter.drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));
        painter.setPen(Qt::black);
        painter.drawText(QRectF(5.0, y - 12.0, 95.0, 24.0), Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(yMinimum + fraction * (yMaximum - yMinimum), 'g', 5));
        painter.setPen(QPen(QColor(225, 225, 225), 1.0));
    }
    painter.setPen(QPen(Qt::black, 1.5));
    painter.drawRect(plotRect);
    painter.drawText(QRectF(plotRect.left(), 15.0, plotRect.width(), 32.0), Qt::AlignCenter,
                     m_seriesSelector->currentText());
    painter.save();
    painter.translate(28.0, plotRect.center().y());
    painter.rotate(-90.0);
    painter.drawText(QRectF(-plotRect.height() / 2.0, -15.0, plotRect.height(), 30.0),
                     Qt::AlignCenter, item.value(QStringLiteral("unit")).toString());
    painter.restore();

    QPainterPath path;
    bool started = false;
    const qint64 firstTime = m_timeValues.first();
    const qint64 lastTime = m_timeValues.at(pointCount - 1);
    const qint64 duration = qMax<qint64>(1, lastTime - firstTime);
    for (int index = 0; index < pointCount; ++index)
    {
        const double value = values.at(index);
        if (!std::isfinite(value))
        {
            started = false;
            continue;
        }
        const double timeFraction = static_cast<double>(m_timeValues.at(index) - firstTime) /
                                    static_cast<double>(duration);
        const double valueFraction = (value - yMinimum) / (yMaximum - yMinimum);
        const QPointF point(plotRect.left() + timeFraction * plotRect.width(),
                            plotRect.bottom() - valueFraction * plotRect.height());
        if (!started)
        {
            path.moveTo(point);
            started = true;
        }
        else
        {
            path.lineTo(point);
        }
    }
    painter.setPen(QPen(QColor(35, 105, 200), 2.0));
    painter.drawPath(path);
    const QString startLabel = QDateTime::fromMSecsSinceEpoch(firstTime).toString(QStringLiteral("yyyy-MM-dd"));
    const QString endLabel = QDateTime::fromMSecsSinceEpoch(lastTime).toString(QStringLiteral("yyyy-MM-dd"));
    painter.setPen(Qt::black);
    painter.drawText(QRectF(plotRect.left(), plotRect.bottom() + 12.0, 150.0, 28.0),
                     Qt::AlignLeft | Qt::AlignTop, startLabel);
    painter.drawText(QRectF(plotRect.right() - 150.0, plotRect.bottom() + 12.0, 150.0, 28.0),
                     Qt::AlignRight | Qt::AlignTop, endLabel);
    m_plotLabel->setPixmap(image);
}
