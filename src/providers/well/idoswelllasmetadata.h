#ifndef IDOS_WELL_LAS_METADATA_H
#define IDOS_WELL_LAS_METADATA_H

#include "idosprovidermetadata.h"

/**
 * @brief LAS 井 provider 的元数据 + 工厂。
 *
 * 注册方式（GUI 启动时）：
 *   IDOSProviderRegistry::instance().registerMetadata(
 *       std::make_unique<IDOSWellLasMetadata>());
 *
 * 使用方式：
 *   auto* meta = registry.metadata("idos.well.las");
 *   auto provider = meta->createProvider();
 *   provider->read(path, project);
 */
class PROVIDERS_EXPORT IDOSWellLasMetadata : public IDOSProviderMetadata
{
public:
    QString id() const override { return QStringLiteral("idos.well.las"); }
    QString displayName() const override { return QStringLiteral("LAS Well"); }
    QStringList fileExtensions() const override
    {
        return QStringList{ QStringLiteral("*.las"), QStringLiteral("*.LAS") };
    }

    std::unique_ptr<IDOSDataProvider> createProvider() const override;
};

#endif // IDOS_WELL_LAS_METADATA_H
