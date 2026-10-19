#include <functional>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "idoslocalmodelprocess.h"

IDOSLocalModelProcess::IDOSLocalModelProcess(QObject* parent)
    :
    QObject(parent)
#ifdef Q_OS_WIN
    , m_jobHandle(nullptr)
    , m_startupInfo()
#endif
    , m_process(new QProcess(this))
    , m_baseUrl(QStringLiteral("http://127.0.0.1:18080"))
    , m_lastError()
{
#ifdef Q_OS_WIN
    m_process->setCreateProcessArgumentsModifier(
        std::bind(&IDOSLocalModelProcess::modifyProcessArguments, this, std::placeholders::_1));
#endif
    connect(m_process,
            &QProcess::stateChanged,
            this,
            &IDOSLocalModelProcess::onProcessStateChanged);
    connect(m_process,
            &QProcess::errorOccurred,
            this,
            &IDOSLocalModelProcess::onProcessError);
}

IDOSLocalModelProcess::~IDOSLocalModelProcess()
{
    stop();
}

bool IDOSLocalModelProcess::start(const QString& assistantRoot, QString& error)
{
    if (isRunning())
    {
        return true;
    }

    QString executablePath;
    QStringList arguments;
    QString workingDirectory;
    if (!loadConfiguration(assistantRoot,
                           executablePath,
                           arguments,
                           workingDirectory,
                           error))
    {
        setError(error);
        return false;
    }

    m_lastError.clear();
#ifdef Q_OS_WIN
    if (!prepareProcessJob(error))
    {
        setError(error);
        return false;
    }
#endif
    m_process->setWorkingDirectory(workingDirectory);
    m_process->start(executablePath, arguments);
    if (!m_process->waitForStarted(5000))
    {
        error = m_process->errorString();
        setError(error);
#ifdef Q_OS_WIN
        releaseProcessJob();
#endif
        return false;
    }

    return true;
}

void IDOSLocalModelProcess::stop()
{
    if (!isRunning())
    {
#ifdef Q_OS_WIN
        releaseProcessJob();
#endif
        return;
    }

    m_process->terminate();
    if (!m_process->waitForFinished(3000))
    {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
#ifdef Q_OS_WIN
    releaseProcessJob();
#endif
}

#ifdef Q_OS_WIN
bool IDOSLocalModelProcess::prepareProcessJob(QString& error)
{
    releaseProcessJob();
    m_jobHandle = CreateJobObjectW(nullptr, nullptr);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (m_jobHandle == nullptr ||
        !SetInformationJobObject(m_jobHandle, JobObjectExtendedLimitInformation,
                                 &limits, sizeof(limits)))
    {
        const DWORD code = GetLastError();
        error = tr("Unable to configure assistant process cleanup (Windows error %1).").arg(code);
        releaseProcessJob();
        return false;
    }

    SIZE_T size = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    LPPROC_THREAD_ATTRIBUTE_LIST attributes = static_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(
        HeapAlloc(GetProcessHeap(), 0, size));
    if (attributes == nullptr)
    {
        error = tr("Unable to allocate assistant process startup attributes.");
        releaseProcessJob();
        return false;
    }
    if (!InitializeProcThreadAttributeList(attributes, 1, 0, &size))
    {
        const DWORD code = GetLastError();
        HeapFree(GetProcessHeap(), 0, attributes);
        error = tr("Unable to configure assistant process cleanup (Windows error %1).").arg(code);
        releaseProcessJob();
        return false;
    }
    m_startupInfo.lpAttributeList = attributes;
    if (!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_JOB_LIST,
                                   &m_jobHandle, sizeof(m_jobHandle), nullptr, nullptr))
    {
        const DWORD code = GetLastError();
        error = tr("Unable to configure assistant process cleanup (Windows error %1).").arg(code);
        releaseProcessJob();
        return false;
    }
    return true;
}

void IDOSLocalModelProcess::releaseProcessJob()
{
    if (m_startupInfo.lpAttributeList != nullptr)
    {
        DeleteProcThreadAttributeList(m_startupInfo.lpAttributeList);
        HeapFree(GetProcessHeap(), 0, m_startupInfo.lpAttributeList);
        m_startupInfo.lpAttributeList = nullptr;
    }
    if (m_jobHandle != nullptr)
    {
        CloseHandle(m_jobHandle);
        m_jobHandle = nullptr;
    }
}

void IDOSLocalModelProcess::modifyProcessArguments(QProcess::CreateProcessArguments* arguments)
{
    m_startupInfo.StartupInfo = *arguments->startupInfo;
    m_startupInfo.StartupInfo.cb = sizeof(m_startupInfo);
    arguments->startupInfo = &m_startupInfo.StartupInfo;
    arguments->flags |= EXTENDED_STARTUPINFO_PRESENT;
}
#endif

bool IDOSLocalModelProcess::isRunning() const
{
    return m_process->state() != QProcess::NotRunning;
}

QString IDOSLocalModelProcess::baseUrl() const
{
    return m_baseUrl;
}

QString IDOSLocalModelProcess::lastError() const
{
    return m_lastError;
}

void IDOSLocalModelProcess::onProcessStateChanged(QProcess::ProcessState state)
{
    if (state == QProcess::Running)
    {
        emit started();
    }
    else if (state == QProcess::NotRunning)
    {
        emit stopped();
    }
}

void IDOSLocalModelProcess::onProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error)
    setError(m_process->errorString());
    emit errorOccurred(m_lastError);
}

bool IDOSLocalModelProcess::loadConfiguration(const QString& assistantRoot,
                                              QString& executablePath,
                                              QStringList& arguments,
                                              QString& workingDirectory,
                                              QString& error) const
{
    const QString configurationPath = QDir(assistantRoot).filePath(QStringLiteral("local-ai.json"));
    QFile configurationFile(configurationPath);
    if (!configurationFile.open(QIODevice::ReadOnly))
    {
        error = QStringLiteral("Unable to open assistant configuration: %1").arg(configurationPath);
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(configurationFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        error = QStringLiteral("Invalid assistant configuration: %1").arg(parseError.errorString());
        return false;
    }

    const QJsonObject configuration = document.object();
    const QString runtime = configuration.value(QStringLiteral("runtime")).toString();
    const QString model = configuration.value(QStringLiteral("model")).toString();
    if (runtime.isEmpty() || model.isEmpty())
    {
        error = QStringLiteral("Assistant configuration must define runtime and model.");
        return false;
    }

    const QDir rootDirectory(assistantRoot);
    executablePath = rootDirectory.filePath(runtime);
    workingDirectory = QFileInfo(executablePath).absolutePath();
    arguments << QStringLiteral("-m")
              << rootDirectory.filePath(model)
              << QStringLiteral("--host")
              << QStringLiteral("127.0.0.1")
              << QStringLiteral("--port")
              << QStringLiteral("18080");

    const int contextSize = configuration.value(QStringLiteral("contextSize")).toInt();
    if (contextSize > 0)
    {
        arguments << QStringLiteral("-c") << QString::number(contextSize);
    }

    const int gpuLayers = configuration.value(QStringLiteral("gpuLayers")).toInt();
    if (gpuLayers >= 0)
    {
        arguments << QStringLiteral("-ngl") << QString::number(gpuLayers);
    }

    const QJsonArray additionalArguments = configuration.value(QStringLiteral("additionalArguments")).toArray();
    for (QJsonArray::const_iterator iterator = additionalArguments.constBegin();
         iterator != additionalArguments.constEnd();
         ++iterator)
    {
        arguments << iterator->toString();
    }

    if (!QFile::exists(executablePath))
    {
        error = QStringLiteral("Assistant runtime was not found: %1").arg(executablePath);
        return false;
    }

    if (!QFile::exists(rootDirectory.filePath(model)))
    {
        error = QStringLiteral("Assistant model was not found: %1").arg(rootDirectory.filePath(model));
        return false;
    }

    return true;
}

void IDOSLocalModelProcess::setError(const QString& error)
{
    m_lastError = error;
}
