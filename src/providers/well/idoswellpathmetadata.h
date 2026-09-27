#ifndef IDOS_WELL_PATH_METADATA_H
#define IDOS_WELL_PATH_METADATA_H
#include "idosprovidermetadata.h"
class PROVIDERS_EXPORT IDOSWellPathMetadata : public IDOSProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    QStringList fileExtensions() const override;
    std::unique_ptr<IDOSDataProvider> createProvider() const override;
};
#endif
