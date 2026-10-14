#include <QCloseEvent>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextCursor>
#include <QTimer>
#include <QUuid>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "data/case/idoscaseitemref.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/grid/idosgrid.h"
#include "idoscommandmanager.h"
#include "idosproject.h"
#include "logger/idoslogger.h"

#include "idossimulationrunstatuswidget.h"

IDOSSimulationRunStatusWidget::IDOSSimulationRunStatusWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_case(simulationCase)
    , m_project(simulationCase != nullptr ? qobject_cast<IDOSProject*>(simulationCase->parent()) : nullptr)
    , m_caseId(simulationCase != nullptr ? simulationCase->objectId() : QString())
    , m_gridId()
    , m_outputDirectory()
    , m_pendingOutput()
    , m_cancelled(false)
    , m_invalidated(false)
    , m_stopAttempts(0)
    , m_process(new QProcess(this))
    , m_stopProcess(new QProcess(this))
    , m_stopTimer(new QTimer(this))
    , m_distributionEdit(nullptr)
    , m_executableEdit(nullptr)
    , m_threadsSpin(nullptr)
    , m_startButton(nullptr)
    , m_stopButton(nullptr)
    , m_outputEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_directoryLabel(nullptr)
    , m_initializedAtLabel(nullptr)
    , m_filesTable(nullptr)
{
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
    QSettings settings;
    QFormLayout* configuration = new QFormLayout();
    m_distributionEdit = new QLineEdit(
        settings.value(QStringLiteral("simulation/distribution"), QStringLiteral("Ubuntu-24.04")).toString(), this);
    m_distributionEdit->setObjectName(QStringLiteral("flowDistributionEdit"));
    m_executableEdit = new QLineEdit(
        settings.value(QStringLiteral("simulation/executable"), QStringLiteral("/usr/bin/flow")).toString(), this);
    m_executableEdit->setObjectName(QStringLiteral("flowExecutableEdit"));
    m_threadsSpin = new QSpinBox(this);
    m_threadsSpin->setRange(1, 256);
    m_threadsSpin->setValue(settings.value(QStringLiteral("simulation/threads"), 2).toInt());
    configuration->addRow(tr("Linux distribution"), m_distributionEdit);
    configuration->addRow(tr("Flow executable"), m_executableEdit);
    configuration->addRow(tr("CPU threads"), m_threadsSpin);
    layout->addLayout(configuration);
    QHBoxLayout* buttons = new QHBoxLayout();
    m_startButton = new QPushButton(tr("Start Flow"), this);
    m_startButton->setObjectName(QStringLiteral("flowStartButton"));
    m_stopButton = new QPushButton(tr("Stop Task"), this);
    m_stopButton->setEnabled(false);
    buttons->addWidget(m_startButton);
    buttons->addWidget(m_stopButton);
    layout->addLayout(buttons);
    m_outputEdit = new QPlainTextEdit(this);
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setMaximumBlockCount(5000);
    layout->addWidget(m_outputEdit, 2);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_startButton, &QPushButton::clicked, this, &IDOSSimulationRunStatusWidget::onStartTriggered);
    connect(m_stopButton, &QPushButton::clicked, this, &IDOSSimulationRunStatusWidget::onStopTriggered);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &IDOSSimulationRunStatusWidget::onOutputReady);
    connect(m_process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            &IDOSSimulationRunStatusWidget::onFinished);
    connect(m_process, &QProcess::errorOccurred, this, &IDOSSimulationRunStatusWidget::onProcessError);
    connect(m_stopTimer, &QTimer::timeout, this, &IDOSSimulationRunStatusWidget::onStopTimer);
    if (m_project != nullptr)
    {
        connect(m_project, &IDOSProject::objectAboutToBeRemoved, this,
                &IDOSSimulationRunStatusWidget::onObjectRemoving);
        connect(m_project, &QObject::destroyed, this, &IDOSSimulationRunStatusWidget::onProjectDestroyed);
    }
    resize(900, 720);
}

IDOSSimulationRunStatusWidget::~IDOSSimulationRunStatusWidget()
{
    if (m_process->state() != QProcess::NotRunning)
    {
        stop();
        for (int attempt = 0; attempt < 8 && m_process->state() != QProcess::NotRunning; ++attempt)
        {
            onStopTimer();
            m_stopProcess->waitForFinished(1000);
            m_process->waitForFinished(500);
        }
    }
    m_stopTimer->stop();
    if (m_stopProcess->state() != QProcess::NotRunning)
    {
        m_stopProcess->waitForFinished(3000);
    }
}

bool IDOSSimulationRunStatusWidget::isRunning() const
{
    return m_process->state() != QProcess::NotRunning || m_stopProcess->state() != QProcess::NotRunning;
}

void IDOSSimulationRunStatusWidget::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}

QString IDOSSimulationRunStatusWidget::linuxPath(const QString& path) const
{
    const QString normalized = QDir::fromNativeSeparators(QFileInfo(path).absoluteFilePath());
    if (normalized.size() < 3 || normalized.at(1) != QLatin1Char(':') || normalized.at(2) != QLatin1Char('/'))
    {
        return QString();
    }
    return QStringLiteral("/mnt/") + normalized.left(1).toLower() + normalized.mid(2);
}

void IDOSSimulationRunStatusWidget::setStatus(const QString& status, const QString& message)
{
    m_statusLabel->setText(message);
    m_outputEdit->appendPlainText(message);
    if (status == QStringLiteral("failed") || status == QStringLiteral("importFailed") ||
        status == QStringLiteral("rejected"))
    {
        IDOS_MESSAGE(message, IDOSLogLevel::Error);
    }
    else if (status == QStringLiteral("cancelled"))
    {
        IDOS_MESSAGE(message, IDOSLogLevel::Warn);
    }
    else
    {
        IDOS_MESSAGE(message, IDOSLogLevel::Info);
    }
    if (status != QStringLiteral("rejected") && m_case != nullptr && m_project != nullptr &&
        m_project->object(m_caseId) == m_case.data())
    {
        QVariantMap info = m_case->runInfo();
        info.insert(QStringLiteral("status"), status);
        info.insert(QStringLiteral("message"), message);
        info.insert(QStringLiteral("outputDirectory"), m_outputDirectory);
        m_case->setRunInfo(info);
    }
    emit runningChanged();
}

void IDOSSimulationRunStatusWidget::onStartTriggered()
{
    if (isRunning() || m_invalidated || m_case == nullptr || m_project == nullptr ||
        m_project->object(m_caseId) != m_case.data())
    {
        return;
    }
    const QVariantMap info = m_case->runInfo();
    const QString directory = info.value(QStringLiteral("runDirectory")).toString();
    const QString dataFile = QDir(directory).absoluteFilePath(info.value(QStringLiteral("dataFile")).toString());
    if (directory.isEmpty() || !QFileInfo(dataFile).isFile() || linuxPath(directory).isEmpty() ||
        m_distributionEdit->text().trimmed().isEmpty() || !m_executableEdit->text().startsWith(QLatin1Char('/')))
    {
        setStatus(QStringLiteral("rejected"),
                  tr("Initialize the case and configure a Linux distribution and absolute Flow path first."));
        return;
    }
    const QStringList inputs = info.value(QStringLiteral("files")).toStringList();
    for (const QString& input : inputs)
    {
        if (!QFileInfo(QDir(directory).absoluteFilePath(input)).isFile())
        {
            setStatus(QStringLiteral("rejected"), tr("An initialized input file is missing: %1").arg(input));
            return;
        }
    }
    m_gridId.clear();
    for (const IDOSCaseItemRef& reference : m_case->itemRefs())
    {
        if (reference.role() == QStringLiteral("case.grid") &&
            qobject_cast<IDOSGrid*>(m_project->object(reference.objectId())) != nullptr)
        {
            m_gridId = reference.objectId();
            break;
        }
    }
    if (m_gridId.isEmpty())
    {
        setStatus(QStringLiteral("rejected"), tr("The target grid is unavailable."));
        return;
    }
    m_outputDirectory = QDir(directory).absoluteFilePath(QStringLiteral("results-") +
                                                         QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (!QDir().mkpath(m_outputDirectory))
    {
        setStatus(QStringLiteral("rejected"), tr("Cannot create the simulation output directory."));
        return;
    }
    QSettings settings;
    settings.setValue(QStringLiteral("simulation/distribution"), m_distributionEdit->text().trimmed());
    settings.setValue(QStringLiteral("simulation/executable"), m_executableEdit->text());
    settings.setValue(QStringLiteral("simulation/threads"), m_threadsSpin->value());
    m_outputEdit->clear();
    m_pendingOutput.clear();
    m_cancelled = false;
    m_stopAttempts = 0;
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    emit runningChanged();
    m_distributionEdit->setEnabled(false);
    m_executableEdit->setEnabled(false);
    m_threadsSpin->setEnabled(false);
    QStringList arguments;
    arguments << QStringLiteral("-d") << m_distributionEdit->text().trimmed() << QStringLiteral("--cd")
              << linuxPath(directory) << QStringLiteral("--exec") << QStringLiteral("/usr/bin/setsid")
              << QStringLiteral("--wait") << QStringLiteral("/bin/sh") << QStringLiteral("-c")
              << QStringLiteral("echo $$ > \"$1\"; shift; exec \"$@\"") << QStringLiteral("idos-flow")
              << linuxPath(QDir(m_outputDirectory).absoluteFilePath(QStringLiteral("flow.pid")))
              << m_executableEdit->text() << linuxPath(dataFile)
              << QStringLiteral("--output-dir=") + linuxPath(m_outputDirectory)
              << QStringLiteral("--threads-per-process=%1").arg(m_threadsSpin->value());
    m_process->start(QStringLiteral("wsl.exe"), arguments);
    setStatus(QStringLiteral("running"), tr("Flow is starting. Output directory: %1").arg(m_outputDirectory));
}

void IDOSSimulationRunStatusWidget::onOutputReady()
{
    const QByteArray output = m_process->readAllStandardOutput();
    m_pendingOutput.append(output);
    forwardOutput(false);
    m_outputEdit->moveCursor(QTextCursor::End);
    m_outputEdit->insertPlainText(QString::fromUtf8(output));
    QFile log(QDir(m_outputDirectory).absoluteFilePath(QStringLiteral("flow-console.log")));
    if (log.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        log.write(output);
    }
}

void IDOSSimulationRunStatusWidget::forwardOutput(bool flush)
{
    while (!m_pendingOutput.isEmpty())
    {
        if (m_pendingOutput.size() >= 4 && m_pendingOutput.at(1) == '\0' && m_pendingOutput.at(3) == '\0')
        {
            const int end = m_pendingOutput.indexOf(QByteArray("\n\0", 2));
            if (end < 0 && !flush)
            {
                return;
            }
            const int length = end >= 0 ? end + 2 : m_pendingOutput.size();
            QString line;
            for (int index = 0; index + 1 < length; index += 2)
            {
                const ushort code = static_cast<uchar>(m_pendingOutput.at(index))
                    | (static_cast<ushort>(static_cast<uchar>(m_pendingOutput.at(index + 1))) << 8);
                line.append(QChar(code));
            }
            m_pendingOutput.remove(0, length);
            if (!line.trimmed().isEmpty())
            {
                IDOS_MESSAGE(line.trimmed(), IDOSLogLevel::Info);
            }
            continue;
        }
        int separator = -1;
        for (int index = 0; index < m_pendingOutput.size(); ++index)
        {
            if (m_pendingOutput.at(index) == '\n' || m_pendingOutput.at(index) == '\r')
            {
                separator = index;
                break;
            }
        }
        if (separator < 0 && !flush)
        {
            return;
        }
        const int length = separator >= 0 ? separator : m_pendingOutput.size();
        QString line = QString::fromUtf8(m_pendingOutput.left(length));
        line.remove(QChar::Null);
        line = line.trimmed();
        m_pendingOutput.remove(0, length + (separator >= 0 ? 1 : 0));
        if (!line.isEmpty())
        {
            IDOS_MESSAGE(line, IDOSLogLevel::Info);
        }
    }
}

void IDOSSimulationRunStatusWidget::stop()
{
    if (!isRunning())
    {
        return;
    }
    m_cancelled = true;
    m_stopButton->setEnabled(false);
    setStatus(QStringLiteral("stopping"), tr("Stopping Flow..."));
    m_stopTimer->start(1000);
    onStopTimer();
    emit runningChanged();
}

void IDOSSimulationRunStatusWidget::onStopTriggered()
{
    stop();
}

void IDOSSimulationRunStatusWidget::onProjectDestroyed()
{
    cancel();
}

void IDOSSimulationRunStatusWidget::cancel()
{
    m_invalidated = true;
    m_cancelled = true;
    m_startButton->setEnabled(false);
    stop();
}

void IDOSSimulationRunStatusWidget::onObjectRemoving(const QString& objectId)
{
    if (objectId == m_caseId || objectId == m_gridId)
    {
        cancel();
    }
}

void IDOSSimulationRunStatusWidget::onStopTimer()
{
    if (!isRunning())
    {
        m_stopTimer->stop();
        return;
    }
    if (m_stopProcess->state() != QProcess::NotRunning)
    {
        return;
    }
    QFile pidFile(QDir(m_outputDirectory).absoluteFilePath(QStringLiteral("flow.pid")));
    if (!pidFile.open(QIODevice::ReadOnly))
    {
        return;
    }
    bool valid = false;
    const qlonglong pid = pidFile.readAll().trimmed().toLongLong(&valid);
    if (!valid || pid <= 1)
    {
        return;
    }
    ++m_stopAttempts;
    QStringList arguments;
    arguments << QStringLiteral("-d") << m_distributionEdit->text().trimmed() << QStringLiteral("--exec")
              << QStringLiteral("/bin/kill") << (m_stopAttempts < 5 ? QStringLiteral("-TERM") : QStringLiteral("-KILL"))
              << QStringLiteral("--") << QStringLiteral("-%1").arg(pid);
    m_stopProcess->start(QStringLiteral("wsl.exe"), arguments);
}

void IDOSSimulationRunStatusWidget::onProcessError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart)
    {
        onFinished(-1, QProcess::CrashExit);
    }
}

void IDOSSimulationRunStatusWidget::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    onOutputReady();
    forwardOutput(true);
    m_stopTimer->stop();
    m_stopButton->setEnabled(false);
    m_startButton->setEnabled(!m_invalidated);
    m_distributionEdit->setEnabled(true);
    m_executableEdit->setEnabled(true);
    m_threadsSpin->setEnabled(true);
    emit runningChanged();
    if (m_case != nullptr && m_project != nullptr && m_project->object(m_caseId) == m_case.data())
    {
        QVariantMap info = m_case->runInfo();
        info.insert(QStringLiteral("exitCode"), exitCode);
        info.insert(QStringLiteral("finishedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        m_case->setRunInfo(info);
    }
    if (m_cancelled)
    {
        setStatus(QStringLiteral("cancelled"), tr("The simulation was cancelled. Results were not imported."));
    }
    else if (exitStatus != QProcess::NormalExit || exitCode != 0)
    {
        setStatus(QStringLiteral("failed"),
                  tr("Flow failed (exit code %1): %2").arg(exitCode).arg(m_process->errorString()));
    }
    else
    {
        importResults();
    }
}

void IDOSSimulationRunStatusWidget::importResults()
{
    if (m_project == nullptr || m_case == nullptr || m_project->object(m_caseId) != m_case.data() ||
        qobject_cast<IDOSGrid*>(m_project->object(m_gridId)) == nullptr)
    {
        setStatus(QStringLiteral("importFailed"),
                  tr("The project or target grid is unavailable. Results were not imported."));
        return;
    }
    bool hasTargetReference = false;
    for (const IDOSCaseItemRef& reference : m_case->itemRefs())
    {
        if (reference.role() == QStringLiteral("case.grid") && reference.objectId() == m_gridId)
        {
            hasTargetReference = true;
            break;
        }
    }
    if (!hasTargetReference)
    {
        setStatus(QStringLiteral("importFailed"), tr("The case grid reference changed. Results were not imported."));
        return;
    }
    const QStringList restartFiles =
        QDir(m_outputDirectory)
            .entryList(QStringList() << QStringLiteral("*.UNRST") << QStringLiteral("*.unrst"), QDir::Files);
    if (restartFiles.size() != 1)
    {
        setStatus(QStringLiteral("importFailed"), tr("Flow finished, but exactly one UNRST result file is required."));
        return;
    }
    IDOSCommandManager* manager = m_project->commandManager();
    IDOS_MESSAGE(tr("Importing Flow results: %1").arg(restartFiles.first()), IDOSLogLevel::Info);
    QJsonObject arguments;
    arguments.insert(QStringLiteral("gridId"), m_gridId);
    arguments.insert(QStringLiteral("filePath"), QDir(m_outputDirectory).absoluteFilePath(restartFiles.first()));
    if (manager == nullptr || !manager->execute(QStringLiteral("simulation.results.import"), arguments)
                                   .value(QStringLiteral("success"))
                                   .toBool())
    {
        setStatus(QStringLiteral("importFailed"),
                  tr("Flow finished, but result import failed: %1")
                      .arg(manager != nullptr ? manager->error() : tr("The command manager is unavailable.")));
        return;
    }
    setStatus(QStringLiteral("succeeded"),
              tr("Flow completed and dynamic properties were imported. You can undo the import."));
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
        m_statusLabel->setText(runInfo.value(QStringLiteral("message"), tr("Run status: %1").arg(status)).toString());
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
