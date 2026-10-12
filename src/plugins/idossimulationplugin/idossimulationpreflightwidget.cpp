#include <QAbstractItemView>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "idoscaseitemref.h"
#include "idossimulationcaseobject.h"
#include "log/idoslogger.h"

#include "idossimulationpreflightwidget.h"

IDOSSimulationPreflightWidget::IDOSSimulationPreflightWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_summaryLabel(nullptr)
    , m_resultsTable(nullptr)
    , m_blockingCount(0)
    , m_warningCount(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationRunPreflightWindow"));
    setWindowTitle(tr("Run Preflight — %1").arg(simulationCase != nullptr ? simulationCase->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(3);
    m_resultsTable->setHorizontalHeaderLabels(QStringList() << tr("Check") << tr("Result") << tr("Details"));
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setAlternatingRowColors(true);
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_resultsTable);

    runChecks(simulationCase);
    resize(900, 560);
}

bool IDOSSimulationPreflightWidget::hasBlockingIssues() const
{
    return m_blockingCount > 0;
}

void IDOSSimulationPreflightWidget::addResult(const QString& checkName, const QString& status, const QString& details)
{
    const QString message = checkName + QStringLiteral(": ") + status + QStringLiteral(" - ") + details;
    const IDOSLogLevel level = status == tr("Blocked") ? IDOSLogLevel::Error
                              : (status == tr("Warning") ? IDOSLogLevel::Warn : IDOSLogLevel::Info);
    IDOSLogger::instance().message(message, level);
    const int row = m_resultsTable->rowCount();
    m_resultsTable->insertRow(row);
    m_resultsTable->setItem(row, 0, new QTableWidgetItem(checkName));
    m_resultsTable->setItem(row, 1, new QTableWidgetItem(status));
    m_resultsTable->setItem(row, 2, new QTableWidgetItem(details));
    if (status == tr("Blocked"))
    {
        ++m_blockingCount;
    }
    else if (status == tr("Warning"))
    {
        ++m_warningCount;
    }
    m_resultsTable->resizeColumnsToContents();
}

void IDOSSimulationPreflightWidget::runChecks(IDOSSimulationCaseObject* simulationCase)
{
    if (simulationCase == nullptr)
    {
        addResult(tr("Simulation case"), tr("Blocked"), tr("No simulation case is selected."));
    }
    else
    {
        const QString sourceFile = simulationCase->sourceFile();
        const QFileInfo sourceInfo(sourceFile);
        if (sourceFile.isEmpty() || !sourceInfo.exists() || !sourceInfo.isFile())
        {
            addResult(tr("DATA source"), tr("Blocked"), tr("The source DATA file is missing."));
        }
        else if (!sourceInfo.isReadable())
        {
            addResult(tr("DATA source"), tr("Blocked"), tr("The source DATA file cannot be read."));
        }
        else
        {
            addResult(tr("DATA source"), tr("Passed"), sourceInfo.absoluteFilePath());
        }

        const QVariantMap parsedData = simulationCase->parsedSourceData();
        const QVariantMap summary = parsedData.value(QStringLiteral("summary")).toMap();
        if (summary.isEmpty())
        {
            addResult(tr("Parsed input summary"), tr("Blocked"), tr("No parse summary is available for this case."));
        }
        else
        {
            addResult(tr("Parsed input summary"), tr("Passed"),
                      tr("The DATA file was parsed when this case was imported."));
        }

        const QVariantMap sectionCounts = summary.value(QStringLiteral("sectionCounts")).toMap();
        const QStringList requiredSections = {QStringLiteral("RUNSPEC"), QStringLiteral("GRID"),
                                              QStringLiteral("PROPS"), QStringLiteral("SOLUTION"),
                                              QStringLiteral("SCHEDULE")};
        for (const QString& section : requiredSections)
        {
            const int count = sectionCounts.value(section).toInt();
            if (count > 0)
            {
                addResult(tr("Required section: %1").arg(section), tr("Passed"), tr("Found %1 section(s).").arg(count));
            }
            else
            {
                addResult(tr("Required section: %1").arg(section), tr("Blocked"),
                          tr("The required section is missing."));
            }
        }

        bool hasGridReference = false;
        const QList<IDOSCaseItemRef> references = simulationCase->itemRefs();
        for (const IDOSCaseItemRef& reference : references)
        {
            if (reference.role() == QStringLiteral("case.grid"))
            {
                hasGridReference = true;
                break;
            }
        }
        if (hasGridReference)
        {
            addResult(tr("Imported grid"), tr("Passed"), tr("A grid is attached to this case."));
        }
        else
        {
            addResult(tr("Imported grid"), tr("Warning"), tr("No grid object is attached to this case."));
        }

        const QStringList wellNames = summary.value(QStringLiteral("wellNames")).toStringList();
        if (wellNames.isEmpty())
        {
            addResult(tr("Well definitions"), tr("Warning"), tr("No WELSPECS records were found."));
        }
        else
        {
            addResult(tr("Well definitions"), tr("Passed"), tr("Found %1 well(s).").arg(wellNames.size()));
        }

        const QVariantMap keywordCounts = summary.value(QStringLiteral("keywordCounts")).toMap();
        const int wellControlCount = keywordCounts.value(QStringLiteral("WCONPROD")).toInt() +
                                     keywordCounts.value(QStringLiteral("WCONINJE")).toInt();
        if (wellControlCount == 0)
        {
            addResult(tr("Well controls"), tr("Warning"), tr("No WCONPROD or WCONINJE records were found."));
        }
        else
        {
            addResult(tr("Well controls"), tr("Passed"), tr("Found %1 well control record(s).").arg(wellControlCount));
        }

        const QVariantList keywordIndex = parsedData.value(QStringLiteral("keywordIndex")).toList();
        QStringList missingFiles;
        QStringList externalFiles;
        QStringList indexedFiles;
        for (int index = 0; index < keywordIndex.size(); ++index)
        {
            const QString filePath = keywordIndex.at(index).toMap().value(QStringLiteral("file")).toString();
            if (!filePath.isEmpty() && !indexedFiles.contains(filePath))
            {
                indexedFiles.append(filePath);
                if (!QFileInfo::exists(filePath))
                {
                    missingFiles.append(filePath);
                }
                else
                {
                    const QString relativePath =
                        QDir(sourceInfo.absolutePath()).relativeFilePath(QFileInfo(filePath).absoluteFilePath());
                    if (relativePath == QStringLiteral("..") || relativePath.startsWith(QStringLiteral("../")) ||
                        relativePath.startsWith(QStringLiteral("..\\")))
                    {
                        externalFiles.append(filePath);
                    }
                }
            }
        }
        if (!missingFiles.isEmpty())
        {
            addResult(tr("Referenced input files"), tr("Blocked"), missingFiles.join(QLatin1Char('\n')));
        }
        else if (indexedFiles.isEmpty())
        {
            addResult(tr("Referenced input files"), tr("Warning"), tr("No keyword source files were indexed."));
        }
        else if (!externalFiles.isEmpty())
        {
            addResult(tr("Referenced input files"), tr("Blocked"),
                      tr("Input files outside the DATA directory tree cannot be relocated safely:") +
                          QLatin1Char('\n') + externalFiles.join(QLatin1Char('\n')));
        }
        else
        {
            addResult(tr("Referenced input files"), tr("Passed"),
                      tr("All %1 indexed source file(s) exist.").arg(indexedFiles.size()));
        }
    }

    if (m_blockingCount > 0)
    {
        m_summaryLabel->setText(
            tr("Preflight blocked: %1 blocking issue(s), %2 warning(s).").arg(m_blockingCount).arg(m_warningCount));
    }
    else if (m_warningCount > 0)
    {
        m_summaryLabel->setText(
            tr("Preflight completed with %1 warning(s). No blocking issue was found.").arg(m_warningCount));
    }
    else
    {
        m_summaryLabel->setText(tr("Preflight passed. No issue was found."));
    }
}
