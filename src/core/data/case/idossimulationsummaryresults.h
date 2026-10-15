#ifndef IDOS_SIMULATION_SUMMARY_RESULTS_H
#define IDOS_SIMULATION_SUMMARY_RESULTS_H


#include <QList>
#include <QString>
#include <QVariantMap>
#include <QVector>

#include "idosdataobject.h"


class CORE_EXPORT IDOSSimulationSummaryResults : public IDOSDataObject
{
    Q_OBJECT

  public:
    explicit IDOSSimulationSummaryResults(QObject* parent = nullptr);

    QString typeId() const override;
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    QVector<qint64> timeValues() const;
    void setTimeValues(const QVector<qint64>& timeValues);
    QList<QVariantMap> series() const;
    void setSeries(const QList<QVariantMap>& series);

  private:
    QString m_sourceFile;
    QVector<qint64> m_timeValues;
    QList<QVariantMap> m_series;
};

#endif // IDOS_SIMULATION_SUMMARY_RESULTS_H
