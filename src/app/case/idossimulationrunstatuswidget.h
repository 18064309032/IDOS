#ifndef IDOS_SIMULATION_RUN_STATUS_WIDGET_H
#define IDOS_SIMULATION_RUN_STATUS_WIDGET_H

#include <QByteArray>
#include <QPointer>
#include <QProcess>
#include <QString>
#include <QWidget>

#include "idos_app.h"

class IDOSProject;
class IDOSSimulationCaseObject;
class QLabel;
class QPlainTextEdit;
class QTableWidget;
class QTimer;

class APP_EXPORT IDOSSimulationRunStatusWidget : public QWidget
{
    Q_OBJECT

  signals:
    void runningChanged();
    void runFinished(bool succeeded);
    void historyEntryActivated(const QString& outputDirectory);

  public:
    explicit IDOSSimulationRunStatusWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent = nullptr);
    ~IDOSSimulationRunStatusWidget() override;
    bool isRunning() const;
    bool isForCase(const QString& caseId) const;
    void stop();
    void cancel();
    void startFlow();
    void refreshSettingsSummary();
    void refreshRunHistory();
    void reportRunRequestFailure(const QString& message);

  private slots:
    void onStartTriggered();
    void onProjectDestroyed();
    void onOutputReady();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onStopTimer();
    void onObjectRemoving(const QString& objectId);
    void onHistoryRowDoubleClicked(int row, int column);

  private:
    void loadStatus(IDOSSimulationCaseObject* simulationCase);
    QString linuxPath(const QString& path) const;
    void setStatus(const QString& status, const QString& message);
    void updateSettingsSummary();
    bool importResults();
    void forwardOutput(bool flush);

    QPointer<IDOSSimulationCaseObject> m_case;
    QPointer<IDOSProject> m_project;
    QString m_caseId;
    QString m_gridId;
    QString m_outputDirectory;
    QString m_currentRunId;
    QByteArray m_pendingOutput;
    bool m_cancelled;
    bool m_invalidated;
    int m_stopAttempts;
    QProcess* m_process;
    QProcess* m_stopProcess;
    QTimer* m_stopTimer;
    QLabel* m_settingsSummaryLabel;
    QPlainTextEdit* m_outputEdit;
    QLabel* m_statusLabel;
    QTableWidget* m_historyTable;
};

#endif // IDOS_SIMULATION_RUN_STATUS_WIDGET_H
