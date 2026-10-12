#include <QAbstractItemView>
#include <QHeaderView>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariantMap>
#include <QVBoxLayout>

#include "idossimulationcaseobject.h"

#include "idossimulationcasetablewidget.h"

IDOSSimulationCaseTableWidget::IDOSSimulationCaseTableWidget(IDOSSimulationCaseObject* simulationCase,
                                                             QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_filter(nullptr)
    , m_table(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationCaseTableWindow"));
    setWindowTitle(tr("Case Table — %1").arg(simulationCase->name()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText(tr("Filter table rows"));
    layout->addWidget(m_filter);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels(QStringList()
                                       << tr("Category")
                                       << tr("Item")
                                       << tr("Value"));
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table);

    const QVariantMap summary = simulationCase->parsedSourceData().value(QStringLiteral("summary")).toMap();
    addRow(tr("Case"), tr("Name"), simulationCase->name());
    addRow(tr("Case"), tr("DATA Title"), summary.value(QStringLiteral("caseName")).toString());
    addRow(tr("Case"), tr("DATA File"), simulationCase->sourceFile());

    const QVariantMap sectionCounts = summary.value(QStringLiteral("sectionCounts")).toMap();
    for (QVariantMap::const_iterator iterator = sectionCounts.constBegin();
         iterator != sectionCounts.constEnd();
         ++iterator)
    {
        addRow(tr("Input Sections"), iterator.key(), QString::number(iterator.value().toInt()));
    }

    const QVariantMap keywordCounts = summary.value(QStringLiteral("keywordCounts")).toMap();
    for (QVariantMap::const_iterator iterator = keywordCounts.constBegin();
         iterator != keywordCounts.constEnd();
         ++iterator)
    {
        addRow(tr("Keyword Records"), iterator.key(), QString::number(iterator.value().toInt()));
    }

    const QStringList wellNames = summary.value(QStringLiteral("wellNames")).toStringList();
    for (const QString& wellName : wellNames)
    {
        addRow(tr("Wells"), tr("Well"), wellName);
    }

    m_table->setSortingEnabled(true);
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setStretchLastSection(true);

    connect(m_filter,
            &QLineEdit::textChanged,
            this,
            &IDOSSimulationCaseTableWidget::onFilterTextChanged);

    resize(900, 600);
}

IDOSSimulationCaseTableWidget::~IDOSSimulationCaseTableWidget()
{
}

void IDOSSimulationCaseTableWidget::addRow(const QString& category,
                                           const QString& item,
                                           const QString& value)
{
    const int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem(category));
    m_table->setItem(row, 1, new QTableWidgetItem(item));
    m_table->setItem(row, 2, new QTableWidgetItem(value));
}

void IDOSSimulationCaseTableWidget::onFilterTextChanged(const QString& text)
{
    for (int row = 0; row < m_table->rowCount(); ++row)
    {
        bool matches = false;
        for (int column = 0; column < m_table->columnCount(); ++column)
        {
            QTableWidgetItem* item = m_table->item(row, column);
            if (item != nullptr && item->text().contains(text, Qt::CaseInsensitive))
            {
                matches = true;
                break;
            }
        }
        m_table->setRowHidden(row, !matches);
    }
}
