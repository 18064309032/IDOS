#ifndef IDOS_GRID_ECLIPSE_METADATA_H
#define IDOS_GRID_ECLIPSE_METADATA_H

#include "idosprovidermetadata.h"

/**
 * @brief ECLIPSE 网格 provider 的元数据 + 工厂。
 *
 * 扩展名匹配 *.GRID / *.EGRID（不区分大小写）；
 * canHandle 默认按扩展名匹配（基类已实现，无需 override）。
 */
class PROVIDERS_EXPORT IDOSGridEclipseMetadata : public IDOSProviderMetadata
{
public:
    QString id() const override { return QStringLiteral("idos.grid.eclipse"); }
    QString displayName() const override { return QStringLiteral("ECLIPSE Grid"); }
    QStringList fileExtensions() const override
    {
        return QStringList{ QStringLiteral("*.GRID"), QStringLiteral("*.EGRID") };
    }

    std::unique_ptr<IDOSDataProvider> createProvider() const override;
};

#endif // IDOS_GRID_ECLIPSE_METADATA_H
