#ifndef IDOS_WELL_PATH_METADATA_H
#define IDOS_WELL_PATH_METADATA_H

#include "idos_providers.h"
#include "providers/idosdataprovidermetadata.h"

class PROVIDERS_EXPORT IDOSWellPathMetadata : public IDOSDataProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    QStringList fileExtensions() const override;
    bool canHandle(const QString& filePath) const override;
    std::unique_ptr<IDOSDataProvider> create() const override;
};
#endif
