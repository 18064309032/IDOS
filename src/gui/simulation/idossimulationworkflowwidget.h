#ifndef IDOS_SIMULATION_WORKFLOW_WIDGET_H
#define IDOS_SIMULATION_WORKFLOW_WIDGET_H

#include <QString>
#include <QWidget>

#include "idos_app.h"

class IDOSSimulationCaseObject;
class QTableWidget;

class APP_EXPORT IDOSSimulationWorkflowWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationWorkflowWidget(IDOSSimulationCaseObject* simulationCase,
                                          QWidget* parent = nullptr);

  private:
    void addStep(const QString& name, const QString& status, const QString& details);

    QTableWidget* m_stepsTable;
};

#endif // IDOS_SIMULATION_WORKFLOW_WIDGET_H
