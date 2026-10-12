#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "idossimulationcaseobject.h"

#include "idossimulationrunstatuswidget.h"

IDOSSimulationRunStatusWidget::IDOSSimulationRunStatusWidget(IDOSSimulationCaseObject* simulationCase,
                                                             QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_statusLabel(nullptr)
    , m_directoryLabel(nullptr)
    , m_initializedAtLabel(nullptr)
    , m_filesTable(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationRunStatusWindow"));
    setWindowTitle(tr("Run Status — %1").arg(simulationCase != nullptr ? simulationCase->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    m_directoryLabel = new QLabel(this);
    m_directoryLabel->setWordWrap(true);
    m_directoryLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(m_directoryLabel);

    m_initializedAtLabel = new QLabel(this);
    m_initializedAtLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(m_initializedAtLabel);

    m_filesTable = new QTableWidget(this);
    m_filesTable->setColumnCount(2);
    m_filesTable->setHorizontalHeaderLabels(QStringList() << tr("Input file") << tr("Status"));
    m_filesTable->setEditTriggers(QTableWidget::NoEditTriggers);
    m_filesTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_filesTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_filesTable, 1);

    loadStatus(simulationCase);
    resize(760, 480);
}

void IDOSSimulationRunStatusWidget::loadStatus(IDOSSimulationCaseObject* simulationCase)
{
    if (simulationCase == nullptr)
    {
        m_statusLabel->setText(tr("No simulation case is selected."));
        m_directoryLabel->clear();
        m_initializedAtLabel->clear();
        return;
    }

    const QVariantMap runInfo = simulationCase->runInfo();
    if (runInfo.isEmpty())
    {
        m_statusLabel->setText(tr("Not initialized. Run Initialize to prepare this case."));
        m_directoryLabel->clear();
        m_initializedAtLabel->clear();
        m_filesTable->setRowCount(0);
        return;
    }

    const QString status = runInfo.value(QStringLiteral("status")).toString();
    const QString runDirectory = runInfo.value(QStringLiteral("runDirectory")).toString();
    const QString dataFile = runInfo.value(QStringLiteral("dataFile")).toString();
    const QStringList files = runInfo.value(QStringLiteral("files")).toStringList();
    const QString initializedAt = runInfo.value(QStringLiteral("initializedAt")).toString();
    if (status == QStringLiteral("initialized"))
    {
        m_statusLabel->setText(tr("Initialized. The simulator has not been started."));
    }
    else
    {
        m_statusLabel->setText(tr("Run status: %1").arg(status));
    }
    m_directoryLabel->setText(tr("Run directory: %1").arg(runDirectory));
    m_initializedAtLabel->setText(tr("Initialized at (UTC): %1").arg(initializedAt));

    QStringList filesToShow = files;
    if (!filesToShow.contains(dataFile, Qt::CaseInsensitive))
    {
        filesToShow.prepend(dataFile);
    }
    m_filesTable->setRowCount(filesToShow.size());
    int availableCount = 0;
    for (int row = 0; row < filesToShow.size(); ++row)
    {
        const QString relativePath = filesToShow.at(row);
        const QFileInfo fileInfo(QDir(runDirectory).absoluteFilePath(relativePath));
        const bool exists = fileInfo.exists() && fileInfo.isFile();
        if (exists)
        {
            ++availableCount;
        }
        m_filesTable->setItem(row, 0, new QTableWidgetItem(relativePath));
        m_filesTable->setItem(row, 1, new QTableWidgetItem(exists ? tr("Available") : tr("Missing")));
    }
    m_filesTable->resizeColumnsToContents();
    if (availableCount != filesToShow.size())
    {
        m_statusLabel->setText(tr("Initialized input files are missing. The simulator has not been started."));
    }
}
