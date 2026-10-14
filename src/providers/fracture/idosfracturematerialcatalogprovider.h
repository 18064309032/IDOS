#ifndef IDOS_FRACTURE_MATERIAL_CATALOG_PROVIDER_H
#define IDOS_FRACTURE_MATERIAL_CATALOG_PROVIDER_H

#include "idosdataprovider.h"

class PROVIDERS_EXPORT IDOSFractureMaterialCatalogProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;
};

#endif // IDOS_FRACTURE_MATERIAL_CATALOG_PROVIDER_H
