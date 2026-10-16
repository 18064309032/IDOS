#ifndef IDOS_MICROSEISMIC_PROVIDER_METADATA_H
#define IDOS_MICROSEISMIC_PROVIDER_METADATA_H

#include "idos_providers.h"
#include "providers/idosdataprovidermetadata.h"

class PROVIDERS_EXPORT IDOSMicroseismicMetadata : public IDOSDataProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    QStringList fileExtensions() const override;
    std::unique_ptr<IDOSDataProvider> createProvider() const override;
    bool canHandle(const QString& filePath) const override;
};

#endif // IDOS_MICROSEISMIC_PROVIDER_METADATA_H
