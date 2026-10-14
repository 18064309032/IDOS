#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTemporaryDir>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "data/case/idossimulationcaseobject.h"
#include "logger/idoslogger.h"

#include "idossimulationinitializewidget.h"

IDOSSimulationInitializeWidget::IDOSSimulationInitializeWidget(IDOSSimulationCaseObject* simulationCase,
                                                               QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_sourceFile(simulationCase != nullptr ? simulationCase->sourceFile() : QString())
    , m_inputFiles(inputFiles(simulationCase))
    , m_simulationCase(simulationCase)
    , m_directoryEdit(nullptr)
    , m_inputTable(nullptr)
    , m_outputEdit(nullptr)
    , m_initializeButton(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationInitializeWindow"));
    setWindowTitle(tr("Initialize — %1").arg(simulationCase != nullptr ? simulationCase->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* descriptionLabel = new QLabel(tr("Prepare an isolated run directory and copy the DATA file with its "
                                             "indexed input files. The simulator will not be started."),
                                          this);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);

    QHBoxLayout* directoryLayout = new QHBoxLayout();
    directoryLayout->addWidget(new QLabel(tr("Run directory"), this));
    m_directoryEdit = new QLineEdit(this);
    const QFileInfo sourceInfo(m_sourceFile);
    const QString defaultDirectory =
        sourceInfo.dir().absoluteFilePath(QStringLiteral("idos_runs/%1").arg(sourceInfo.completeBaseName()));
    m_directoryEdit->setText(defaultDirectory);
    directoryLayout->addWidget(m_directoryEdit, 1);
    QPushButton* browseButton = new QPushButton(tr("Browse…"), this);
    directoryLayout->addWidget(browseButton);
    layout->addLayout(directoryLayout);
    connect(browseButton, &QPushButton::clicked, this, &IDOSSimulationInitializeWidget::onBrowseDirectoryTriggered);

    m_inputTable = new QTableWidget(this);
    m_inputTable->setColumnCount(2);
    m_inputTable->setHorizontalHeaderLabels(QStringList() << tr("Source file") << tr("Run directory path"));
    m_inputTable->setEditTriggers(QTableWidget::NoEditTriggers);
    m_inputTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_inputTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_inputTable, 2);

    m_outputEdit = new QPlainTextEdit(this);
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setMaximumBlockCount(1000);
    layout->addWidget(m_outputEdit, 1);

    m_initializeButton = new QPushButton(tr("Prepare Run Directory"), this);
    layout->addWidget(m_initializeButton);
    connect(m_initializeButton, &QPushButton::clicked, this, &IDOSSimulationInitializeWidget::onInitializeTriggered);

    updateInputTable();
    resize(820, 620);
}

QStringList IDOSSimulationInitializeWidget::inputFiles(IDOSSimulationCaseObject* simulationCase) const
{
    QStringList files;
    if (m_sourceFile.isEmpty())
    {
        return files;
    }

    files.append(QFileInfo(m_sourceFile).absoluteFilePath());
    if (simulationCase == nullptr)
    {
        return files;
    }

    const QVariantList keywordIndex = simulationCase->parsedSourceData().value(QStringLiteral("keywordIndex")).toList();
    for (const QVariant& keywordValue : keywordIndex)
    {
        const QString filePath = keywordValue.toMap().value(QStringLiteral("file")).toString();
        if (!filePath.isEmpty())
        {
            const QString absoluteFilePath = QFileInfo(filePath).absoluteFilePath();
            if (!files.contains(absoluteFilePath, Qt::CaseInsensitive))
            {
                files.append(absoluteFilePath);
            }
        }
    }
    return files;
}

QString IDOSSimulationInitializeWidget::relativeInputPath(const QString& inputFile) const
{
    const QFileInfo sourceInfo(m_sourceFile);
    const QFileInfo inputInfo(inputFile);
    QString relativePath = QDir(sourceInfo.absolutePath()).relativeFilePath(inputInfo.absoluteFilePath());
    if (relativePath == QStringLiteral("..") || relativePath.startsWith(QStringLiteral("../")) ||
        relativePath.startsWith(QStringLiteral("..\\")))
    {
        relativePath = QStringLiteral("_external/%1").arg(inputInfo.fileName());
    }
    return QDir::fromNativeSeparators(relativePath);
}

bool IDOSSimulationInitializeWidget::validateInputs(QStringList* messages) const
{
    if (m_sourceFile.isEmpty())
    {
        messages->append(tr("The source DATA file is not set."));
        return false;
    }
    if (m_inputFiles.isEmpty())
    {
        messages->append(tr("No input files are available to initialize."));
        return false;
    }

    bool valid = true;
    QStringList targetPaths;
    for (const QString& inputFile : m_inputFiles)
    {
        const QFileInfo inputInfo(inputFile);
        if (!inputInfo.exists() || !inputInfo.isFile() || !inputInfo.isReadable())
        {
            messages->append(tr("Input file is missing or unreadable: %1").arg(inputFile));
            valid = false;
            continue;
        }

        const QString relativePath = relativeInputPath(inputFile);
        if (relativePath.startsWith(QStringLiteral("_external/")))
        {
            messages->append(
                tr("Input file is outside the DATA directory tree and cannot be relocated safely: %1").arg(inputFile));
            valid = false;
        }
        if (targetPaths.contains(relativePath, Qt::CaseInsensitive))
        {
            messages->append(tr("Multiple input files map to the same run path: %1").arg(relativePath));
            valid = false;
        }
        targetPaths.append(relativePath);
    }
    return valid;
}

void IDOSSimulationInitializeWidget::updateInputTable()
{
    m_inputTable->setRowCount(m_inputFiles.size());
    for (int row = 0; row < m_inputFiles.size(); ++row)
    {
        m_inputTable->setItem(row, 0, new QTableWidgetItem(m_inputFiles.at(row)));
        m_inputTable->setItem(row, 1, new QTableWidgetItem(relativeInputPath(m_inputFiles.at(row))));
    }
    m_inputTable->resizeColumnsToContents();
}

void IDOSSimulationInitializeWidget::onBrowseDirectoryTriggered()
{
    const QString selectedDirectory = QFileDialog::getExistingDirectory(
        this, tr("Select run directory parent"), QFileInfo(m_directoryEdit->text()).absolutePath());
    if (selectedDirectory.isEmpty())
    {
        return;
    }

    const QString directoryName = QFileInfo(m_directoryEdit->text()).fileName();
    m_directoryEdit->setText(QDir(selectedDirectory).absoluteFilePath(directoryName));
}

void IDOSSimulationInitializeWidget::onInitializeTriggered()
{
    QStringList messages;
    if (!validateInputs(&messages))
    {
        showOutput(messages.join(QLatin1Char('\n')));
        return;
    }

    const QString targetDirectory = QDir::cleanPath(QFileInfo(m_directoryEdit->text()).absoluteFilePath());
    const QFileInfo targetInfo(targetDirectory);
    if (targetDirectory.isEmpty() || targetDirectory == QFileInfo(m_sourceFile).absolutePath())
    {
        showOutput(tr("Choose a run directory separate from the source DATA directory."));
        return;
    }
    if (targetInfo.exists())
    {
        showOutput(tr("The run directory already exists. Choose a new directory to avoid overwriting existing files."));
        return;
    }

    const QString parentDirectory = targetInfo.absolutePath();
    if (!QDir().mkpath(parentDirectory))
    {
        showOutput(tr("Unable to create the parent directory: %1").arg(parentDirectory));
        return;
    }

    QTemporaryDir stagingDirectory(QDir(parentDirectory).absoluteFilePath(QStringLiteral(".idos_initialize_XXXXXX")));
    if (!stagingDirectory.isValid())
    {
        showOutput(tr("Unable to create a temporary initialization directory in %1.").arg(parentDirectory));
        return;
    }

    QStringList copiedPaths;
    IDOS_MESSAGE(tr("Preparing run directory: %1").arg(targetDirectory), IDOSLogLevel::Info);
    for (const QString& inputFile : m_inputFiles)
    {
        const QString relativePath = relativeInputPath(inputFile);
        IDOS_MESSAGE(tr("Copying input file: %1").arg(relativePath), IDOSLogLevel::Info);
        const QString stagedPath = QDir(stagingDirectory.path()).absoluteFilePath(relativePath);
        if (!QDir().mkpath(QFileInfo(stagedPath).absolutePath()) || !QFile::copy(inputFile, stagedPath))
        {
            messages.append(tr("Unable to copy input file: %1").arg(inputFile));
        }
        else
        {
            const QFileInfo sourceInfo(inputFile);
            const QFileInfo copiedInfo(stagedPath);
            if (!copiedInfo.exists() || copiedInfo.size() != sourceInfo.size())
            {
                messages.append(tr("Copied input file failed verification: %1").arg(inputFile));
            }
            else
            {
                copiedPaths.append(relativePath);
            }
        }
    }

    if (!messages.isEmpty())
    {
        showOutput(messages.join(QLatin1Char('\n')));
        return;
    }

    const bool renamed =
        QDir(parentDirectory)
            .rename(QFileInfo(stagingDirectory.path()).fileName(), QFileInfo(targetDirectory).fileName());
    if (!renamed)
    {
        showOutput(tr("Unable to finalize the run directory: %1").arg(targetDirectory));
        return;
    }

    messages.append(tr("Run directory prepared: %1").arg(targetDirectory));
    messages.append(tr("Copied %1 input file(s):").arg(copiedPaths.size()));
    messages.append(copiedPaths);
    messages.append(tr("Input files were copied and verified. The simulator has not been started."));
    showOutput(messages.join(QLatin1Char('\n')), IDOSLogLevel::Info);
    if (m_simulationCase != nullptr)
    {
        QVariantMap runInfo;
        runInfo.insert(QStringLiteral("status"), QStringLiteral("initialized"));
        runInfo.insert(QStringLiteral("runDirectory"), targetDirectory);
        runInfo.insert(QStringLiteral("dataFile"), relativeInputPath(m_sourceFile));
        runInfo.insert(QStringLiteral("files"), copiedPaths);
        runInfo.insert(QStringLiteral("initializedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        m_simulationCase->setRunInfo(runInfo);
    }
    m_initializeButton->setEnabled(false);
}

void IDOSSimulationInitializeWidget::showOutput(const QString& message, IDOSLogLevel level)
{
    m_outputEdit->setPlainText(message);
    const QStringList lines = message.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines)
    {
        IDOSLogger::instance().message(line, level);
    }
}
