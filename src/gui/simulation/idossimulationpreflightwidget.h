#ifndef IDOS_SIMULATION_PREFLIGHT_WIDGET_H
#define IDOS_SIMULATION_PREFLIGHT_WIDGET_H

#include <QString>
#include <QVariantMap>
#include <QWidget>

#include "idos_app.h"

class IDOSSimulationCaseObject;
class QLabel;
class QTableWidget;

class APP_EXPORT IDOSSimulationPreflightWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationPreflightWidget(IDOSSimulationCaseObject* simulationCase,
                                           QWidget* parent = nullptr);
    bool hasBlockingIssues() const;

  private:
    void addResult(const QString& checkName, const QString& status, const QString& details);
    void runChecks(IDOSSimulationCaseObject* simulationCase);

    QLabel* m_summaryLabel;
    QTableWidget* m_resultsTable;
    int m_blockingCount;
    int m_warningCount;
};

#endif // IDOS_SIMULATION_PREFLIGHT_WIDGET_H
