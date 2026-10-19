#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSettings>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextCursor>
#include <QTimer>
#include <QUuid>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "command/idoscommandmanager.h"
#include "data/case/idoscaseitemref.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/grid/idosgridobject.h"
#include "idosproject.h"
#include "logger/idoslogger.h"

#include "idossimulationrunstatuswidget.h"

IDOSSimulationRunStatusWidget::IDOSSimulationRunStatusWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent)
    :
    QWidget(parent)
    , m_case(simulationCase)
    , m_project(simulationCase != nullptr ? qobject_cast<IDOSProject*>(simulationCase->parent()) : nullptr)
    , m_caseId(simulationCase != nullptr ? simulationCase->objectId() : QString())
    , m_gridId()
    , m_outputDirectory()
    , m_currentRunId()
    , m_pendingOutput()
    , m_cancelled(false)
    , m_invalidated(false)
    , m_stopAttempts(0)
    , m_process(new QProcess(this))
    , m_stopProcess(new QProcess(this))
    , m_stopTimer(new QTimer(this))
    , m_settingsSummaryLabel(nullptr)
    , m_outputEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_historyTable(nullptr)
{
    setObjectName(QStringLiteral("simulationRunStatusPage"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    m_settingsSummaryLabel = new QLabel(this);
    m_settingsSummaryLabel->setWordWrap(true);
    layout->addWidget(m_settingsSummaryLabel);
    updateSettingsSummary();

    QLabel* historyTitle = new QLabel(tr("Run History"), this);
    historyTitle->setStyleSheet(QStringLiteral("font-weight: 600; color: #26374a;"));
    layout->addWidget(historyTitle);
    m_historyTable = new QTableWidget(this);
    m_historyTable->setObjectName(QStringLiteral("simulationRunHistoryTable"));
    m_historyTable->setColumnCount(3);
    m_historyTable->setHorizontalHeaderLabels(QStringList() << tr("Started") << tr("Status") << tr("Output"));
    m_historyTable->setEditTriggers(QTableWidget::NoEditTriggers);
    m_historyTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_historyTable->setSelectionMode(QTableWidget::SingleSelection);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setToolTip(tr("Double-click a run to inspect its output files."));
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_historyTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_historyTable->setMaximumHeight(170);
    layout->addWidget(m_historyTable);

    m_outputEdit = new QPlainTextEdit(this);
    m_outputEdit->setObjectName(QStringLiteral("simulationRunOutput"));
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setMaximumBlockCount(5000);
    m_outputEdit->setMaximumHeight(150);
    layout->addWidget(m_outputEdit);
    loadStatus(simulationCase);
    refreshRunHistory();
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_historyTable, &QTableWidget::cellDoubleClicked, this,
            &IDOSSimulationRunStatusWidget::onHistoryRowDoubleClicked);
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
}

void IDOSSimulationRunStatusWidget::refreshSettingsSummary()
{
    updateSettingsSummary();
}

void IDOSSimulationRunStatusWidget::refreshRunHistory()
{
    m_historyTable->setRowCount(0);
    if (m_case.isNull())
    {
        return;
    }

    const QVariantList history = m_case->runInfo().value(QStringLiteral("history")).toList();
    for (int historyIndex = history.size() - 1; historyIndex >= 0; --historyIndex)
    {
        const QVariantMap record = history.at(historyIndex).toMap();
        const int row = m_historyTable->rowCount();
        m_historyTable->insertRow(row);
        const QDateTime startedAt = QDateTime::fromString(record.value(QStringLiteral("startedAt")).toString(),
                                                          Qt::ISODate);
        const QString startedText = startedAt.isValid()
            ? startedAt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
            : record.value(QStringLiteral("startedAt")).toString();
        const QString status = record.value(QStringLiteral("status")).toString();
        QString statusText = status;
        if (status == QStringLiteral("running"))
        {
            statusText = tr("Running");
        }
        else if (status == QStringLiteral("succeeded"))
        {
            statusText = tr("Succeeded");
        }
        else if (status == QStringLiteral("failed"))
        {
            statusText = tr("Failed");
        }
        else if (status == QStringLiteral("importFailed"))
        {
            statusText = tr("Import failed");
        }
        else if (status == QStringLiteral("cancelled"))
        {
            statusText = tr("Cancelled");
        }
        const QString outputDirectory = record.value(QStringLiteral("outputDirectory")).toString();
        QTableWidgetItem* startedItem = new QTableWidgetItem(startedText);
        startedItem->setData(Qt::UserRole, record.value(QStringLiteral("runId")).toString());
        QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);
        statusItem->setToolTip(record.value(QStringLiteral("message")).toString());
        QTableWidgetItem* outputItem = new QTableWidgetItem(outputDirectory);
        outputItem->setToolTip(outputDirectory);
        outputItem->setData(Qt::UserRole, outputDirectory);
        m_historyTable->setItem(row, 0, startedItem);
        m_historyTable->setItem(row, 1, statusItem);
        m_historyTable->setItem(row, 2, outputItem);
    }
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
}

void IDOSSimulationRunStatusWidget::startFlow()
{
    onStartTriggered();
}

void IDOSSimulationRunStatusWidget::reportRunRequestFailure(const QString& message)
{
    setStatus(QStringLiteral("rejected"), message);
}

void IDOSSimulationRunStatusWidget::updateSettingsSummary()
{
    QSettings settings;
    const QString runDirectoryRoot =
        settings.value(QStringLiteral("simulation/runDirectoryRoot")).toString().trimmed();
    const QString directorySummary = runDirectoryRoot.isEmpty()
        ? tr("Beside case input")
        : runDirectoryRoot;
    const QString distribution =
        settings.value(QStringLiteral("simulation/distribution"), QStringLiteral("Ubuntu-24.04")).toString();
    const QString executable =
        settings.value(QStringLiteral("simulation/executable"), QStringLiteral("/usr/bin/flow")).toString();
    const int threads = settings.value(QStringLiteral("simulation/threads"), 2).toInt();
    m_settingsSummaryLabel->setText(tr("Run folder: %1 · WSL: %2 · Flow: %3 · %4 threads")
                                        .arg(directorySummary, distribution, executable)
                                        .arg(threads));
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

bool IDOSSimulationRunStatusWidget::isForCase(const QString& caseId) const
{
    return m_caseId == caseId;
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
        QVariantList history = info.value(QStringLiteral("history")).toList();
        for (int historyIndex = history.size() - 1; historyIndex >= 0; --historyIndex)
        {
            QVariantMap record = history.at(historyIndex).toMap();
            if (record.value(QStringLiteral("runId")).toString() == m_currentRunId)
            {
                record.insert(QStringLiteral("status"), status);
                record.insert(QStringLiteral("message"), message);
                record.insert(QStringLiteral("outputDirectory"), m_outputDirectory);
                if (status == QStringLiteral("succeeded") || status == QStringLiteral("failed") ||
                    status == QStringLiteral("importFailed") || status == QStringLiteral("cancelled"))
                {
                    record.insert(QStringLiteral("finishedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
                }
                history[historyIndex] = record;
                break;
            }
        }
        info.insert(QStringLiteral("history"), history);
        m_case->setRunInfo(info);
        refreshRunHistory();
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
    QSettings settings;
    const QString distribution =
        settings.value(QStringLiteral("simulation/distribution"), QStringLiteral("Ubuntu-24.04")).toString().trimmed();
    const QString executable =
        settings.value(QStringLiteral("simulation/executable"), QStringLiteral("/usr/bin/flow")).toString();
    const int threads = settings.value(QStringLiteral("simulation/threads"), 2).toInt();
    const QVariantMap info = m_case->runInfo();
    const QString directory = info.value(QStringLiteral("runDirectory")).toString();
    const QString dataFile = QDir(directory).absoluteFilePath(info.value(QStringLiteral("dataFile")).toString());
    if (directory.isEmpty() || !QFileInfo(dataFile).isFile() || linuxPath(directory).isEmpty() ||
        distribution.isEmpty() || !executable.startsWith(QLatin1Char('/')))
    {
        setStatus(QStringLiteral("rejected"),
                  tr("Configure a Linux distribution and absolute Flow executable path in Run Settings first."));
        return;
    }
    const QStringList inputs = info.value(QStringLiteral("files")).toStringList();
    for (const QString& input : inputs)
    {
        if (!QFileInfo(QDir(directory).absoluteFilePath(input)).isFile())
        {
            setStatus(QStringLiteral("rejected"), tr("A prepared run input file is missing: %1").arg(input));
            return;
        }
    }
    m_gridId.clear();
    for (const IDOSCaseItemRef& reference : m_case->itemRefs())
    {
        if (reference.role() == QStringLiteral("case.grid") &&
            qobject_cast<IDOSGridObject*>(m_project->object(reference.objectId())) != nullptr)
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
    m_currentRunId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_outputDirectory = QDir(directory).absoluteFilePath(QStringLiteral("results-") + m_currentRunId);
    if (!QDir().mkpath(m_outputDirectory))
    {
        setStatus(QStringLiteral("rejected"), tr("Cannot create the simulation output directory."));
        return;
    }
    m_outputEdit->clear();
    m_pendingOutput.clear();
    QVariantMap runInfo = m_case->runInfo();
    QVariantMap runRecord;
    runRecord.insert(QStringLiteral("runId"), m_currentRunId);
    runRecord.insert(QStringLiteral("startedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    runRecord.insert(QStringLiteral("status"), QStringLiteral("running"));
    runRecord.insert(QStringLiteral("runDirectory"), directory);
    runRecord.insert(QStringLiteral("outputDirectory"), m_outputDirectory);
    runRecord.insert(QStringLiteral("distribution"), distribution);
    runRecord.insert(QStringLiteral("executable"), executable);
    runRecord.insert(QStringLiteral("threads"), threads);
    QVariantList history = runInfo.value(QStringLiteral("history")).toList();
    history.append(runRecord);
    runInfo.insert(QStringLiteral("history"), history);
    runInfo.insert(QStringLiteral("outputDirectory"), m_outputDirectory);
    m_case->setRunInfo(runInfo);
    refreshRunHistory();
    m_cancelled = false;
    m_stopAttempts = 0;
    emit runningChanged();
    QStringList arguments;
    arguments << QStringLiteral("-d") << distribution << QStringLiteral("--cd")
              << linuxPath(directory) << QStringLiteral("--exec") << QStringLiteral("/usr/bin/setsid")
              << QStringLiteral("--wait") << QStringLiteral("/bin/sh") << QStringLiteral("-c")
              << QStringLiteral("echo $$ > \"$1\"; shift; exec \"$@\"") << QStringLiteral("idos-flow")
              << linuxPath(QDir(m_outputDirectory).absoluteFilePath(QStringLiteral("flow.pid")))
              << executable << linuxPath(dataFile)
              << QStringLiteral("--output-dir=") + linuxPath(m_outputDirectory)
              << QStringLiteral("--threads-per-process=%1").arg(threads);
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
    setStatus(QStringLiteral("stopping"), tr("Stopping Flow..."));
    m_stopTimer->start(1000);
    onStopTimer();
    emit runningChanged();
}

void IDOSSimulationRunStatusWidget::onProjectDestroyed()
{
    cancel();
}

void IDOSSimulationRunStatusWidget::cancel()
{
    m_invalidated = true;
    m_cancelled = true;
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
    QSettings settings;
    const QString distribution =
        settings.value(QStringLiteral("simulation/distribution"), QStringLiteral("Ubuntu-24.04")).toString().trimmed();
    arguments << QStringLiteral("-d") << distribution << QStringLiteral("--exec")
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
        emit runFinished(false);
    }
    else if (exitStatus != QProcess::NormalExit || exitCode != 0)
    {
        setStatus(QStringLiteral("failed"),
                  tr("Flow failed (exit code %1): %2").arg(exitCode).arg(m_process->errorString()));
        emit runFinished(false);
    }
    else
    {
        emit runFinished(importResults());
    }
}

bool IDOSSimulationRunStatusWidget::importResults()
{
    if (m_project == nullptr || m_case == nullptr || m_project->object(m_caseId) != m_case.data() ||
        qobject_cast<IDOSGridObject*>(m_project->object(m_gridId)) == nullptr)
    {
        setStatus(QStringLiteral("importFailed"),
                  tr("The project or target grid is unavailable. Results were not imported."));
        return false;
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
        return false;
    }
    const QStringList restartFiles =
        QDir(m_outputDirectory)
            .entryList(QStringList() << QStringLiteral("*.UNRST") << QStringLiteral("*.unrst"), QDir::Files);
    if (restartFiles.size() != 1)
    {
        setStatus(QStringLiteral("importFailed"), tr("Flow finished, but exactly one UNRST result file is required."));
        return false;
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
        return false;
    }
    QVariantMap runInfo = m_case->runInfo();
    QVariantList history = runInfo.value(QStringLiteral("history")).toList();
    for (int historyIndex = history.size() - 1; historyIndex >= 0; --historyIndex)
    {
        QVariantMap record = history.at(historyIndex).toMap();
        if (record.value(QStringLiteral("runId")).toString() == m_currentRunId)
        {
            record.insert(QStringLiteral("resultFile"), QDir(m_outputDirectory).absoluteFilePath(restartFiles.first()));
            history[historyIndex] = record;
            break;
        }
    }
    runInfo.insert(QStringLiteral("history"), history);
    m_case->setRunInfo(runInfo);
    setStatus(QStringLiteral("succeeded"),
              tr("Flow completed and dynamic properties were imported. You can undo the import."));
    return true;
}

void IDOSSimulationRunStatusWidget::loadStatus(IDOSSimulationCaseObject* simulationCase)
{
    if (simulationCase == nullptr)
    {
        m_statusLabel->setText(tr("No simulation case is selected."));
        return;
    }

    const QVariantMap runInfo = simulationCase->runInfo();
    if (runInfo.value(QStringLiteral("status")).toString() == QStringLiteral("succeeded"))
    {
        m_statusLabel->setText(runInfo.value(QStringLiteral("message")).toString());
        return;
    }
    const QString message = runInfo.value(QStringLiteral("message")).toString();
    if (!message.isEmpty())
    {
        m_statusLabel->setText(message);
        return;
    }
    m_statusLabel->setText(tr("Select a case and click Run to check inputs and start Flow."));
}

void IDOSSimulationRunStatusWidget::onHistoryRowDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    QTableWidgetItem* item = m_historyTable->item(row, 2);
    if (item == nullptr)
    {
        return;
    }
    const QString outputDirectory = item->data(Qt::UserRole).toString();
    if (!outputDirectory.isEmpty() && QFileInfo(outputDirectory).isDir())
    {
        emit historyEntryActivated(outputDirectory);
    }
}
