#ifndef IDOS_WELL_HEADER_METADATA_H
#define IDOS_WELL_HEADER_METADATA_H

#include "idosprovidermetadata.h"

/**
 * @brief 井头汇总表 provider 的元数据 + 工厂。
 *
 * 文件名固定为 Wellheader.txt，canHandle 按文件名匹配
 * （包含 "wellheader"），不靠扩展名。
 */
class PROVIDERS_EXPORT IDOSWellHeaderMetadata : public IDOSProviderMetadata
{
public:
    QString id() const override
    { return QStringLiteral("idos.well.header"); }

    QString displayName() const override
    { return QStringLiteral("Well Header"); }

    QStringList fileExtensions() const override
    { return QStringList{ QStringLiteral("Wellheader.txt") }; }

    bool canHandle(const QString& filePath) const override;

    std::unique_ptr<IDOSDataProvider> createProvider() const override;
};

#endif // IDOS_WELL_HEADER_METADATA_H
