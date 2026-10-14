#ifndef IDOS_SIMULATION_CURVE_COMPARISON_WIDGET_H
#define IDOS_SIMULATION_CURVE_COMPARISON_WIDGET_H

#include <QVariantList>
#include <QWidget>

#include "idos_app.h"

class IDOSSimulationCaseObject;
class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;

class APP_EXPORT IDOSSimulationCurveComparisonWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationCurveComparisonWidget(IDOSSimulationCaseObject* simulationCase,
                                                 QWidget* parent = nullptr);

  private slots:
    void onKeywordSelectionChanged(int index);
    void onCurveSelectionChanged(int index);
    void onTableItemChanged(QListWidgetItem* item);

  private:
    void updateTableSelection();
    void renderPlot();

    QVariantList m_tables;
    QComboBox* m_keywordSelector;
    QComboBox* m_curveSelector;
    QListWidget* m_tableList;
    QLabel* m_plotLabel;
};

#endif // IDOS_SIMULATION_CURVE_COMPARISON_WIDGET_H
