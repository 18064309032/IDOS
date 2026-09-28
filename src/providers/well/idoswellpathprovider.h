#ifndef IDOS_WELL_PATH_PROVIDER_H
#define IDOS_WELL_PATH_PROVIDER_H

#include "idosdataprovider.h"
#include "idoswellspatialreference.h"

class PROVIDERS_EXPORT IDOSWellPathProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    IDOSWellSpatialReference spatialReferenceFromComments(const QStringList& comments) const;
};

#endif // IDOS_WELL_PATH_PROVIDER_H
