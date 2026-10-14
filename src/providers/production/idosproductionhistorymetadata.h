#ifndef IDOS_PRODUCTION_HISTORY_PROVIDER_METADATA_H
#define IDOS_PRODUCTION_HISTORY_PROVIDER_METADATA_H

#include "idosdataprovidermetadata.h"

class PROVIDERS_EXPORT IDOSProductionHistoryMetadata : public IDOSDataProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    QStringList fileExtensions() const override;
    std::unique_ptr<IDOSDataProvider> create() const override;
    bool canHandle(const QString& filePath) const override;
};

#endif // IDOS_PRODUCTION_HISTORY_PROVIDER_METADATA_H
