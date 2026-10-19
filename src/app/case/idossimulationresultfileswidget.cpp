#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariantMap>

#include "command/idoscommandmanager.h"
#include "data/case/idoscaseitemref.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/grid/idosgridobject.h"
#include "idosproject.h"
#include "logger/idoslogger.h"

#include "idossimulationresultfileswidget.h"

IDOSSimulationResultFilesWidget::IDOSSimulationResultFilesWidget(IDOSSimulationCaseObject* simulationCase,
                                                                 QWidget* parent)
    :
    QWidget(parent)
    , m_runDirectory()
    , m_inputFiles()
    , m_gridId()
    , m_project(nullptr)
    , m_summaryLabel(nullptr)
    , m_filesTable(nullptr)
    , m_importButton(nullptr)
{
    setObjectName(QStringLiteral("simulationResultFilesPage"));
    setStyleSheet(QStringLiteral(
        "QTableWidget { border: 1px solid #d8e0e9; border-radius: 3px; background: #fbfcfe; }"
        "QPushButton { min-height: 28px; padding: 2px 10px; }"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    m_filesTable = new QTableWidget(this);
    m_filesTable->setColumnCount(3);
    m_filesTable->setHorizontalHeaderLabels(QStringList() << tr("File") << tr("Size") << tr("Modified"));
    m_filesTable->setEditTriggers(QTableWidget::NoEditTriggers);
    m_filesTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_filesTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_filesTable, 1);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);
    m_importButton = new QPushButton(tr("Import as Dynamic Properties"), this);
    m_importButton->setEnabled(false);
    buttonLayout->addWidget(m_importButton);
    QPushButton* openDirectoryButton = new QPushButton(tr("Open Run Directory"), this);
    buttonLayout->addWidget(openDirectoryButton);
    layout->addLayout(buttonLayout);
    connect(openDirectoryButton, &QPushButton::clicked, this,
            &IDOSSimulationResultFilesWidget::onOpenRunDirectoryTriggered);
    connect(m_filesTable, &QTableWidget::cellDoubleClicked, this,
            &IDOSSimulationResultFilesWidget::onResultFileDoubleClicked);
    connect(m_filesTable, &QTableWidget::itemSelectionChanged, this,
            &IDOSSimulationResultFilesWidget::onSelectionChanged);
    connect(m_importButton, &QPushButton::clicked, this,
            &IDOSSimulationResultFilesWidget::onImportDynamicPropertiesTriggered);

    loadResultFiles(simulationCase);
}

void IDOSSimulationResultFilesWidget::loadResultFiles(IDOSSimulationCaseObject* simulationCase)
{
    if (simulationCase == nullptr)
    {
        m_summaryLabel->setText(tr("No simulation case is selected."));
        return;
    }

    m_project = qobject_cast<IDOSProject*>(simulationCase->parent());
    if (m_project.isNull())
    {
        m_summaryLabel->setText(tr("The simulation case is not attached to a project."));
        return;
    }

    for (const IDOSCaseItemRef& reference : simulationCase->itemRefs())
    {
        if (reference.role() != QStringLiteral("case.grid"))
        {
            continue;
        }
        IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(m_project->object(reference.objectId()));
        if (grid != nullptr)
        {
            m_gridId = grid->objectId();
            break;
        }
    }

    const QVariantMap runInfo = simulationCase->runInfo();
    m_inputFiles = runInfo.value(QStringLiteral("files")).toStringList();
    const QString latestOutputDirectory = runInfo.value(QStringLiteral("outputDirectory")).toString();
    m_runDirectory = QFileInfo(latestOutputDirectory).isDir()
        ? latestOutputDirectory
        : runInfo.value(QStringLiteral("runDirectory")).toString();
    if (m_runDirectory.isEmpty() || !QFileInfo(m_runDirectory).isDir())
    {
        m_summaryLabel->setText(tr("No run output is available for this case yet."));
        return;
    }

    populateResultFiles(m_runDirectory == latestOutputDirectory ? QStringList() : m_inputFiles);
}

void IDOSSimulationResultFilesWidget::showRunDirectory(const QString& runDirectory)
{
    if (!QFileInfo(runDirectory).isDir())
    {
        return;
    }
    m_runDirectory = QDir::cleanPath(QFileInfo(runDirectory).absoluteFilePath());
    populateResultFiles(QStringList());
}

void IDOSSimulationResultFilesWidget::populateResultFiles(const QStringList& excludedInputFiles)
{
    m_filesTable->setRowCount(0);
    QStringList resultFiles;
    QDirIterator iterator(m_runDirectory, QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        const QString filePath = iterator.next();
        const QString relativePath = QDir::fromNativeSeparators(QDir(m_runDirectory).relativeFilePath(filePath));
        if (!excludedInputFiles.contains(relativePath, Qt::CaseInsensitive))
        {
            resultFiles.append(relativePath);
        }
    }
    resultFiles.sort(Qt::CaseInsensitive);

    if (resultFiles.isEmpty())
    {
        m_summaryLabel->setText(tr("No result files were found in this run directory."));
        return;
    }

    m_summaryLabel->setText(
        tr("Found %1 result file(s). Double-click a row to open the file.").arg(resultFiles.size()));
    m_filesTable->setRowCount(resultFiles.size());
    for (int row = 0; row < resultFiles.size(); ++row)
    {
        const QString relativePath = resultFiles.at(row);
        const QFileInfo fileInfo(QDir(m_runDirectory).absoluteFilePath(relativePath));
        m_filesTable->setItem(row, 0, new QTableWidgetItem(relativePath));
        m_filesTable->setItem(row, 1, new QTableWidgetItem(tr("%1 bytes").arg(fileInfo.size())));
        m_filesTable->setItem(row, 2, new QTableWidgetItem(fileInfo.lastModified().toString(Qt::ISODate)));
    }
    m_filesTable->resizeColumnsToContents();
}

void IDOSSimulationResultFilesWidget::onOpenRunDirectoryTriggered()
{
    if (!m_runDirectory.isEmpty() && QFileInfo(m_runDirectory).isDir())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_runDirectory));
    }
}

void IDOSSimulationResultFilesWidget::onResultFileDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    QTableWidgetItem* item = m_filesTable->item(row, 0);
    if (item == nullptr)
    {
        return;
    }

    const QString filePath = QDir(m_runDirectory).absoluteFilePath(item->text());
    if (QFileInfo(filePath).isFile())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
}

void IDOSSimulationResultFilesWidget::onImportDynamicPropertiesTriggered()
{
    if (m_project.isNull() || m_gridId.isEmpty())
    {
        IDOS_MESSAGE(tr("The project or target grid is unavailable."), IDOSLogLevel::Error);
        QMessageBox::warning(this, tr("Import Dynamic Properties"), tr("The project or target grid is unavailable."));
        return;
    }

    QTableWidgetItem* item = m_filesTable->item(m_filesTable->currentRow(), 0);
    if (item == nullptr)
    {
        return;
    }

    const QString filePath = QDir(m_runDirectory).absoluteFilePath(item->text());
    if (QFileInfo(filePath).suffix().compare(QStringLiteral("UNRST"), Qt::CaseInsensitive) != 0)
    {
        IDOS_MESSAGE(tr("The selected file is not a unified Eclipse restart file."), IDOSLogLevel::Error);
        QMessageBox::warning(this, tr("Import Dynamic Properties"),
                             tr("The selected file is not a unified Eclipse restart file."));
        return;
    }

    IDOSCommandManager* commandManager = m_project->commandManager();
    if (commandManager == nullptr)
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("gridId"), m_gridId);
    arguments.insert(QStringLiteral("filePath"), filePath);
    if (!commandManager->execute(QStringLiteral("simulation.results.import"), arguments)
             .value(QStringLiteral("success"))
             .toBool())
    {
        IDOS_MESSAGE(commandManager->error(), IDOSLogLevel::Error);
        QMessageBox::warning(this, tr("Import Dynamic Properties"), commandManager->error());
        return;
    }

    const QJsonObject result = commandManager->result();
    m_summaryLabel->setText(tr("Imported %1 dynamic properties across %2 report steps (%3–%4).")
                                .arg(result.value(QStringLiteral("propertyCount")).toInt())
                                .arg(result.value(QStringLiteral("reportStepCount")).toInt())
                                .arg(result.value(QStringLiteral("firstReportStep")).toInt())
                                .arg(result.value(QStringLiteral("lastReportStep")).toInt()));
    IDOS_MESSAGE(m_summaryLabel->text(), IDOSLogLevel::Info);
}

void IDOSSimulationResultFilesWidget::onSelectionChanged()
{
    QTableWidgetItem* item = m_filesTable->item(m_filesTable->currentRow(), 0);
    const bool isRestartFile =
        item != nullptr && QFileInfo(item->text()).suffix().compare(QStringLiteral("UNRST"), Qt::CaseInsensitive) == 0;
    m_importButton->setEnabled(!m_project.isNull() && !m_gridId.isEmpty() && isRestartFile);
}
