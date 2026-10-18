#ifndef IDOS_SIMULATION_DATA_GENERATION_WIDGET_H
#define IDOS_SIMULATION_DATA_GENERATION_WIDGET_H

#include <QPointer>
#include <QWidget>

#include "idos_app.h"
#include "logger/idosloglevel.h"

class IDOSSimulationCaseObject;
class QLineEdit;
class QPlainTextEdit;

class APP_EXPORT IDOSSimulationDataGenerationWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationDataGenerationWidget(IDOSSimulationCaseObject* simulationCase,
                                                QWidget* parent = nullptr);

  private slots:
    void onBrowseParentDirectoryTriggered();
    void onGeneratePackageTriggered();

  private:
    void showOutput(const QString& message, IDOSLogLevel level = IDOSLogLevel::Error);
    QString generatedMainFileName() const;
    bool writePackage(const QString& parentDirectory, QString* packagePath, QString* errorMessage) const;

    QString m_caseName;
    QString m_sourceFile;
    QPointer<IDOSSimulationCaseObject> m_simulationCase;
    QLineEdit* m_parentDirectoryEdit;
    QPlainTextEdit* m_dataEditor;
    QPlainTextEdit* m_outputEdit;
};

#endif // IDOS_SIMULATION_DATA_GENERATION_WIDGET_H
