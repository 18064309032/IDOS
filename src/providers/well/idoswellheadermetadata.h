#ifndef IDOS_WELL_HEADER_METADATA_H
#define IDOS_WELL_HEADER_METADATA_H

#include <QObject>

#include "idos_providers.h"
#include "providers/idosdataprovidermetadata.h"

/**
 * @brief 井头汇总表 provider 的元数据 + 工厂。
 *
 * Supports the legacy seven-column Wellheader format and Petrel VERSION 1 files.
 */
class PROVIDERS_EXPORT IDOSWellHeaderMetadata : public IDOSDataProviderMetadata
{
  public:
    QString id() const override;

    QString displayName() const override;

    QStringList fileExtensions() const override;

    bool canHandle(const QString& filePath) const override;

    std::unique_ptr<IDOSDataProvider> createProvider() const override;
};

#endif // IDOS_WELL_HEADER_METADATA_H
