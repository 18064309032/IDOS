#ifndef IDOS_PROVIDER_METADATA_H
#define IDOS_PROVIDER_METADATA_H

#include "idos_providers.h"

#include <QString>
#include <QStringList>
#include <memory>

class IDOSDataProvider;

/**
 * @brief 数据解析 provider 的元数据 + 工厂。
 *
 * 职责分离：
 *   IDOSProviderMetadata  —— 元数据（id / 显示名 / 扩展名 / 格式检测）+ 工厂（createProvider）
 *   IDOSDataProvider      —— 纯解析行为（read + lastError）
 *
 * 为什么要分离？
 *   IDOSDataProvider 是有状态的（lastError、未来解析进度/取消），
 *   不能复用同一个实例解析多个文件。Metadata 无状态，每次
 *   createProvider() 返回 fresh instance，用完即弃。
 *
 * 用法：
 *   // 按扩展名查 → 拿 Metadata → 工厂创建 → 解析
 *   QList<IDOSProviderMetadata*> metas = registry.metadataForFile(path);
 *   std::unique_ptr<IDOSDataProvider> provider = metas.first()->createProvider();
 *   provider->read(path, project);
 */
class PROVIDERS_EXPORT IDOSProviderMetadata
{
public:
    virtual ~IDOSProviderMetadata() = default;

    // ===== 元数据 =====

    /** 稳定标识（注册表查找用），如 "idos.well.las"、"idos.grid.eclipse"。 */
    virtual QString id() const = 0;

    /** 用户可读的显示名（文件对话框 / 错误提示用），如 "LAS Well"。 */
    virtual QString displayName() const = 0;

    /**
     * @brief 本 provider 支持的文件扩展名（通配符形式）。
     *
     * 如 {"*.las", "*.LAS"}。注册表汇总所有 metadata 的扩展名生成
     * 文件对话框过滤器；也用于按扩展名反查候选 metadata。
     */
    virtual QStringList fileExtensions() const = 0;

    // ===== 工厂 =====

    /**
     * @brief 创建一个 fresh 的 IDOSDataProvider 实例。
     *
     * 每次调用返回新实例，不复用。调用方接管所有权。
     */
    virtual std::unique_ptr<IDOSDataProvider> createProvider() const = 0;

    // ===== 格式检测 =====

    /**
     * @brief 检测文件是否能被本 provider 处理。
     *
     * 默认实现按扩展名匹配（不区分大小写）。子类可覆盖做真检测
     * （如读文件头判断 LAS 2.0 还是 ECLIPSE），返回 true 表示有把握。
     */
    virtual bool canHandle(const QString& filePath) const;
};

#endif // IDOS_PROVIDER_METADATA_H
