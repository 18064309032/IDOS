#ifndef IDOS_DATA_LOAD_SERVICE_H
#define IDOS_DATA_LOAD_SERVICE_H

#include <QString>
#include <QStringList>

#include "idos_providers.h"

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
    IDOSDataLoadService();

    /**
     * @brief 从文件加载数据到工程。
     * @param targetGridId 非空时进入属性定向导入模式：只保留 provider 返回的
     *                    IDOSGridProperty 对象，将其 gridId 覆盖为该值后挂到目标网格；
     *                    provider 顺带创建的网格/工况对象丢弃不导入。
     * @param targetCaseId 非空且 targetGridId 为空时进入网格定向导入模式：只接收网格
     *                    及其属性，网格以 "case.grid"、属性以 "case.gridProperty" 角色
     *                    加入该工况引用集；工况等附带对象丢弃。属性定向导入模式下仅
     *                    追加 "case.gridProperty" 引用。
     * @return 新加入工程的 objectId 列表；失败返回空并可通过 lastError() 获取错误信息。
     */
    QStringList loadFile(const QString& filePath, IDOSProject* project,
                         const QString& targetGridId = QString(),
                         const QString& targetCaseId = QString());

    QString lastError() const;

  private:
    QString m_lastError;
};

#endif // IDOS_DATA_LOAD_SERVICE_H
