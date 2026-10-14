#include <algorithm>

#include <QAbstractItemView>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "data/case/idossimulationcaseobject.h"

#include "idossimulationwellcontrolswidget.h"

IDOSSimulationWellControlsWidget::IDOSSimulationWellControlsWidget(IDOSSimulationCaseObject* simulationCase,
                                                                   QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_rows(simulationCase != nullptr ? simulationCase->wellControlRows() : QVariantList())
    , m_wellFilter(nullptr)
    , m_stepFilter(nullptr)
    , m_table(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationWellControlsWindow"));
    setWindowTitle(tr("Well Controls — %1").arg(simulationCase != nullptr ? simulationCase->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* filterLayout = new QHBoxLayout();
    m_wellFilter = new QComboBox(this);
    m_stepFilter = new QComboBox(this);
    filterLayout->addWidget(m_wellFilter);
    filterLayout->addWidget(m_stepFilter);
    layout->addLayout(filterLayout);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels(QStringList()
                                       << tr("Step")
                                       << tr("Keyword")
                                       << tr("Well")
                                       << tr("Status")
                                       << tr("Control Mode")
                                       << tr("Control Values"));
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table);
    m_table->horizontalHeader()->setStretchLastSection(true);

    populateFilters();
    connect(m_wellFilter,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &IDOSSimulationWellControlsWidget::onFiltersChanged);
    connect(m_stepFilter,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &IDOSSimulationWellControlsWidget::onFiltersChanged);
    populateTable();
    resize(1100, 650);
}

void IDOSSimulationWellControlsWidget::onFiltersChanged()
{
    populateTable();
}

QString IDOSSimulationWellControlsWidget::stepLabel(const QVariantMap& row) const
{
    const QString stepType = row.value(QStringLiteral("stepType")).toString();
    const int step = row.value(QStringLiteral("step")).toInt();
    if (stepType == QStringLiteral("date"))
    {
        return row.value(QStringLiteral("stepValue")).toString();
    }
    if (stepType == QStringLiteral("days"))
    {
        return tr("Step %1 (+%2 days)").arg(step).arg(row.value(QStringLiteral("stepValue")).toString());
    }
    return tr("Initial");
}

void IDOSSimulationWellControlsWidget::populateFilters()
{
    m_wellFilter->addItem(tr("All Wells"), QString());
    m_stepFilter->addItem(tr("All Steps"), -1);
    QSet<QString> wellNames;
    QSet<int> steps;
    for (int rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex)
    {
        const QVariantMap row = m_rows.at(rowIndex).toMap();
        const QVariantMap fields = row.value(QStringLiteral("fields")).toMap();
        const QString wellName = fields.value(QStringLiteral("WELL")).toString();
        if (!wellName.isEmpty())
        {
            wellNames.insert(wellName);
        }
        steps.insert(row.value(QStringLiteral("step")).toInt());
    }
    QStringList sortedWellNames = wellNames.values();
    sortedWellNames.sort(Qt::CaseInsensitive);
    for (const QString& wellName : sortedWellNames)
    {
        m_wellFilter->addItem(wellName, wellName);
    }
    QList<int> sortedSteps = steps.values();
    std::sort(sortedSteps.begin(), sortedSteps.end());
    for (int step : sortedSteps)
    {
        for (int rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex)
        {
            const QVariantMap row = m_rows.at(rowIndex).toMap();
            if (row.value(QStringLiteral("step")).toInt() == step)
            {
                m_stepFilter->addItem(stepLabel(row), step);
                break;
            }
        }
    }
}

void IDOSSimulationWellControlsWidget::populateTable()
{
    m_table->setRowCount(0);
    const QString selectedWell = m_wellFilter->currentData().toString();
    const int selectedStep = m_stepFilter->currentData().toInt();
    for (int rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex)
    {
        const QVariantMap row = m_rows.at(rowIndex).toMap();
        const QVariantMap fields = row.value(QStringLiteral("fields")).toMap();
        const QString wellName = fields.value(QStringLiteral("WELL")).toString();
        const int step = row.value(QStringLiteral("step")).toInt();
        if ((!selectedWell.isEmpty() && selectedWell != wellName)
            || (selectedStep >= 0 && selectedStep != step))
        {
            continue;
        }

        QStringList controlValues;
        for (QVariantMap::const_iterator iterator = fields.constBegin(); iterator != fields.constEnd(); ++iterator)
        {
            if (iterator.key() == QStringLiteral("WELL") || iterator.key() == QStringLiteral("STATUS")
                || iterator.key() == QStringLiteral("CMODE"))
            {
                continue;
            }
            const QVariantList values = iterator.value().type() == QVariant::List
                ? iterator.value().toList()
                : QVariantList() << iterator.value();
            QStringList formattedValues;
            for (const QVariant& value : values)
            {
                if (value.isValid())
                {
                    formattedValues.append(value.toString());
                }
            }
            if (!formattedValues.isEmpty())
            {
                controlValues.append(QStringLiteral("%1: %2").arg(iterator.key(), formattedValues.join(", ")));
            }
        }

        const int tableRow = m_table->rowCount();
        m_table->insertRow(tableRow);
        m_table->setItem(tableRow, 0, new QTableWidgetItem(stepLabel(row)));
        m_table->setItem(tableRow, 1, new QTableWidgetItem(row.value(QStringLiteral("keyword")).toString()));
        m_table->setItem(tableRow, 2, new QTableWidgetItem(wellName));
        m_table->setItem(tableRow, 3, new QTableWidgetItem(fields.value(QStringLiteral("STATUS")).toString()));
        m_table->setItem(tableRow, 4, new QTableWidgetItem(fields.value(QStringLiteral("CMODE")).toString()));
        m_table->setItem(tableRow, 5, new QTableWidgetItem(controlValues.join(QStringLiteral("; "))));
    }
    m_table->resizeColumnsToContents();
}
