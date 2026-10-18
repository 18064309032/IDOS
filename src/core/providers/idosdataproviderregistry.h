#ifndef IDOS_DATA_PROVIDER_REGISTRY_H
#define IDOS_DATA_PROVIDER_REGISTRY_H

#include <memory>
#include <vector>

#include <QList>
#include <QString>
#include <QStringList>

#include "idos_core.h"
#include "idosdataprovidermetadata.h"

class IDOSDataProviderMetadata;

/**
 * @brief 数据解析 provider 注册表。
 *
 * 单例，管理所有 IDOSDataProviderMetadata 子类实例。由应用注册内置 provider，
 * 插件可在启动后追加注册自己的 provider。
 *
 * 用法：
 *   // 按扩展名查 → 拿 Metadata → 工厂创建 → 解析
 *   QList<IDOSDataProviderMetadata*> metas = registry.metadataForFile(path);
 *   if (!metas.isEmpty()) {
 *       std::unique_ptr<IDOSDataProvider> provider = metas.first()->create();
 *       QList<IDOSDataObject*> objects = provider->read(path);
 *   }
 *
 *   // 文件对话框过滤器
 *   QString filter = registry.fileFilters();
 *
 * 未来插件可调用 registerMetadata() 注册自己的格式。
 */
class CORE_EXPORT IDOSDataProviderRegistry
{
  public:
    /** 全局唯一实例。 */
    static IDOSDataProviderRegistry& instance();

    ~IDOSDataProviderRegistry();

    /**
     * @brief 注册 metadata，接管所有权。
     * @return true 成功；false 表示输入无效或同 id 已存在。
     */
    bool registerMetadata(IDOSDataProviderMetadata* metadata);
    bool unregisterMetadata(const QString& id);

    /** 按 id 查找 metadata；找不到返回 nullptr。 */
    IDOSDataProviderMetadata* metadata(const QString& id) const;

    /** 按 id 创建 fresh provider 实例；找不到返回 nullptr。 */
    std::unique_ptr<IDOSDataProvider> create(const QString& id) const;

    /** 已注册的全部 metadata。 */
    QList<IDOSDataProviderMetadata*> metadata() const;


    /**
     * @brief 按扩展名查候选 metadata（不区分大小写）。
     *
     * 返回列表按优先级排序：canHandle() 真检测返回 true 的排前面，
     * 仅扩展名匹配的排后面；都没匹配上返回空列表。
     */
    QList<IDOSDataProviderMetadata*> metadataForFile(const QString& filePath) const;

    /**
     * @brief 汇总所有 metadata 的扩展名生成文件对话框过滤器。
     *
     * 格式："All Supported Files (*.las *.LAS *.GRID *.EGRID);;LAS Well (*.las *.LAS);;ECLIPSE Grid (*.GRID *.EGRID)"
     * 直接喂给 QFileDialog::getOpenFileName 的 filter 参数。
     */
    QString fileFilters(const QString& providerIdPrefix = QString()) const;

  private:
    IDOSDataProviderRegistry();
    Q_DISABLE_COPY(IDOSDataProviderRegistry)

    std::vector<std::unique_ptr<IDOSDataProviderMetadata>> m_metadata;
};

#endif // IDOS_DATA_PROVIDER_REGISTRY_H
