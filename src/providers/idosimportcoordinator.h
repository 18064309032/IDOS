#ifndef IDOS_IMPORT_COORDINATOR_H
#define IDOS_IMPORT_COORDINATOR_H

#include "idos_providers.h"
#include <QString>
#include <QStringList>

class IDOSProject;

/**
 * @brief 导入协调者：路由 provider + 把碎片多态合并进 Project。
 *
 * 自己不解析、不写类型判断、不认具体字段：
 *   ① Registry::metadataForFile 找 provider
 *   ② metadata->createProvider + read 产碎片
 *   ③ project->objectByName find-or-create 目标（首次碎片直接 addObject）
 *   ④ target->mergeFrom(frag) 多态合并（well/grid/model 各自语义）
 *
 * 合并语义在 IDOSDataObject::mergeFrom，解析在 IDOSDataProvider，
 * 存储在 IDOSProject——Coordinator 只编排顺序。
 *
 * 无状态工具，每次导入 new 一个、用完即弃。
 * 放 providers 模块：依赖 Registry/Metadata(providers) + Project(core)，
 * 单向 core ← providers ← gui 不破。
 */
class PROVIDERS_EXPORT IDOSImportCoordinator
{
public:
    IDOSImportCoordinator() = default;

    /**
     * @brief 单文件导入，自动路由 + 合并到 project。
     * @return 受影响的 objectId 列表（GUI 据此刷树/选中）；
     *         失败返回空 + lastError()。
     */
    QStringList import(const QString& filePath, IDOSProject* project);

    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
};

#endif // IDOS_IMPORT_COORDINATOR_H
