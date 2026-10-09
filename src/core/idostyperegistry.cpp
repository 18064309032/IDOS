#include <QObject>

#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idossimulationcaseobject.h"
#include "idoswell.h"
#include "log/idoslogger.h"

#include "idostyperegistry.h"

/**
 * @brief 默认模板实现：create 直接 new T(parent)。
 *
 * T 必须是 IDOSDataObject 子类且提供 QObject* 构造函数。
 * 模板无需导出（MSVC 模板不导出），仅在头文件内实例化使用。
 */
template <typename T> class IDOSObjectTypeMetadataImpl : public IDOSObjectTypeMetadata
{
  public:
    using IDOSObjectTypeMetadata::IDOSObjectTypeMetadata;

    IDOSDataObject* create(QObject* parent = nullptr) const override
    {
        return new T(parent);
    }
};

IDOSObjectTypeMetadata::IDOSObjectTypeMetadata(const QString& typeId, const QString& displayName,
                                               IDOSObjectCategory category, const QString& inputGroup)
    : m_typeId(typeId)
    , m_displayName(displayName)
    , m_category(category)
    , m_inputGroup(inputGroup)
{
}

IDOSObjectTypeMetadata::~IDOSObjectTypeMetadata()
{
}

QString IDOSObjectTypeMetadata::typeId() const
{
    return m_typeId;
}

QString IDOSObjectTypeMetadata::displayName() const
{
    return m_displayName;
}

IDOSObjectCategory IDOSObjectTypeMetadata::category() const
{
    return m_category;
}

QString IDOSObjectTypeMetadata::inputGroup() const
{
    return m_inputGroup;
}

IDOSTypeRegistry& IDOSTypeRegistry::instance()
{
    static IDOSTypeRegistry s_instance;
    return s_instance;
}

IDOSTypeRegistry::IDOSTypeRegistry()
{
    registerBuiltinTypes();
}

IDOSTypeRegistry::~IDOSTypeRegistry()
{
    qDeleteAll(m_types);
}

void IDOSTypeRegistry::registerBuiltinTypes()
{
    // 输入数据：归输入树，井挂"井组"分组下（分组标签走翻译，非硬编码中文）
    registerType(std::make_unique<IDOSObjectTypeMetadataImpl<IDOSWell>>(
        QStringLiteral("idos.well"), QObject::tr("Well"), IDOSObjectCategory::Input, QObject::tr("Well Group")));
    // 模型对象：归模型树
    registerType(std::make_unique<IDOSObjectTypeMetadataImpl<IDOSGrid>>(
        QStringLiteral("idos.grid"), QObject::tr("Grid"), IDOSObjectCategory::Model));
    // 模型对象：网格属性，作为网格容器的子对象展示（containerId = gridId）
    registerType(std::make_unique<IDOSObjectTypeMetadataImpl<IDOSGridProperty>>(
        QStringLiteral("idos.gridproperty"), QObject::tr("Grid Property"), IDOSObjectCategory::Model));
    // 工况对象：工况树使用
    registerType(std::make_unique<IDOSObjectTypeMetadataImpl<IDOSSimulationCaseObject>>(
        QStringLiteral("idos.case"), QObject::tr("Case"), IDOSObjectCategory::Model));
}

void IDOSTypeRegistry::registerType(std::unique_ptr<IDOSObjectTypeMetadata> meta)
{
    if (meta == nullptr)
    {
        IDOS_WARN(QObject::tr("IDOSTypeRegistry::registerType: null metadata"));
        return;
    }

    const QString typeId = meta->typeId();
    if (m_types.contains(typeId))
    {
        IDOS_WARN(QObject::tr("IDOSTypeRegistry::registerType: typeId %1 already registered, overwriting")
                      .arg(typeId));
        delete m_types.take(typeId);
    }
    m_types.insert(typeId, meta.release());
}

IDOSDataObject* IDOSTypeRegistry::create(const QString& typeId, QObject* parent) const
{
    QHash<QString, IDOSObjectTypeMetadata*>::const_iterator it = m_types.constFind(typeId);
    if (it == m_types.constEnd())
    {
        IDOS_WARN(QObject::tr("IDOSTypeRegistry::create: unknown typeId %1").arg(typeId));
        return nullptr;
    }
    return it.value()->create(parent);
}

const IDOSObjectTypeMetadata* IDOSTypeRegistry::metadata(const QString& typeId) const
{
    QHash<QString, IDOSObjectTypeMetadata*>::const_iterator it = m_types.constFind(typeId);
    return it == m_types.constEnd() ? nullptr : it.value();
}

IDOSObjectCategory IDOSTypeRegistry::categoryOf(const QString& typeId) const
{
    QHash<QString, IDOSObjectTypeMetadata*>::const_iterator it = m_types.constFind(typeId);
    return it == m_types.constEnd() ? IDOSObjectCategory::Model : it.value()->category();
}

QList<const IDOSObjectTypeMetadata*> IDOSTypeRegistry::knownTypes() const
{
    QList<const IDOSObjectTypeMetadata*> list;
    list.reserve(m_types.size());
    for (QHash<QString, IDOSObjectTypeMetadata*>::const_iterator it = m_types.constBegin(); it != m_types.constEnd();
         ++it)
    {
        list.append(it.value());
    }
    return list;
}
