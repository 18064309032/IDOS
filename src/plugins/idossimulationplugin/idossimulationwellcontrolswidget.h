#ifndef IDOS_SIMULATION_WELL_CONTROLS_WIDGET_H
#define IDOS_SIMULATION_WELL_CONTROLS_WIDGET_H

#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

class IDOSSimulationCaseObject;
class QComboBox;
class QTableWidget;

class IDOSSimulationWellControlsWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationWellControlsWidget(IDOSSimulationCaseObject* simulationCase,
                                              QWidget* parent = nullptr);

  private slots:
    void onFiltersChanged();

  private:
    QString stepLabel(const QVariantMap& row) const;
    void populateFilters();
    void populateTable();

    QVariantList m_rows;
    QComboBox* m_wellFilter;
    QComboBox* m_stepFilter;
    QTableWidget* m_table;
};

#endif // IDOS_SIMULATION_WELL_CONTROLS_WIDGET_H
