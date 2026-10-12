#ifndef IDOS_SIMULATION_PARSE_SUMMARY_WIDGET_H
#define IDOS_SIMULATION_PARSE_SUMMARY_WIDGET_H

#include <QString>
#include <QVariantMap>
#include <QWidget>

class IDOSSimulationCaseObject;

class IDOSSimulationParseSummaryWidget : public QWidget
{
    Q_OBJECT

  public:
    IDOSSimulationParseSummaryWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent = nullptr);
    ~IDOSSimulationParseSummaryWidget() override;
};

#endif // IDOS_SIMULATION_PARSE_SUMMARY_WIDGET_H
