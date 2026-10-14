#ifndef IDOS_WELL_LAS_METADATA_H
#define IDOS_WELL_LAS_METADATA_H

#include <QObject>

#include "idos_providers.h"
#include "providers/idosdataprovidermetadata.h"

/**
 * @brief LAS 井 provider 的元数据 + 工厂。
 *
 * 使用方式：
 *   IDOSDataProviderMetadata* meta = registry.metadata("idos.well.las");
 *   std::unique_ptr<IDOSDataProvider> provider = meta->create();
 *   provider->read(path, project);
 */
class PROVIDERS_EXPORT IDOSWellLasMetadata : public IDOSDataProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    bool canHandle(const QString& filePath) const override;
    QStringList fileExtensions() const override;

    std::unique_ptr<IDOSDataProvider> create() const override;
};

#endif // IDOS_WELL_LAS_METADATA_H
