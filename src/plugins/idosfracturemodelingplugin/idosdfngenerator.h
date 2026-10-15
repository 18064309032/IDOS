#ifndef IDOS_DFN_GENERATOR_H
#define IDOS_DFN_GENERATOR_H

#include <QJsonObject>
#include <QString>
#include <QVariantList>
#include <QVector>

class IDOSGrid;
class IDOSGridProperty;

class IDOSDfnGenerator
{
  public:
    static bool validate(const IDOSGrid* grid, const IDOSGridProperty* density,
                         const QJsonObject& parameters, QString& error);
    static bool generate(const IDOSGrid* grid, const IDOSGridProperty* density,
                         const QJsonObject& parameters, QVariantList& fractures, QString& error);

  private:
    static bool cellCoordinates(const IDOSGrid* grid, int cell, QVector<double>& corners);
};

#endif // IDOS_DFN_GENERATOR_H
