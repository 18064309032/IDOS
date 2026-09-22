#include "idosproviderregistry.h"
#include "idosdataprovider.h"

#include <QFileInfo>
#include <QHash>
#include <vector>

struct IDOSProviderRegistry::Impl
{
    QHash<QString, IDOSProviderMetadata*> metadatas;   // id → metadata，裸指针（unique_ptr 持有所有权）
    std::vector<std::unique_ptr<IDOSProviderMetadata>> ownership;   // 析构时统一释放
};

IDOSProviderRegistry::IDOSProviderRegistry()
    : m_impl(std::make_unique<Impl>())
{
}

IDOSProviderRegistry::~IDOSProviderRegistry() = default;

IDOSProviderRegistry& IDOSProviderRegistry::instance()
{
    static IDOSProviderRegistry s_instance;
    return s_instance;
}

bool IDOSProviderRegistry::registerMetadata(std::unique_ptr<IDOSProviderMetadata> metadata)
{
    if (!metadata) return false;

    QString id = metadata->id();
    auto it = m_impl->metadatas.find(id);
    if (it != m_impl->metadatas.end())
    {
        // 替换旧的
        auto oldIt = std::find_if(m_impl->ownership.begin(), m_impl->ownership.end(),
            [&](const std::unique_ptr<IDOSProviderMetadata>& p) { return p.get() == it.value(); });
        if (oldIt != m_impl->ownership.end()) m_impl->ownership.erase(oldIt);
        m_impl->metadatas.erase(it);
    }

    IDOSProviderMetadata* raw = metadata.get();
    m_impl->metadatas.insert(id, raw);
    m_impl->ownership.push_back(std::move(metadata));
    return true;
}

IDOSProviderMetadata* IDOSProviderRegistry::metadata(const QString& id) const
{
    return m_impl->metadatas.value(id, nullptr);
}

std::unique_ptr<IDOSDataProvider> IDOSProviderRegistry::createProvider(const QString& id) const
{
    IDOSProviderMetadata* meta = metadata(id);
    if (!meta) return nullptr;
    return meta->createProvider();
}

QList<IDOSProviderMetadata*> IDOSProviderRegistry::metadataList() const
{
    return m_impl->metadatas.values();
}

QList<IDOSProviderMetadata*> IDOSProviderRegistry::metadataForFile(const QString& filePath) const
{
    QList<IDOSProviderMetadata*> trueMatch;   // canHandle() 真检测匹配
    QList<IDOSProviderMetadata*> extMatch;    // 仅扩展名匹配

    for (IDOSProviderMetadata* m : m_impl->metadatas)
    {
        if (m->canHandle(filePath))
        {
            trueMatch.append(m);
        }
        else
        {
            QFileInfo fi(filePath);
            QString suffix = fi.suffix().toLower();
            for (const QString& ext : m->fileExtensions())
            {
                QString pattern = ext.mid(ext.indexOf('.') + 1).toLower();
                if (pattern == suffix)
                {
                    extMatch.append(m);
                    break;
                }
            }
        }
    }

    trueMatch.append(extMatch);
    return trueMatch;
}

QString IDOSProviderRegistry::fileFilters() const
{
    QStringList allExtensions;
    QStringList perProvider;

    for (IDOSProviderMetadata* m : m_impl->metadatas)
    {
        QStringList exts = m->fileExtensions();
        allExtensions.append(exts);
        perProvider.append(QStringLiteral("%1 (%2)").arg(m->displayName(), exts.join(" ")));
    }

    if (allExtensions.isEmpty()) return QString();

    QString allFilter = QStringLiteral("All Supported Files (%1)").arg(allExtensions.join(" "));
    QStringList result;
    result.append(allFilter);
    result.append(perProvider);
    return result.join(QStringLiteral(";;"));
}
