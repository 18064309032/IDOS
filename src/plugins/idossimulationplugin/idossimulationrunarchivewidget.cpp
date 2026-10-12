#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QVBoxLayout>

#include "idossimulationcaseobject.h"
#include "log/idoslogger.h"

#include "idossimulationrunarchivewidget.h"

IDOSSimulationRunArchiveWidget::IDOSSimulationRunArchiveWidget(IDOSSimulationCaseObject* simulationCase,
                                                               QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_caseName(simulationCase != nullptr ? simulationCase->name() : QString())
    , m_runInfo(simulationCase != nullptr ? simulationCase->runInfo() : QVariantMap())
    , m_archiveDirectoryEdit(nullptr)
    , m_outputEdit(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationRunArchiveWindow"));
    setWindowTitle(tr("Run Archive — %1").arg(m_caseName));

    const QString runDirectory = m_runInfo.value(QStringLiteral("runDirectory")).toString();
    const QFileInfo runInfoFile(runDirectory);
    const QString archiveName =
        QStringLiteral("%1_archive_%2")
            .arg(runInfoFile.fileName(), QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString defaultArchiveDirectory = runInfoFile.dir().absoluteFilePath(archiveName);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* descriptionLabel = new QLabel(tr("Create a separate directory snapshot of this initialized run, including "
                                             "its inputs, generated files, and a run manifest."),
                                          this);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);

    QHBoxLayout* directoryLayout = new QHBoxLayout();
    directoryLayout->addWidget(new QLabel(tr("Archive directory"), this));
    m_archiveDirectoryEdit = new QLineEdit(defaultArchiveDirectory, this);
    directoryLayout->addWidget(m_archiveDirectoryEdit, 1);
    QPushButton* browseButton = new QPushButton(tr("Browse…"), this);
    directoryLayout->addWidget(browseButton);
    layout->addLayout(directoryLayout);
    connect(browseButton, &QPushButton::clicked, this, &IDOSSimulationRunArchiveWidget::onBrowseDirectoryTriggered);

    m_outputEdit = new QPlainTextEdit(this);
    m_outputEdit->setReadOnly(true);
    layout->addWidget(m_outputEdit, 1);

    QPushButton* createArchiveButton = new QPushButton(tr("Create Archive Snapshot"), this);
    layout->addWidget(createArchiveButton);
    connect(createArchiveButton, &QPushButton::clicked, this,
            &IDOSSimulationRunArchiveWidget::onCreateArchiveTriggered);
    resize(760, 460);
}

void IDOSSimulationRunArchiveWidget::onBrowseDirectoryTriggered()
{
    const QString selectedDirectory = QFileDialog::getExistingDirectory(
        this, tr("Select archive parent directory"), QFileInfo(m_archiveDirectoryEdit->text()).absolutePath());
    if (selectedDirectory.isEmpty())
    {
        return;
    }

    const QString directoryName = QFileInfo(m_archiveDirectoryEdit->text()).fileName();
    m_archiveDirectoryEdit->setText(QDir(selectedDirectory).absoluteFilePath(directoryName));
}

void IDOSSimulationRunArchiveWidget::onCreateArchiveTriggered()
{
    const QString sourceDirectory = QDir::cleanPath(m_runInfo.value(QStringLiteral("runDirectory")).toString());
    const QFileInfo sourceInfo(sourceDirectory);
    if (sourceDirectory.isEmpty() || !sourceInfo.isDir())
    {
        showOutput(tr("The initialized run directory is missing."));
        return;
    }

    const QString archiveDirectory = QDir::cleanPath(QFileInfo(m_archiveDirectoryEdit->text()).absoluteFilePath());
    const QString relativeTarget = QDir(sourceDirectory).relativeFilePath(archiveDirectory);
    const bool targetIsInsideSource =
        relativeTarget == QStringLiteral(".") ||
        (!QDir::isAbsolutePath(relativeTarget) && !relativeTarget.startsWith(QStringLiteral("../")) &&
         !relativeTarget.startsWith(QStringLiteral("..\\")));
    if (targetIsInsideSource)
    {
        showOutput(tr("Choose an archive directory outside the run directory."));
        return;
    }
    if (QFileInfo::exists(archiveDirectory))
    {
        showOutput(tr("The archive directory already exists. Choose a new directory."));
        return;
    }

    const QString archiveParent = QFileInfo(archiveDirectory).absolutePath();
    if (!QDir().mkpath(archiveParent))
    {
        showOutput(tr("Unable to create archive parent directory: %1").arg(archiveParent));
        return;
    }

    QTemporaryDir stagingDirectory(QDir(archiveParent).absoluteFilePath(QStringLiteral(".idos_archive_XXXXXX")));
    if (!stagingDirectory.isValid())
    {
        showOutput(tr("Unable to create a temporary archive directory in %1.").arg(archiveParent));
        return;
    }

    QStringList archivedFiles;
    IDOS_MESSAGE(tr("Creating run archive: %1").arg(archiveDirectory), IDOSLogLevel::Info);
    QStringList errors;
    QDirIterator iterator(sourceDirectory, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        const QString sourcePath = iterator.next();
        const QFileInfo sourceEntryInfo(sourcePath);
        const QString relativePath = QDir::fromNativeSeparators(QDir(sourceDirectory).relativeFilePath(sourcePath));
        const QString targetPath = QDir(stagingDirectory.path()).absoluteFilePath(relativePath);
        if (sourceEntryInfo.isSymLink())
        {
            errors.append(tr("Symbolic links are not supported in run archives: %1").arg(relativePath));
            continue;
        }
        if (sourceEntryInfo.isDir())
        {
            if (!QDir().mkpath(targetPath))
            {
                errors.append(tr("Unable to create archive subdirectory: %1").arg(relativePath));
            }
            continue;
        }

        if (!QDir().mkpath(QFileInfo(targetPath).absolutePath()) || !QFile::copy(sourcePath, targetPath))
        {
            errors.append(tr("Unable to archive file: %1").arg(relativePath));
            continue;
        }
        if (!QFileInfo(targetPath).exists() || QFileInfo(targetPath).size() != sourceEntryInfo.size())
        {
            errors.append(tr("Archived file failed verification: %1").arg(relativePath));
            continue;
        }
        archivedFiles.append(relativePath);
    }

    if (!errors.isEmpty())
    {
        showOutput(errors.join(QLatin1Char('\n')));
        return;
    }

    QJsonArray filesArray;
    for (const QString& file : archivedFiles)
    {
        filesArray.append(file);
    }
    QJsonObject manifest;
    manifest.insert(QStringLiteral("caseName"), m_caseName);
    manifest.insert(QStringLiteral("sourceRunDirectory"), sourceDirectory);
    manifest.insert(QStringLiteral("runStatus"), m_runInfo.value(QStringLiteral("status")).toString());
    manifest.insert(QStringLiteral("initializedAt"), m_runInfo.value(QStringLiteral("initializedAt")).toString());
    manifest.insert(QStringLiteral("archivedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    manifest.insert(QStringLiteral("files"), filesArray);

    const QByteArray manifestContents = QJsonDocument(manifest).toJson(QJsonDocument::Indented);
    QSaveFile manifestFile(QDir(stagingDirectory.path()).absoluteFilePath(QStringLiteral("idos_run_archive.json")));
    if (!manifestFile.open(QIODevice::WriteOnly) || manifestFile.write(manifestContents) != manifestContents.size() ||
        !manifestFile.commit())
    {
        showOutput(tr("Unable to write the run archive manifest."));
        return;
    }

    const bool renamed =
        QDir(archiveParent)
            .rename(QFileInfo(stagingDirectory.path()).fileName(), QFileInfo(archiveDirectory).fileName());
    if (!renamed)
    {
        showOutput(tr("Unable to finalize the run archive: %1").arg(archiveDirectory));
        return;
    }

    QStringList messages;
    messages.append(tr("Run archive created: %1").arg(archiveDirectory));
    messages.append(tr("Archived %1 file(s), plus the run manifest.").arg(archivedFiles.size()));
    showOutput(messages.join(QLatin1Char('\n')), IDOSLogLevel::Info);
}

void IDOSSimulationRunArchiveWidget::showOutput(const QString& message, IDOSLogLevel level)
{
    m_outputEdit->setPlainText(message);
    const QStringList lines = message.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines)
    {
        IDOSLogger::instance().message(line, level);
    }
}
