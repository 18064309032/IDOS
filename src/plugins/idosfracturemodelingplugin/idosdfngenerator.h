#ifndef IDOS_DFN_GENERATOR_H
#define IDOS_DFN_GENERATOR_H

#include <QJsonObject>
#include <QString>
#include <QVariantList>
#include <QVector>

class IDOSGridObject;
class IDOSGridPropertyObject;

class IDOSDfnGenerator
{
  public:
    static bool validate(const IDOSGridObject* grid, const IDOSGridPropertyObject* density,
                         const QJsonObject& parameters, QString& error);
    static bool generate(const IDOSGridObject* grid, const IDOSGridPropertyObject* density,
                         const QJsonObject& parameters, QVariantList& fractures, QString& error);

  private:
    static bool cellCoordinates(const IDOSGridObject* grid, int cell, QVector<double>& corners);
};

#endif // IDOS_DFN_GENERATOR_H
