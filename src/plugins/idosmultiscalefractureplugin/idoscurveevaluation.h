#ifndef IDOS_CURVE_EVALUATION_H
#define IDOS_CURVE_EVALUATION_H

#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QStringList>

class IDOSProject;

class IDOSCurveEvaluation
{
  public:
    static bool evaluate(IDOSProject* project, const QJsonObject& arguments, QJsonObject& result, QString& error);

  private:
    static bool readSource(IDOSProject* project, const QJsonObject& arguments, const QString& prefix,
                           QMap<double, double>& points, QString& error);
    static bool readCsv(const QJsonObject& arguments, const QString& prefix,
                        QMap<double, double>& points, QString& error);
    static bool splitRow(const QString& line, QStringList& fields);
    static bool insertPoint(QMap<double, double>& points, double time, double value, QString& error);
    static QString normalizedUnit(const QString& unit);
};

#endif // IDOS_CURVE_EVALUATION_H
