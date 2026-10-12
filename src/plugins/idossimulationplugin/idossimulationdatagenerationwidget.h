#ifndef IDOS_SIMULATION_DATA_GENERATION_WIDGET_H
#define IDOS_SIMULATION_DATA_GENERATION_WIDGET_H

#include <QStringList>
#include <QWidget>

class IDOSSimulationCaseObject;
class QLineEdit;
class QPlainTextEdit;

class IDOSSimulationDataGenerationWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationDataGenerationWidget(IDOSSimulationCaseObject* simulationCase,
                                                QWidget* parent = nullptr);

  private slots:
    void onBrowseParentDirectoryTriggered();
    void onGeneratePackageTriggered();

  private:
    QString generatedMainFileName() const;
    bool writePackage(const QString& parentDirectory, QString* packagePath, QString* errorMessage) const;

    QString m_caseName;
    QString m_sourceFile;
    QStringList m_sourceFiles;
    QLineEdit* m_parentDirectoryEdit;
    QPlainTextEdit* m_dataEditor;
    QPlainTextEdit* m_outputEdit;
};

#endif // IDOS_SIMULATION_DATA_GENERATION_WIDGET_H
