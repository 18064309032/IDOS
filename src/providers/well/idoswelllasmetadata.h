#ifndef IDOS_WELL_LAS_METADATA_H
#define IDOS_WELL_LAS_METADATA_H

#include <QObject>

#include "idosprovidermetadata.h"

/**
 * @brief LAS 井 provider 的元数据 + 工厂。
 *
 * 使用方式：
 *   IDOSProviderMetadata* meta = registry.metadata("idos.well.las");
 *   std::unique_ptr<IDOSDataProvider> provider = meta->createProvider();
 *   provider->read(path, project);
 */
class PROVIDERS_EXPORT IDOSWellLasMetadata : public IDOSProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    QStringList fileExtensions() const override;

    std::unique_ptr<IDOSDataProvider> createProvider() const override;
};

#endif // IDOS_WELL_LAS_METADATA_H
