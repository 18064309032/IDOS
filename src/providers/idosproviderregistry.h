#ifndef IDOS_PROVIDER_REGISTRY_H
#define IDOS_PROVIDER_REGISTRY_H

#include "idos_providers.h"
#include "idosprovidermetadata.h"

#include <QString>
#include <QStringList>
#include <QList>
#include <memory>

class IDOSProviderMetadata;

/**
 * @brief 数据解析 provider 注册表。
 *
 * 单例，管理所有 IDOSProviderMetadata 子类实例。GUI 启动时集中注册
 * （不自注册，避免跨 DLL 静态初始化顺序问题）。
 *
 * 用法：
 *   // GUI 启动时注册（每种格式一行）
 *   registry.registerMetadata(std::make_unique<IDOSWellLasMetadata>());
 *   registry.registerMetadata(std::make_unique<IDOSGridEclipseMetadata>());
 *
 *   // 按扩展名查 → 拿 Metadata → 工厂创建 → 解析
 *   QList<IDOSProviderMetadata*> metas = registry.metadataForFile(path);
 *   if (!metas.isEmpty()) {
 *       std::unique_ptr<IDOSDataProvider> provider = metas.first()->createProvider();
 *       provider->read(path, project);
 *   }
 *
 *   // 文件对话框过滤器
 *   QString filter = registry.fileFilters();
 *
 * 未来插件可调用 registerMetadata() 注册自己的格式。
 */
class PROVIDERS_EXPORT IDOSProviderRegistry
{
public:
    /** 全局唯一实例。 */
    static IDOSProviderRegistry& instance();

    ~IDOSProviderRegistry();

    /**
     * @brief 注册 metadata，接管所有权。
     * @return true 成功；false 表示同 id 已存在，旧的被替换。
     */
    bool registerMetadata(std::unique_ptr<IDOSProviderMetadata> metadata);

    /** 按 id 查找 metadata；找不到返回 nullptr。 */
    IDOSProviderMetadata* metadata(const QString& id) const;

    /** 按 id 创建 fresh provider 实例；找不到返回 nullptr。 */
    std::unique_ptr<IDOSDataProvider> createProvider(const QString& id) const;

    /** 已注册的全部 metadata。 */
    QList<IDOSProviderMetadata*> metadataList() const;

    /**
     * @brief 按扩展名查候选 metadata（不区分大小写）。
     *
     * 返回列表按优先级排序：canHandle() 真检测返回 true 的排前面，
     * 仅扩展名匹配的排后面；都没匹配上返回空列表。
     */
    QList<IDOSProviderMetadata*> metadataForFile(const QString& filePath) const;

    /**
     * @brief 汇总所有 metadata 的扩展名生成文件对话框过滤器。
     *
     * 格式："All Supported Files (*.las *.LAS *.GRID *.EGRID);;LAS Well (*.las *.LAS);;ECLIPSE Grid (*.GRID *.EGRID)"
     * 直接喂给 QFileDialog::getOpenFileName 的 filter 参数。
     */
    QString fileFilters() const;

private:
    IDOSProviderRegistry();
    Q_DISABLE_COPY(IDOSProviderRegistry)

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

#endif // IDOS_PROVIDER_REGISTRY_H
