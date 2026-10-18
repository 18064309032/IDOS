#ifndef IDOS_FRACTURE_SCHEDULE_PROVIDER_METADATA_H
#define IDOS_FRACTURE_SCHEDULE_PROVIDER_METADATA_H

#include "idos_providers.h"
#include "providers/idosdataprovidermetadata.h"

class PROVIDERS_EXPORT IDOSFractureScheduleMetadata : public IDOSDataProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    QStringList fileExtensions() const override;
    std::unique_ptr<IDOSDataProvider> create() const override;
    bool canHandle(const QString& filePath) const override;
};

#endif // IDOS_FRACTURE_SCHEDULE_PROVIDER_METADATA_H
