#include <QAbstractItemView>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariantMap>
#include <QVBoxLayout>

#include "idossimulationcaseobject.h"
#include "idossimulationpreflightwidget.h"

#include "idossimulationworkflowwidget.h"

IDOSSimulationWorkflowWidget::IDOSSimulationWorkflowWidget(IDOSSimulationCaseObject* simulationCase,
                                                           QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_stepsTable(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationWorkflowWindow"));
    setWindowTitle(tr("Simulation Workflow — %1")
                       .arg(simulationCase != nullptr ? simulationCase->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* descriptionLabel = new QLabel(
        tr("Review input preparation, Flow execution and result import for this case."),
        this);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);

    m_stepsTable = new QTableWidget(this);
    m_stepsTable->setColumnCount(3);
    m_stepsTable->setHorizontalHeaderLabels(QStringList()
                                            << tr("Step")
                                            << tr("Status")
                                            << tr("Details"));
    m_stepsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_stepsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_stepsTable->setAlternatingRowColors(true);
    m_stepsTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_stepsTable, 1);

    if (simulationCase == nullptr)
    {
        addStep(tr("Simulation case"), tr("Blocked"), tr("No simulation case is selected."));
    }
    else
    {
        const QString sourceFile = simulationCase->sourceFile();
        const QFileInfo sourceInfo(sourceFile);
        const QVariantMap parsedData = simulationCase->parsedSourceData();
        const bool sourceReady = !sourceFile.isEmpty()
            && sourceInfo.exists()
            && sourceInfo.isFile()
            && !parsedData.value(QStringLiteral("summary")).toMap().isEmpty();
        addStep(tr("DATA input"),
                sourceReady ? tr("Ready") : tr("Needs attention"),
                sourceReady ? sourceFile : tr("A readable DATA source and parse summary are required."));

        IDOSSimulationPreflightWidget preflightWidget(simulationCase);
        addStep(tr("Run Preflight"),
                preflightWidget.hasBlockingIssues() ? tr("Blocked") : tr("No blocking issues"),
                preflightWidget.hasBlockingIssues()
                    ? tr("Resolve the blocking checks before initialization.")
                    : tr("Preflight found no blocking checks."));

        const QVariantMap runInfo = simulationCase->runInfo();
        const QString runDirectory = runInfo.value(QStringLiteral("runDirectory")).toString();
        const bool initialized = !runInfo.isEmpty() && QFileInfo(runDirectory).isDir();
        addStep(tr("Initialize"),
                initialized ? tr("Ready") : tr("Not initialized"),
                initialized ? runDirectory : tr("Prepare an isolated run directory after preflight."));

        addStep(tr("Run Simulator"),
                initialized ? tr("Available") : tr("Not initialized"),
                tr("Open Run Status to configure and start Flow through WSL2."));

        const QStringList inputFiles = runInfo.value(QStringLiteral("files")).toStringList();
        int resultFileCount = 0;
        if (initialized)
        {
            QDirIterator iterator(runDirectory, QDir::Files, QDirIterator::Subdirectories);
            while (iterator.hasNext())
            {
                const QString filePath = iterator.next();
                const QString relativePath = QDir::fromNativeSeparators(QDir(runDirectory).relativeFilePath(filePath));
                if (!inputFiles.contains(relativePath, Qt::CaseInsensitive))
                {
                    ++resultFileCount;
                }
            }
        }
        addStep(tr("Review Results"),
                resultFileCount > 0 ? tr("Available") : tr("No results"),
                resultFileCount > 0
                    ? tr("%1 generated file(s) are present.").arg(resultFileCount)
                    : tr("Generated result files will appear after a simulator run."));
    }

    resize(900, 420);
}

void IDOSSimulationWorkflowWidget::addStep(const QString& name,
                                           const QString& status,
                                           const QString& details)
{
    const int row = m_stepsTable->rowCount();
    m_stepsTable->insertRow(row);
    m_stepsTable->setItem(row, 0, new QTableWidgetItem(name));
    m_stepsTable->setItem(row, 1, new QTableWidgetItem(status));
    m_stepsTable->setItem(row, 2, new QTableWidgetItem(details));
    m_stepsTable->resizeColumnsToContents();
}
