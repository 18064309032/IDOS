#ifndef IDOS_SIMULATION_CASE_TABLE_WIDGET_H
#define IDOS_SIMULATION_CASE_TABLE_WIDGET_H

#include <QString>
#include <QWidget>

#include "idos_app.h"

class IDOSSimulationCaseObject;
class QLineEdit;
class QTableWidget;

class APP_EXPORT IDOSSimulationCaseTableWidget : public QWidget
{
    Q_OBJECT

  public:
    IDOSSimulationCaseTableWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent = nullptr);
    ~IDOSSimulationCaseTableWidget() override;

  private slots:
    void onFilterTextChanged(const QString& text);

  private:
    void addRow(const QString& category, const QString& item, const QString& value);

    QLineEdit* m_filter;
    QTableWidget* m_table;
};

#endif // IDOS_SIMULATION_CASE_TABLE_WIDGET_H
