#ifndef IDOS_SIMULATION_CASE_INVENTORY_REPORT_WIDGET_H
#define IDOS_SIMULATION_CASE_INVENTORY_REPORT_WIDGET_H

#include <QString>
#include <QWidget>

class IDOSSimulationCaseObject;
class QTextBrowser;

class IDOSSimulationCaseInventoryReportWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationCaseInventoryReportWidget(IDOSSimulationCaseObject* simulationCase,
                                                     QWidget* parent = nullptr);

  private slots:
    void onSaveReportTriggered();

  private:
    QString buildReportHtml(IDOSSimulationCaseObject* simulationCase) const;

    QString m_reportHtml;
    QTextBrowser* m_reportBrowser;
};

#endif // IDOS_SIMULATION_CASE_INVENTORY_REPORT_WIDGET_H
