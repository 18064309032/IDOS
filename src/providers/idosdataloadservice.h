#ifndef IDOS_DATA_LOAD_SERVICE_H
#define IDOS_DATA_LOAD_SERVICE_H

#include "idos_providers.h"

#include <QString>
#include <QStringList>

class IDOSProject;

/**
 * @brief 数据加载服务。
 *
 * 负责根据数据源路径查找合适的 provider，读取数据对象并加入工程。
 * 具体格式解析由 IDOSDataProvider 完成，本服务不解析业务字段。
 */
class PROVIDERS_EXPORT IDOSDataLoadService
{
public:
    IDOSDataLoadService() = default;

    /**
     * @brief 从文件加载数据到工程。
     * @return 新加入工程的 objectId 列表；失败返回空并可通过 lastError() 获取错误信息。
     */
    QStringList loadFile(const QString& filePath, IDOSProject* project);

    QString lastError() const;

private:
    QString m_lastError;
};

#endif // IDOS_DATA_LOAD_SERVICE_H
