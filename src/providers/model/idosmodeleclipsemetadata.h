#ifndef IDOS_MODEL_ECLIPSE_METADATA_H
#define IDOS_MODEL_ECLIPSE_METADATA_H

#include "idosprovidermetadata.h"

/**
 * @brief ECLIPSE 主输入文件（.DATA）工况 provider 的元数据 + 工厂。
 *
 * 注册方式（GUI 启动时）：
 *   IDOSProviderRegistry::instance().registerMetadata(
 *       std::make_unique<IDOSModelEclipseMetadata>());
 *
 * 扩展名匹配 *.DATA / *.data（不区分大小写）；canHandle 默认按扩展名匹配。
 */
class PROVIDERS_EXPORT IDOSModelEclipseMetadata : public IDOSProviderMetadata
{
public:
    QString id() const override { return QStringLiteral("idos.model.eclipse"); }
    QString displayName() const override { return QStringLiteral("ECLIPSE Model"); }
    QStringList fileExtensions() const override
    {
        return QStringList{ QStringLiteral("*.DATA"), QStringLiteral("*.data") };
    }

    std::unique_ptr<IDOSDataProvider> createProvider() const override;
};

#endif // IDOS_MODEL_ECLIPSE_METADATA_H
