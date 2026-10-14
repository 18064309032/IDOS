#include <cmath>

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "data/case/idossimulationcaseobject.h"

#include "idossimulationpropertycurvewidget.h"

IDOSSimulationPropertyCurveWidget::IDOSSimulationPropertyCurveWidget(IDOSSimulationCaseObject* simulationCase,
                                                                     QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_tables(simulationCase != nullptr ? simulationCase->curveTables() : QVariantList())
    , m_tableSelector(nullptr)
    , m_seriesList(nullptr)
    , m_plotLabel(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationPropertyCurvesWindow"));
    setWindowTitle(tr("Property Curves — %1").arg(simulationCase != nullptr ? simulationCase->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_tableSelector = new QComboBox(this);
    layout->addWidget(m_tableSelector);

    m_seriesList = new QListWidget(this);
    m_seriesList->setMaximumHeight(100);
    layout->addWidget(m_seriesList);

    m_plotLabel = new QLabel(this);
    m_plotLabel->setMinimumSize(640, 400);
    m_plotLabel->setAlignment(Qt::AlignCenter);
    m_plotLabel->setScaledContents(true);
    layout->addWidget(m_plotLabel, 1);

    for (int tableIndex = 0; tableIndex < m_tables.size(); ++tableIndex)
    {
        const QVariantMap table = m_tables.at(tableIndex).toMap();
        const QString keyword = table.value(QStringLiteral("keyword")).toString();
        const int index = table.value(QStringLiteral("tableIndex")).toInt();
        m_tableSelector->addItem(tr("%1 — Table %2").arg(keyword).arg(index), tableIndex);
    }

    connect(m_tableSelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &IDOSSimulationPropertyCurveWidget::onTableSelectionChanged);
    connect(m_seriesList,
            &QListWidget::itemChanged,
            this,
            &IDOSSimulationPropertyCurveWidget::onSeriesItemChanged);

    updateSeries();
    resize(900, 650);
}

void IDOSSimulationPropertyCurveWidget::onTableSelectionChanged(int index)
{
    Q_UNUSED(index);
    updateSeries();
}

void IDOSSimulationPropertyCurveWidget::onSeriesItemChanged(QListWidgetItem* item)
{
    Q_UNUSED(item);
    renderPlot();
}

void IDOSSimulationPropertyCurveWidget::updateSeries()
{
    m_seriesList->clear();
    const int tableIndex = m_tableSelector->currentData().toInt();
    if (tableIndex < 0 || tableIndex >= m_tables.size())
    {
        renderPlot();
        return;
    }

    const QVariantMap table = m_tables.at(tableIndex).toMap();
    const QStringList columns = table.value(QStringLiteral("columns")).toStringList();
    for (int columnIndex = 1; columnIndex < columns.size() - 1; ++columnIndex)
    {
        QListWidgetItem* item = new QListWidgetItem(columns.at(columnIndex), m_seriesList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }
    renderPlot();
}

void IDOSSimulationPropertyCurveWidget::renderPlot()
{
    const int imageWidth = 1000;
    const int imageHeight = 620;
    QPixmap image(imageWidth, imageHeight);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int tableIndex = m_tableSelector->currentData().toInt();
    if (tableIndex < 0 || tableIndex >= m_tables.size())
    {
        painter.drawText(image.rect(), Qt::AlignCenter, tr("No relative permeability tables are available."));
        m_plotLabel->setPixmap(image);
        return;
    }

    const QVariantMap table = m_tables.at(tableIndex).toMap();
    const QStringList columns = table.value(QStringLiteral("columns")).toStringList();
    const QVariantList rows = table.value(QStringLiteral("rows")).toList();
    QList<int> selectedColumns;
    for (int itemIndex = 0; itemIndex < m_seriesList->count(); ++itemIndex)
    {
        QListWidgetItem* item = m_seriesList->item(itemIndex);
        if (item->checkState() == Qt::Checked)
        {
            selectedColumns.append(itemIndex + 1);
        }
    }

    double xMinimum = 0.0;
    double xMaximum = 0.0;
    double yMinimum = 0.0;
    double yMaximum = 0.0;
    bool hasPoint = false;
    for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
    {
        const QVariantList row = rows.at(rowIndex).toList();
        if (row.isEmpty() || !row.first().isValid())
        {
            continue;
        }
        const double xValue = row.first().toDouble();
        if (!std::isfinite(xValue))
        {
            continue;
        }
        for (int selectedIndex = 0; selectedIndex < selectedColumns.size(); ++selectedIndex)
        {
            const int columnIndex = selectedColumns.at(selectedIndex);
            if (columnIndex >= row.size() || !row.at(columnIndex).isValid())
            {
                continue;
            }
            const double yValue = row.at(columnIndex).toDouble();
            if (!std::isfinite(yValue))
            {
                continue;
            }
            if (!hasPoint)
            {
                xMinimum = xMaximum = xValue;
                yMinimum = yMaximum = yValue;
                hasPoint = true;
            }
            else
            {
                xMinimum = qMin(xMinimum, xValue);
                xMaximum = qMax(xMaximum, xValue);
                yMinimum = qMin(yMinimum, yValue);
                yMaximum = qMax(yMaximum, yValue);
            }
        }
    }

    if (!hasPoint)
    {
        painter.drawText(image.rect(), Qt::AlignCenter, tr("No valid curve points are available."));
        m_plotLabel->setPixmap(image);
        return;
    }

    if (qFuzzyCompare(xMinimum + 1.0, xMaximum + 1.0))
    {
        xMinimum -= 0.5;
        xMaximum += 0.5;
    }
    if (qFuzzyCompare(yMinimum + 1.0, yMaximum + 1.0))
    {
        yMinimum -= 0.5;
        yMaximum += 0.5;
    }
    const double yPadding = (yMaximum - yMinimum) * 0.08;
    yMinimum -= yPadding;
    yMaximum += yPadding;

    const QRectF plotRect(100.0, 55.0, 820.0, 470.0);
    painter.setPen(QPen(QColor(220, 220, 220), 1.0));
    for (int tickIndex = 0; tickIndex <= 5; ++tickIndex)
    {
        const double fraction = static_cast<double>(tickIndex) / 5.0;
        const qreal x = plotRect.left() + fraction * plotRect.width();
        const qreal y = plotRect.bottom() - fraction * plotRect.height();
        painter.drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));
        painter.setPen(Qt::black);
        painter.drawText(QRectF(x - 45.0, plotRect.bottom() + 8.0, 90.0, 24.0), Qt::AlignHCenter | Qt::AlignTop,
                         QString::number(xMinimum + fraction * (xMaximum - xMinimum), 'g', 4));
        painter.drawText(QRectF(8.0, y - 12.0, 82.0, 24.0), Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(yMinimum + fraction * (yMaximum - yMinimum), 'g', 4));
        painter.setPen(QPen(QColor(220, 220, 220), 1.0));
    }

    painter.setPen(QPen(Qt::black, 1.5));
    painter.drawRect(plotRect);
    painter.drawText(QRectF(plotRect.left(), 12.0, plotRect.width(), 30.0), Qt::AlignCenter,
                     table.value(QStringLiteral("keyword")).toString());
    painter.drawText(QRectF(plotRect.left(), plotRect.bottom() + 35.0, plotRect.width(), 30.0), Qt::AlignCenter,
                     columns.isEmpty() ? QString() : columns.first());
    painter.save();
    painter.translate(25.0, plotRect.center().y());
    painter.rotate(-90.0);
    painter.drawText(QRectF(-plotRect.height() / 2.0, -15.0, plotRect.height(), 30.0), Qt::AlignCenter,
                     tr("Relative permeability"));
    painter.restore();

    const QList<QColor> colors = {QColor(40, 110, 210), QColor(225, 115, 35)};
    for (int selectedIndex = 0; selectedIndex < selectedColumns.size(); ++selectedIndex)
    {
        const int columnIndex = selectedColumns.at(selectedIndex);
        const QColor color = colors.at(selectedIndex % colors.size());
        QPainterPath path;
        bool pathStarted = false;
        for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
        {
            const QVariantList row = rows.at(rowIndex).toList();
            if (columnIndex >= row.size() || row.isEmpty() || !row.first().isValid()
                || !row.at(columnIndex).isValid())
            {
                pathStarted = false;
                continue;
            }
            const double xValue = row.first().toDouble();
            const double yValue = row.at(columnIndex).toDouble();
            if (!std::isfinite(xValue) || !std::isfinite(yValue))
            {
                pathStarted = false;
                continue;
            }
            const QPointF point(plotRect.left() + (xValue - xMinimum) / (xMaximum - xMinimum) * plotRect.width(),
                                plotRect.bottom() - (yValue - yMinimum) / (yMaximum - yMinimum) * plotRect.height());
            if (!pathStarted)
            {
                path.moveTo(point);
                pathStarted = true;
            }
            else
            {
                path.lineTo(point);
            }
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(point, 3.0, 3.0);
        }
        painter.setPen(QPen(color, 2.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
        const int legendY = 555 + selectedIndex * 25;
        painter.setPen(QPen(color, 3.0));
        painter.drawLine(360, legendY, 400, legendY);
        painter.setPen(Qt::black);
        painter.drawText(410, legendY + 5, columns.value(columnIndex));
    }

    m_plotLabel->setPixmap(image);
}
