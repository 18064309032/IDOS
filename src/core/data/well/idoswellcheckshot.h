#ifndef IDOS_WELL_CHECKSHOT_H
#define IDOS_WELL_CHECKSHOT_H

#include <QString>
#include <QVector>

#include "idos_core.h"

class CORE_EXPORT IDOSWellCheckshot
{
  public:
    IDOSWellCheckshot();
    bool setSamples(const QVector<double>& depths, const QVector<double>& times);
    const QVector<double>& depths() const;
    const QVector<double>& times() const;
    QString depthUnit() const;
    QString timeUnit() const;
    QString timeType() const;
    QString depthType() const;
    void setUnits(const QString& depthUnit, const QString& timeUnit);
    void setTimeType(const QString& timeType);
    void setDepthType(const QString& depthType);

  private:
    QVector<double> m_depths;
    QVector<double> m_times;
    QString m_depthUnit;
    QString m_timeUnit;
    QString m_timeType;
    QString m_depthType;
};

#endif
