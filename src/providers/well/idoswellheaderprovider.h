#ifndef IDOS_WELL_HEADER_PROVIDER_H
#define IDOS_WELL_HEADER_PROVIDER_H

#include "idosdataprovider.h"

/**
 * @brief 井头汇总表解析 provider（Wellheader.txt）。
 *
 * 解析空白分隔的井头汇总表，为每行新建 IDOSWell 并填充
 * wellHead（X/Y/KB/顶底深）。返回的对象无 parent，调用方
 * 负责按井名合并到 Project。
 */
class PROVIDERS_EXPORT IDOSWellHeaderProvider : public IDOSDataProvider
{
public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
};

#endif // IDOS_WELL_HEADER_PROVIDER_H
