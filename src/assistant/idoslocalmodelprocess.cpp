#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "idoslocalmodelprocess.h"

IDOSLocalModelProcess::IDOSLocalModelProcess(QObject* parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_baseUrl(QStringLiteral("http://127.0.0.1:18080"))
    , m_lastError()
{
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
    m_process->setWorkingDirectory(workingDirectory);
    m_process->start(executablePath, arguments);
    if (!m_process->waitForStarted(5000))
    {
        error = m_process->errorString();
        setError(error);
        return false;
    }

    return true;
}

void IDOSLocalModelProcess::stop()
{
    if (!isRunning())
    {
        return;
    }

    m_process->terminate();
    if (!m_process->waitForFinished(3000))
    {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

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
