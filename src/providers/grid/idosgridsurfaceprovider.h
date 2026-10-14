#ifndef IDOS_GRID_SURFACE_PROVIDER_H
#define IDOS_GRID_SURFACE_PROVIDER_H

#include "idos_providers.h"
#include "providers/idosdataprovider.h"

class PROVIDERS_EXPORT IDOSGridSurfaceProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    QString readCoordinateReferenceSystem(const QString& filePath) const;
};

#endif // IDOS_GRID_SURFACE_PROVIDER_H
