#ifndef IDOS_WELL_LAS_PROVIDER_H
#define IDOS_WELL_LAS_PROVIDER_H

#include "idosdataprovider.h"

/**
 * @brief LAS 2.0 / 3.0 井数据解析 provider（骨架实现）。
 *
 * 只负责解析逻辑，元数据（id / 扩展名 / 格式检测）在
 * IDOSWellLasMetadata 里。每次通过 Metadata::createProvider()
 * 返回 fresh 实例。
 *
 * 解析 LAS 文件的 Well Section（井名、坐标、深度参考等），
 * 构建 IDOSWell 并返回。轨迹段和射孔段解析后续扩展。
 */
class PROVIDERS_EXPORT IDOSWellLasProvider : public IDOSDataProvider
{
public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
};

#endif // IDOS_WELL_LAS_PROVIDER_H
