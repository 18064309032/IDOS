#ifndef IDOS_LOCAL_MODEL_PROCESS_H
#define IDOS_LOCAL_MODEL_PROCESS_H

#include <QProcess>
#include <QString>

#include "idos_assistant.h"

class ASSISTANT_EXPORT IDOSLocalModelProcess : public QObject
{
    Q_OBJECT

public:
    explicit IDOSLocalModelProcess(QObject* parent = nullptr);
    ~IDOSLocalModelProcess() override;

    IDOSLocalModelProcess(const IDOSLocalModelProcess&) = delete;
    IDOSLocalModelProcess& operator=(const IDOSLocalModelProcess&) = delete;

    bool start(const QString& assistantRoot, QString& error);
    void stop();
    bool isRunning() const;
    QString baseUrl() const;
    QString lastError() const;

signals:
    void started();
    void stopped();
    void errorOccurred(const QString& error);

private slots:
    void onProcessStateChanged(QProcess::ProcessState state);
    void onProcessError(QProcess::ProcessError error);

private:
    bool loadConfiguration(const QString& assistantRoot,
                           QString& executablePath,
                           QStringList& arguments,
                           QString& workingDirectory,
                           QString& error) const;
    void setError(const QString& error);

    QProcess* m_process;
    QString m_baseUrl;
    QString m_lastError;
};

#endif // IDOS_LOCAL_MODEL_PROCESS_H
