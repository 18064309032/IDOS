#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QVariantList>
#include <QVariantMap>

#include "data/case/idossimulationcaseobject.h"
#include "logger/idoslogger.h"

#include "idossimulationinitializewidget.h"

IDOSSimulationInitializeWidget::IDOSSimulationInitializeWidget(IDOSSimulationCaseObject* simulationCase,
                                                               QObject* parent)
    :
    QObject(parent)
    , m_sourceFile(simulationCase != nullptr ? simulationCase->sourceFile() : QString())
    , m_inputFiles()
    , m_simulationCase(simulationCase)
    , m_runDirectory()
    , m_preparedRunInfo()
{
    setObjectName(QStringLiteral("simulationRunPreparer"));
    if (simulationCase != nullptr)
    {
        const QString generatedDataFile =
            simulationCase->runInfo().value(QStringLiteral("generatedDataFile")).toString();
        if (QFileInfo(generatedDataFile).isFile())
        {
            m_sourceFile = QFileInfo(generatedDataFile).absoluteFilePath();
        }
    }
    m_inputFiles = inputFiles(simulationCase);

    const QFileInfo sourceInfo(m_sourceFile);
    const QString defaultDirectory =
        sourceInfo.dir().absoluteFilePath(QStringLiteral("idos_runs/%1").arg(sourceInfo.completeBaseName()));
    m_runDirectory = defaultDirectory;
}

void IDOSSimulationInitializeWidget::setRunDirectory(const QString& directory)
{
    m_runDirectory = directory;
}

void IDOSSimulationInitializeWidget::setGeneratedDataFile(const QString& filePath)
{
    m_sourceFile = QFileInfo(filePath).absoluteFilePath();
    m_inputFiles.clear();
    m_inputFiles.append(m_sourceFile);
}

QVariantMap IDOSSimulationInitializeWidget::preparedRunInfo() const
{
    return m_preparedRunInfo;
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

    const QString generatedDataFile =
        QFileInfo(simulationCase->runInfo().value(QStringLiteral("generatedDataFile")).toString()).absoluteFilePath();
    if (!generatedDataFile.isEmpty() &&
        generatedDataFile.compare(QFileInfo(m_sourceFile).absoluteFilePath(), Qt::CaseInsensitive) == 0)
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

bool IDOSSimulationInitializeWidget::prepareRunDirectory()
{
    QStringList messages;
    if (!validateInputs(&messages))
    {
        showOutput(messages.join(QLatin1Char('\n')));
        return false;
    }

    const QString targetDirectory = QDir::cleanPath(QFileInfo(m_runDirectory).absoluteFilePath());
    const QFileInfo targetInfo(targetDirectory);
    if (targetDirectory.isEmpty() || targetDirectory == QFileInfo(m_sourceFile).absolutePath())
    {
        showOutput(tr("Choose a run directory separate from the source DATA directory."));
        return false;
    }
    if (targetInfo.exists())
    {
        showOutput(tr("The run directory already exists. Choose a new directory to avoid overwriting existing files."));
        return false;
    }

    const QString parentDirectory = targetInfo.absolutePath();
    if (!QDir().mkpath(parentDirectory))
    {
        showOutput(tr("Unable to create the parent directory: %1").arg(parentDirectory));
        return false;
    }

    QTemporaryDir stagingDirectory(QDir(parentDirectory).absoluteFilePath(QStringLiteral(".idos_initialize_XXXXXX")));
    if (!stagingDirectory.isValid())
    {
        showOutput(tr("Unable to create a temporary initialization directory in %1.").arg(parentDirectory));
        return false;
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
        return false;
    }

    const bool renamed =
        QDir(parentDirectory)
            .rename(QFileInfo(stagingDirectory.path()).fileName(), QFileInfo(targetDirectory).fileName());
    if (!renamed)
    {
        showOutput(tr("Unable to finalize the run directory: %1").arg(targetDirectory));
        return false;
    }

    messages.append(tr("Run directory prepared: %1").arg(targetDirectory));
    messages.append(tr("Copied %1 input file(s):").arg(copiedPaths.size()));
    messages.append(copiedPaths);
    messages.append(tr("Input files were copied and verified. Starting the simulator."));
    showOutput(messages.join(QLatin1Char('\n')), IDOSLogLevel::Info);
    m_preparedRunInfo.clear();
    m_preparedRunInfo.insert(QStringLiteral("status"), QStringLiteral("initialized"));
    m_preparedRunInfo.insert(QStringLiteral("runDirectory"), targetDirectory);
    m_preparedRunInfo.insert(QStringLiteral("dataFile"), relativeInputPath(m_sourceFile));
    m_preparedRunInfo.insert(QStringLiteral("files"), copiedPaths);
    m_preparedRunInfo.insert(QStringLiteral("initializedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    return true;
}

void IDOSSimulationInitializeWidget::showOutput(const QString& message, IDOSLogLevel level)
{
    const QStringList lines = message.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines)
    {
        IDOSLogger::instance().message(line, level);
    }
}
