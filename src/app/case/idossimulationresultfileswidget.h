#ifndef IDOS_SIMULATION_RESULT_FILES_WIDGET_H
#define IDOS_SIMULATION_RESULT_FILES_WIDGET_H

#include <QPointer>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "idos_app.h"

class IDOSProject;
class IDOSSimulationCaseObject;
class QLabel;
class QPushButton;
class QTableWidget;

class APP_EXPORT IDOSSimulationResultFilesWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationResultFilesWidget(IDOSSimulationCaseObject* simulationCase,
                                             QWidget* parent = nullptr);
    void showRunDirectory(const QString& runDirectory);

  private slots:
    void onOpenRunDirectoryTriggered();
    void onResultFileDoubleClicked(int row, int column);
    void onImportDynamicPropertiesTriggered();
    void onSelectionChanged();

  private:
    void loadResultFiles(IDOSSimulationCaseObject* simulationCase);
    void populateResultFiles(const QStringList& excludedInputFiles);

    QString m_runDirectory;
    QStringList m_inputFiles;
    QString m_gridId;
    QPointer<IDOSProject> m_project;
    QLabel* m_summaryLabel;
    QTableWidget* m_filesTable;
    QPushButton* m_importButton;
};

#endif // IDOS_SIMULATION_RESULT_FILES_WIDGET_H
