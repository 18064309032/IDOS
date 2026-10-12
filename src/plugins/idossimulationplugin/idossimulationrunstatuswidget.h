#ifndef IDOS_SIMULATION_RUN_STATUS_WIDGET_H
#define IDOS_SIMULATION_RUN_STATUS_WIDGET_H

#include <QByteArray>
#include <QPointer>
#include <QProcess>
#include <QString>
#include <QWidget>

class IDOSProject;
class IDOSSimulationCaseObject;
class QCloseEvent;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTimer;

class IDOSSimulationRunStatusWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationRunStatusWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent = nullptr);
    ~IDOSSimulationRunStatusWidget() override;
    bool isRunning() const;
    void stop();
    void cancel();

  protected:
    void closeEvent(QCloseEvent* event) override;

  private slots:
    void onStartTriggered();
    void onStopTriggered();
    void onProjectDestroyed();
    void onOutputReady();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onStopTimer();
    void onObjectRemoving(const QString& objectId);

  private:
    void loadStatus(IDOSSimulationCaseObject* simulationCase);
    QString linuxPath(const QString& path) const;
    void setStatus(const QString& status, const QString& message);
    void importResults();
    void forwardOutput(bool flush);

    QPointer<IDOSSimulationCaseObject> m_case;
    QPointer<IDOSProject> m_project;
    QString m_caseId;
    QString m_gridId;
    QString m_outputDirectory;
    QByteArray m_pendingOutput;
    bool m_cancelled;
    bool m_invalidated;
    int m_stopAttempts;
    QProcess* m_process;
    QProcess* m_stopProcess;
    QTimer* m_stopTimer;
    QLineEdit* m_distributionEdit;
    QLineEdit* m_executableEdit;
    QSpinBox* m_threadsSpin;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPlainTextEdit* m_outputEdit;
    QLabel* m_statusLabel;
    QLabel* m_directoryLabel;
    QLabel* m_initializedAtLabel;
    QTableWidget* m_filesTable;
};

#endif // IDOS_SIMULATION_RUN_STATUS_WIDGET_H
