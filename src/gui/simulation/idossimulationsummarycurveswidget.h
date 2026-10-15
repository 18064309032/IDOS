#ifndef IDOS_SIMULATION_SUMMARY_CURVES_WIDGET_H
#define IDOS_SIMULATION_SUMMARY_CURVES_WIDGET_H


#include <QList>
#include <QVariantMap>
#include <QVector>
#include <QWidget>

#include "idos_app.h"


class QComboBox;
class QLabel;

class IDOSSimulationSummaryResults;


class APP_EXPORT IDOSSimulationSummaryCurvesWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationSummaryCurvesWidget(IDOSSimulationSummaryResults* results,
                                               QWidget* parent = nullptr);

  private slots:
    void onSeriesSelectionChanged(int index);

  private:
    void renderPlot();

    QVector<qint64> m_timeValues;
    QList<QVariantMap> m_series;
    QComboBox* m_seriesSelector;
    QLabel* m_plotLabel;
};

#endif // IDOS_SIMULATION_SUMMARY_CURVES_WIDGET_H
