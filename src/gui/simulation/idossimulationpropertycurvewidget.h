#ifndef IDOS_SIMULATION_PROPERTY_CURVE_WIDGET_H
#define IDOS_SIMULATION_PROPERTY_CURVE_WIDGET_H

#include <QString>
#include <QVariantList>
#include <QWidget>

#include "idos_app.h"

class IDOSSimulationCaseObject;
class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;

class APP_EXPORT IDOSSimulationPropertyCurveWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationPropertyCurveWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent = nullptr);

  private slots:
    void onTableSelectionChanged(int index);
    void onSeriesItemChanged(QListWidgetItem* item);

  private:
    void updateSeries();
    void renderPlot();

    QVariantList m_tables;
    QComboBox* m_tableSelector;
    QListWidget* m_seriesList;
    QLabel* m_plotLabel;
};

#endif // IDOS_SIMULATION_PROPERTY_CURVE_WIDGET_H
