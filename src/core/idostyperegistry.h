#ifndef IDOS_TYPE_REGISTRY_H
#define IDOS_TYPE_REGISTRY_H

#include "idos_core.h"
#include "idosobject.h"
#include <QHash>
#include <QList>
#include <QString>
#include <memory>

class IDOSDataObject;

/**
 * @brief 数据对象类别：决定对象归入哪棵树。
 *
 * 按类型分流，实例不存储类别（同一类型永远属于同一边）：
 * 输入数据（井、地震、流体等）归输入树；模型对象（网格、工况、属性场等）归模型树。
 * 模型通过 objectId 引用输入数据，数据本体永远只有一份。
 */
enum class IDOSObjectCategory
{
    Input,   ///< 输入数据，归输入树
    Model    ///< 模型对象，归模型树
};

/**
 * @brief 数据对象类型的元数据档案卡（扩展点）。
 *
 * 描述一种数据类型：稳定 typeId、显示名、工厂函数、类别与输入树分组。
 * 官方类型与插件类型走同一注册通道，非 QObject，由注册表通过 unique_ptr 持有。
 */
class CORE_EXPORT IDOSObjectTypeMetadata
{
public:
    /**
     * @brief 构造类型元数据。
     * @param typeId 稳定类型标识（如 "idos.well"），插件建议用带命名空间的逆域名形式。
     * @param displayName 显示名（UI 展示用）。
     * @param category 类别（输入树 / 模型树）。
     * @param inputGroup 仅 Input 类别有效：输入树中的分组名（如"井组"），为空则不归组。
     */
    IDOSObjectTypeMetadata(const QString& typeId, const QString& displayName,
                           IDOSObjectCategory category,
                           const QString& inputGroup = QString());
    virtual ~IDOSObjectTypeMetadata();

    /** 稳定类型标识。 */
    QString typeId() const;

    /** 显示名。 */
    QString displayName() const;

    /** 类别：输入树 / 模型树。 */
    IDOSObjectCategory category() const;

    /** 输入树分组名（仅 Input 类别有意义），为空表示不归组。 */
    QString inputGroup() const;

    /**
     * @brief 工厂：创建该类型的数据对象。
     * @param parent Qt 父对象，用于父子所有权管理生命周期。
     * @return 新建的对象；创建失败返回 nullptr。
     */
    virtual IDOSDataObject* create(QObject* parent = nullptr) const = 0;

private:
    QString m_typeId;
    QString m_displayName;
    IDOSObjectCategory m_category;
    QString m_inputGroup;
};

/**
 * @brief 默认模板实现：create 直接 new T(parent)。
 *
 * T 必须是 IDOSDataObject 子类且提供 QObject* 构造函数。
 * 模板无需导出（MSVC 模板不导出），仅在头文件内实例化使用。
 */
template<typename T>
class IDOSObjectTypeMetadataImpl : public IDOSObjectTypeMetadata
{
public:
    using IDOSObjectTypeMetadata::IDOSObjectTypeMetadata;

    IDOSDataObject* create(QObject* parent = nullptr) const override
    {
        return new T(parent);
    }
};

/**
 * @brief 数据对象类型注册表（单例）。
 *
 * typeId → 元数据 的全局映射，供序列化重建、树展示分发、插件注册使用。
 * 首次调用 instance() 时自动注册内置类型（idos.well / idos.grid）。
 * 单例定义在 idos_core.dll 内，跨模块/未来插件 DLL 共享同一实例。
 */
class CORE_EXPORT IDOSTypeRegistry
{
public:
    /** 全局唯一实例；首次调用时注册内置类型。 */
    static IDOSTypeRegistry& instance();

    /** 析构实现放 .cpp（unique_ptr 成员要求 out-of-line，避免 MSVC C4150）。 */
    ~IDOSTypeRegistry();

    /**
     * @brief 注册类型元数据，接管所有权。
     * @param meta 元数据；同 typeId 重复注册时覆盖旧的并告警。
     */
    void registerType(std::unique_ptr<IDOSObjectTypeMetadata> meta);

    /**
     * @brief 按 typeId 创建数据对象。
     * @param typeId 稳定类型标识。
     * @param parent Qt 父对象。
     * @return 新对象；未知 typeId 返回 nullptr 并告警。
     */
    IDOSDataObject* create(const QString& typeId, QObject* parent = nullptr) const;

    /** 查询元数据；未知 typeId 返回 nullptr。 */
    const IDOSObjectTypeMetadata* metadata(const QString& typeId) const;

    /**
     * @brief 查询类型类别；未知 typeId 按 Model 处理（保持"未识别类型进模型树"的兜底行为）。
     */
    IDOSObjectCategory categoryOf(const QString& typeId) const;

    /** 已注册的全部类型元数据。 */
    QList<const IDOSObjectTypeMetadata*> knownTypes() const;

private:
    IDOSTypeRegistry();
    Q_DISABLE_COPY(IDOSTypeRegistry)

    void registerBuiltinTypes();

    // Qt 容器要求可拷贝值类型，故内部存裸指针；所有权归注册表，析构时统一释放
    QHash<QString, IDOSObjectTypeMetadata*> m_types;
};

#endif // IDOS_TYPE_REGISTRY_H
