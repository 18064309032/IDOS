#ifndef IDOS_SIMULATION_INITIALIZE_WIDGET_H
#define IDOS_SIMULATION_INITIALIZE_WIDGET_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "idos_app.h"
#include "logger/idosloglevel.h"

class IDOSSimulationCaseObject;

class APP_EXPORT IDOSSimulationInitializeWidget : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSSimulationInitializeWidget(IDOSSimulationCaseObject* simulationCase,
                                            QObject* parent = nullptr);
    void setRunDirectory(const QString& directory);
    void setGeneratedDataFile(const QString& filePath);
    bool prepareRunDirectory();
    QVariantMap preparedRunInfo() const;

  private:
    void showOutput(const QString& message, IDOSLogLevel level = IDOSLogLevel::Error);
    QStringList inputFiles(IDOSSimulationCaseObject* simulationCase) const;
    QString relativeInputPath(const QString& inputFile) const;
    bool validateInputs(QStringList* messages) const;

    QString m_sourceFile;
    QStringList m_inputFiles;
    QPointer<IDOSSimulationCaseObject> m_simulationCase;
    QString m_runDirectory;
    QVariantMap m_preparedRunInfo;
};

#endif // IDOS_SIMULATION_INITIALIZE_WIDGET_H
