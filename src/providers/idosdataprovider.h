#ifndef IDOS_DATA_PROVIDER_H
#define IDOS_DATA_PROVIDER_H

#include "idos_providers.h"

#include <QString>
#include <QList>

class IDOSDataObject;

/**
 * @brief 数据解析行为基类。
 *
 * 只负责解析——从文件读数据、构建 IDOSDataObject、返回给调用方。
 * 元数据（id / 扩展名 / 格式检测）在 IDOSProviderMetadata 里，
 * 工厂也在那边。每次解析用 createProvider() 拿 fresh instance。
 *
 * 非 QObject，轻量无信号槽。错误通过 lastError() 取回，解析进度
 * 未来可扩展 cancel()/progress()。
 *
 * provider 不碰 IDOSProject：返回的对象由调用方（GUI 层）接管
 * 所有权、按井名合并、加进 Project。
 */
class PROVIDERS_EXPORT IDOSDataProvider
{
public:
    virtual ~IDOSDataProvider() = default;

    /**
     * @brief 从文件解析数据对象。
     *
     * @param filePath 数据源路径。
     * @return 成功解析的对象列表（调用方接管所有权）；空列表 +
     *         lastError() 表示失败。
     *
     * 子类负责 new 出 IDOSWell / IDOSGrid 等并填充字段。
     * 对象无 parent，调用方负责加进 Project 或合并后释放。
     */
    virtual QList<IDOSDataObject*> read(const QString& filePath) = 0;

    // ===== 错误 =====

    /** 最近一次 read() 的错误信息；无错误返回空。 */
    QString lastError() const { return m_lastError; }

protected:
    /** 设置错误信息（子类解析失败时调用）。 */
    void setLastError(const QString& msg) { m_lastError = msg; }

private:
    QString m_lastError;
};

#endif // IDOS_DATA_PROVIDER_H
