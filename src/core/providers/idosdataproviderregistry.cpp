#include <QFileInfo>
#include <QObject>

#include "idosdataprovider.h"

#include "idosdataproviderregistry.h"

IDOSDataProviderRegistry::IDOSDataProviderRegistry()
{

}

IDOSDataProviderRegistry::~IDOSDataProviderRegistry()
{
}

IDOSDataProviderRegistry& IDOSDataProviderRegistry::instance()
{
    static IDOSDataProviderRegistry s_instance;
    return s_instance;
}

bool IDOSDataProviderRegistry::registerMetadata(IDOSDataProviderMetadata* metadata)
{
    if (!metadata || metadata->id().isEmpty() || this->metadata(metadata->id()))
    {
        return false;
    }

    m_metadata.push_back(std::unique_ptr<IDOSDataProviderMetadata>(metadata));
    return true;
}

bool IDOSDataProviderRegistry::unregisterMetadata(const QString& id)
{
    for (std::vector<std::unique_ptr<IDOSDataProviderMetadata>>::iterator item = m_metadata.begin();
         item != m_metadata.end(); ++item)
    {
        if ((*item)->id() == id)
        {
            m_metadata.erase(item);
            return true;
        }
    }
    return false;
}

IDOSDataProviderMetadata* IDOSDataProviderRegistry::metadata(const QString& id) const
{
    for (const std::unique_ptr<IDOSDataProviderMetadata>& item : m_metadata)
    {
        if (item->id() == id)
        {
            return item.get();
        }
    }
    return nullptr;
}

std::unique_ptr<IDOSDataProvider> IDOSDataProviderRegistry::create(const QString& id) const
{
    IDOSDataProviderMetadata* meta = metadata(id);
    if (!meta)
    {
        return nullptr;
    }
    return meta->create();
}

QList<IDOSDataProviderMetadata*> IDOSDataProviderRegistry::metadata() const
{
    QList<IDOSDataProviderMetadata*> result;
    for (const std::unique_ptr<IDOSDataProviderMetadata>& item : m_metadata)
    {
        result.append(item.get());
    }
    return result;
}

QList<IDOSDataProviderMetadata*> IDOSDataProviderRegistry::metadataForFile(const QString& filePath) const
{
    QList<IDOSDataProviderMetadata*> trueMatch; // canHandle() 真检测匹配
    QList<IDOSDataProviderMetadata*> extMatch;  // 仅扩展名匹配

    for (IDOSDataProviderMetadata* m : metadata())
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

QString IDOSDataProviderRegistry::fileFilters(const QString& providerIdPrefix) const
{
    QStringList allExtensions;
    QStringList perProvider;

    for (IDOSDataProviderMetadata* m : metadata())
    {
        if (!providerIdPrefix.isEmpty() && !m->id().startsWith(providerIdPrefix))
        {
            continue;
        }
        QStringList exts = m->fileExtensions();
        allExtensions.append(exts);
        perProvider.append(QStringLiteral("%1 (%2)").arg(m->displayName(), exts.join(" ")));
    }

    if (allExtensions.isEmpty())
    {
        return QString();
    }

    allExtensions.removeDuplicates();
    QString allFilter = QObject::tr("All Supported Files (%1)").arg(allExtensions.join(" "));
    QStringList result;
    result.append(allFilter);
    result.append(perProvider);
    return result.join(QStringLiteral(";;"));
}
