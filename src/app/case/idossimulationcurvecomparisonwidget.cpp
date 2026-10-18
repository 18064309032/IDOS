#include <cmath>

#include <QColor>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QStringList>
#include <QVBoxLayout>
#include <QVariantList>
#include <QVariantMap>

#include "data/case/idossimulationcaseobject.h"

#include "idossimulationcurvecomparisonwidget.h"

IDOSSimulationCurveComparisonWidget::IDOSSimulationCurveComparisonWidget(
    IDOSSimulationCaseObject* simulationCase,
    QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_tables(simulationCase != nullptr ? simulationCase->curveTables() : QVariantList())
    , m_keywordSelector(nullptr)
    , m_curveSelector(nullptr)
    , m_tableList(nullptr)
    , m_plotLabel(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationCurveComparisonWindow"));
    setWindowTitle(tr("Curve Comparison — %1")
                       .arg(simulationCase != nullptr ? simulationCase->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* selectionLayout = new QHBoxLayout();
    selectionLayout->addWidget(new QLabel(tr("Keyword"), this));
    m_keywordSelector = new QComboBox(this);
    selectionLayout->addWidget(m_keywordSelector, 1);
    selectionLayout->addWidget(new QLabel(tr("Curve"), this));
    m_curveSelector = new QComboBox(this);
    selectionLayout->addWidget(m_curveSelector, 1);
    layout->addLayout(selectionLayout);

    m_tableList = new QListWidget(this);
    m_tableList->setMaximumHeight(110);
    layout->addWidget(m_tableList);

    m_plotLabel = new QLabel(this);
    m_plotLabel->setMinimumSize(640, 400);
    m_plotLabel->setAlignment(Qt::AlignCenter);
    m_plotLabel->setScaledContents(true);
    layout->addWidget(m_plotLabel, 1);

    QStringList keywords;
    for (int tableIndex = 0; tableIndex < m_tables.size(); ++tableIndex)
    {
        const QString keyword = m_tables.at(tableIndex).toMap().value(QStringLiteral("keyword")).toString();
        if (!keyword.isEmpty() && !keywords.contains(keyword, Qt::CaseInsensitive))
        {
            keywords.append(keyword);
        }
    }
    keywords.sort(Qt::CaseInsensitive);
    m_keywordSelector->addItems(keywords);

    connect(m_keywordSelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &IDOSSimulationCurveComparisonWidget::onKeywordSelectionChanged);
    connect(m_curveSelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &IDOSSimulationCurveComparisonWidget::onCurveSelectionChanged);
    connect(m_tableList,
            &QListWidget::itemChanged,
            this,
            &IDOSSimulationCurveComparisonWidget::onTableItemChanged);

    updateTableSelection();
    resize(980, 700);
}

void IDOSSimulationCurveComparisonWidget::onKeywordSelectionChanged(int index)
{
    Q_UNUSED(index);
    updateTableSelection();
}

void IDOSSimulationCurveComparisonWidget::onCurveSelectionChanged(int index)
{
    Q_UNUSED(index);
    renderPlot();
}

void IDOSSimulationCurveComparisonWidget::onTableItemChanged(QListWidgetItem* item)
{
    Q_UNUSED(item);
    renderPlot();
}

void IDOSSimulationCurveComparisonWidget::updateTableSelection()
{
    m_tableList->clear();
    m_curveSelector->clear();
    const QString keyword = m_keywordSelector->currentText();
    QStringList columns;
    int checkedCount = 0;
    for (int tableIndex = 0; tableIndex < m_tables.size(); ++tableIndex)
    {
        const QVariantMap table = m_tables.at(tableIndex).toMap();
        if (table.value(QStringLiteral("keyword")).toString().compare(keyword, Qt::CaseInsensitive) != 0)
        {
            continue;
        }

        const int displayedIndex = table.value(QStringLiteral("tableIndex")).toInt();
        QListWidgetItem* item = new QListWidgetItem(tr("Table %1").arg(displayedIndex), m_tableList);
        item->setData(Qt::UserRole, tableIndex);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(checkedCount < 2 ? Qt::Checked : Qt::Unchecked);
        ++checkedCount;
        if (columns.isEmpty())
        {
            columns = table.value(QStringLiteral("columns")).toStringList();
        }
    }

    for (int columnIndex = 1; columnIndex < columns.size() - 1; ++columnIndex)
    {
        m_curveSelector->addItem(columns.at(columnIndex), columnIndex);
    }
    renderPlot();
}

void IDOSSimulationCurveComparisonWidget::renderPlot()
{
    const int imageWidth = 1000;
    const int imageHeight = 700;
    QPixmap image(imageWidth, imageHeight);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int curveColumn = m_curveSelector->currentData().toInt();
    QList<int> selectedTableIndices;
    for (int itemIndex = 0; itemIndex < m_tableList->count(); ++itemIndex)
    {
        QListWidgetItem* item = m_tableList->item(itemIndex);
        if (item->checkState() == Qt::Checked)
        {
            selectedTableIndices.append(item->data(Qt::UserRole).toInt());
        }
    }
    if (curveColumn <= 0 || selectedTableIndices.isEmpty())
    {
        painter.drawText(image.rect(), Qt::AlignCenter, tr("Select a curve and at least one table to compare."));
        m_plotLabel->setPixmap(image);
        return;
    }

    double xMinimum = 0.0;
    double xMaximum = 0.0;
    double yMinimum = 0.0;
    double yMaximum = 0.0;
    bool hasPoint = false;
    for (int selectedIndex = 0; selectedIndex < selectedTableIndices.size(); ++selectedIndex)
    {
        const QVariantMap table = m_tables.at(selectedTableIndices.at(selectedIndex)).toMap();
        const QVariantList rows = table.value(QStringLiteral("rows")).toList();
        for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
        {
            const QVariantList row = rows.at(rowIndex).toList();
            if (curveColumn >= row.size() || row.isEmpty() || !row.first().isValid()
                || !row.at(curveColumn).isValid())
            {
                continue;
            }
            const double xValue = row.first().toDouble();
            const double yValue = row.at(curveColumn).toDouble();
            if (!std::isfinite(xValue) || !std::isfinite(yValue))
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

    const QRectF plotRect(100.0, 55.0, 820.0, 500.0);
    painter.setPen(QPen(QColor(220, 220, 220), 1.0));
    for (int tickIndex = 0; tickIndex <= 5; ++tickIndex)
    {
        const double fraction = static_cast<double>(tickIndex) / 5.0;
        const qreal x = plotRect.left() + fraction * plotRect.width();
        const qreal y = plotRect.bottom() - fraction * plotRect.height();
        painter.drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));
        painter.setPen(Qt::black);
        painter.drawText(QRectF(x - 45.0, plotRect.bottom() + 8.0, 90.0, 24.0),
                         Qt::AlignHCenter | Qt::AlignTop,
                         QString::number(xMinimum + fraction * (xMaximum - xMinimum), 'g', 4));
        painter.drawText(QRectF(8.0, y - 12.0, 82.0, 24.0),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(yMinimum + fraction * (yMaximum - yMinimum), 'g', 4));
        painter.setPen(QPen(QColor(220, 220, 220), 1.0));
    }

    painter.setPen(QPen(Qt::black, 1.5));
    painter.drawRect(plotRect);
    painter.drawText(QRectF(plotRect.left(), 12.0, plotRect.width(), 30.0),
                     Qt::AlignCenter,
                     QStringLiteral("%1 — %2").arg(m_keywordSelector->currentText(), m_curveSelector->currentText()));
    const QVariantMap firstTable = m_tables.at(selectedTableIndices.first()).toMap();
    const QStringList columns = firstTable.value(QStringLiteral("columns")).toStringList();
    painter.drawText(QRectF(plotRect.left(), plotRect.bottom() + 35.0, plotRect.width(), 30.0),
                     Qt::AlignCenter,
                     columns.isEmpty() ? QString() : columns.first());
    painter.save();
    painter.translate(25.0, plotRect.center().y());
    painter.rotate(-90.0);
    painter.drawText(QRectF(-plotRect.height() / 2.0, -15.0, plotRect.height(), 30.0),
                     Qt::AlignCenter,
                     m_curveSelector->currentText());
    painter.restore();

    const QList<QColor> colors = { QColor(40, 110, 210), QColor(225, 115, 35), QColor(45, 155, 85),
                                   QColor(170, 75, 175), QColor(210, 65, 65), QColor(40, 155, 175) };
    for (int selectedIndex = 0; selectedIndex < selectedTableIndices.size(); ++selectedIndex)
    {
        const QVariantMap table = m_tables.at(selectedTableIndices.at(selectedIndex)).toMap();
        const QVariantList rows = table.value(QStringLiteral("rows")).toList();
        const QColor color = colors.at(selectedIndex % colors.size());
        QPainterPath path;
        bool pathStarted = false;
        for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
        {
            const QVariantList row = rows.at(rowIndex).toList();
            if (curveColumn >= row.size() || row.isEmpty() || !row.first().isValid()
                || !row.at(curveColumn).isValid())
            {
                pathStarted = false;
                continue;
            }
            const double xValue = row.first().toDouble();
            const double yValue = row.at(curveColumn).toDouble();
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
        const int legendX = 120 + (selectedIndex % 3) * 285;
        const int legendY = 620 + (selectedIndex / 3) * 24;
        painter.setPen(QPen(color, 3.0));
        painter.drawLine(legendX, legendY, legendX + 35, legendY);
        painter.setPen(Qt::black);
        painter.drawText(legendX + 43, legendY + 5,
                         tr("Table %1").arg(table.value(QStringLiteral("tableIndex")).toInt()));
    }

    m_plotLabel->setPixmap(image);
}
