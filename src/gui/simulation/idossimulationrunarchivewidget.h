#ifndef IDOS_SIMULATION_RUN_ARCHIVE_WIDGET_H
#define IDOS_SIMULATION_RUN_ARCHIVE_WIDGET_H

#include <QVariantMap>
#include <QWidget>

#include "idos_app.h"
#include "logger/idosloglevel.h"

class IDOSSimulationCaseObject;
class QLineEdit;
class QPlainTextEdit;

class APP_EXPORT IDOSSimulationRunArchiveWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationRunArchiveWidget(IDOSSimulationCaseObject* simulationCase,
                                            QWidget* parent = nullptr);

  private slots:
    void onBrowseDirectoryTriggered();
    void onCreateArchiveTriggered();

  private:
    void showOutput(const QString& message, IDOSLogLevel level = IDOSLogLevel::Error);
    QString m_caseName;
    QVariantMap m_runInfo;
    QLineEdit* m_archiveDirectoryEdit;
    QPlainTextEdit* m_outputEdit;
};

#endif // IDOS_SIMULATION_RUN_ARCHIVE_WIDGET_H
