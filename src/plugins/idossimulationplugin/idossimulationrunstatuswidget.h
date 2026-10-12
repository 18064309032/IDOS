#ifndef IDOS_SIMULATION_RUN_STATUS_WIDGET_H
#define IDOS_SIMULATION_RUN_STATUS_WIDGET_H

#include <QWidget>

class IDOSSimulationCaseObject;
class QLabel;
class QTableWidget;

class IDOSSimulationRunStatusWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationRunStatusWidget(IDOSSimulationCaseObject* simulationCase,
                                           QWidget* parent = nullptr);

  private:
    void loadStatus(IDOSSimulationCaseObject* simulationCase);

    QLabel* m_statusLabel;
    QLabel* m_directoryLabel;
    QLabel* m_initializedAtLabel;
    QTableWidget* m_filesTable;
};

#endif // IDOS_SIMULATION_RUN_STATUS_WIDGET_H
