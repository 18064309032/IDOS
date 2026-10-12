#ifndef IDOS_SIMULATION_INITIALIZE_WIDGET_H
#define IDOS_SIMULATION_INITIALIZE_WIDGET_H

#include <QPointer>
#include <QStringList>
#include <QWidget>

#include "log/idosloglevel.h"

class IDOSSimulationCaseObject;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QTableWidget;

class IDOSSimulationInitializeWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationInitializeWidget(IDOSSimulationCaseObject* simulationCase,
                                            QWidget* parent = nullptr);

  private slots:
    void onBrowseDirectoryTriggered();
    void onInitializeTriggered();

  private:
    void showOutput(const QString& message, IDOSLogLevel level = IDOSLogLevel::Error);
    QStringList inputFiles(IDOSSimulationCaseObject* simulationCase) const;
    QString relativeInputPath(const QString& inputFile) const;
    bool validateInputs(QStringList* messages) const;
    void updateInputTable();

    QString m_sourceFile;
    QStringList m_inputFiles;
    QPointer<IDOSSimulationCaseObject> m_simulationCase;
    QLineEdit* m_directoryEdit;
    QTableWidget* m_inputTable;
    QPlainTextEdit* m_outputEdit;
    QPushButton* m_initializeButton;
};

#endif // IDOS_SIMULATION_INITIALIZE_WIDGET_H
