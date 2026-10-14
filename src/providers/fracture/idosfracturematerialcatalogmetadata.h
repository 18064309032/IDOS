#ifndef IDOS_FRACTURE_MATERIAL_CATALOG_PROVIDER_METADATA_H
#define IDOS_FRACTURE_MATERIAL_CATALOG_PROVIDER_METADATA_H

#include "idosdataprovidermetadata.h"

class PROVIDERS_EXPORT IDOSFractureMaterialCatalogMetadata : public IDOSDataProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    QStringList fileExtensions() const override;
    std::unique_ptr<IDOSDataProvider> create() const override;
    bool canHandle(const QString& filePath) const override;
};

#endif // IDOS_FRACTURE_MATERIAL_CATALOG_PROVIDER_METADATA_H
