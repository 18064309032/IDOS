#include <exception>
#include <memory>

#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QThread>

#include "idosdataobject.h"

#include "idosdataprovider.h"

QString IDOSDataProvider::lastError() const
{
    return m_lastError;
}

QStringList IDOSDataProvider::splitTextFields(const QString& line, bool& valid) const
{
    QStringList fields;
    QString token;
    bool quoted = false;
    bool started = false;
    for (int index = 0; index < line.size(); ++index)
    {
        const QChar character = line.at(index);
        if (character == QLatin1Char('"'))
        {
            if (quoted && index + 1 < line.size() && line.at(index + 1) == QLatin1Char('"'))
            {
                token.append(character);
                ++index;
            }
            else
            {
                quoted = !quoted;
            }
            started = true;
        }
        else if (!quoted && character.isSpace())
        {
            if (started)
            {
                fields.append(token);
                token.clear();
                started = false;
            }
        }
        else
        {
            token.append(character);
            started = true;
        }
    }
    if (started)
    {
        fields.append(token);
    }
    valid = !quoted;
    return fields;
}

void IDOSDataProvider::setLastError(const QString& msg)
{
    m_lastError = msg;
}

IDOSDataProvider::~IDOSDataProvider()
{
}

QList<IDOSDataObject*> IDOSDataProvider::readInBackground(const QString& filePath)
{
    QList<IDOSDataObject*> objects;
    QThread* targetThread = QThread::currentThread();
    std::unique_ptr<QThread> worker(QThread::create(&IDOSDataProvider::readWorker,
                                                   this, filePath, targetThread, &objects));
    QEventLoop loop;
    QObject::connect(worker.get(), &QThread::finished, &loop, &QEventLoop::quit);
    worker->start();
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    worker->wait();
    return objects;
}

void IDOSDataProvider::readWorker(IDOSDataProvider* provider, const QString& filePath,
                                  QThread* targetThread, QList<IDOSDataObject*>* objects)
{
    QElapsedTimer timer;
    timer.start();
    try
    {
        *objects = provider->read(filePath);
    }
    catch (const std::exception& error)
    {
        provider->setLastError(QObject::tr("Data reading failed: %1").arg(QString::fromUtf8(error.what())));
    }
    catch (...)
    {
        provider->setLastError(QObject::tr("Data reading failed with an unknown error."));
    }
    for (IDOSDataObject* object : *objects)
    {
        if (object != nullptr)
        {
            object->moveToThread(targetThread);
        }
    }
    provider->reportReadPhase(QStringLiteral("read-total"), timer.nsecsElapsed());
}

void IDOSDataProvider::reportReadPhase(const QString& phase, qint64 nanoseconds) const
{
    if (qEnvironmentVariableIsSet("IDOS_IMPORT_PROFILE"))
    {
        qInfo("IMPORT_PROFILE phase=%s elapsed_ms=%.3f", qPrintable(phase), nanoseconds / 1000000.0);
    }
}
