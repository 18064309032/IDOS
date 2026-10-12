#ifndef IDOS_SIMULATION_WORKFLOW_WIDGET_H
#define IDOS_SIMULATION_WORKFLOW_WIDGET_H

#include <QString>
#include <QWidget>

class IDOSSimulationCaseObject;
class QTableWidget;

class IDOSSimulationWorkflowWidget : public QWidget
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
