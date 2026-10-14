#ifndef IDOS_GRID_BOUNDARY_PROVIDER_H
#define IDOS_GRID_BOUNDARY_PROVIDER_H

#include "idosdataprovider.h"

class PROVIDERS_EXPORT IDOSGridBoundaryProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    QString readCoordinateReferenceSystem(const QString& filePath) const;
};

#endif // IDOS_GRID_BOUNDARY_PROVIDER_H
