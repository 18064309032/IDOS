#include <QThread>

#include "idostoolbar.h"
#include "idostoolprovidermetadata.h"

#include "idostoolproviderregistry.h"

IDOSToolProviderRegistry::IDOSToolProviderRegistry(QObject* parent)
    :
    QObject(parent)
    , m_changing(false)
{
}

IDOSToolProviderRegistry::~IDOSToolProviderRegistry()
{
    clear();
}

bool IDOSToolProviderRegistry::registerMetadata(IDOSToolProviderMetadata* metadata)
{
    if (isBusy() || QThread::currentThread() != thread() || metadata == nullptr)
    {
        return false;
    }
    const QString id = metadata->id();
    if (id.isEmpty() || m_metadataById.contains(id))
    {
        return false;
    }
    m_changing = true;
    m_metadataById.insert(id, metadata);
    emit metadataRegistered(id);
    m_changing = false;
    return true;
}

bool IDOSToolProviderRegistry::unregisterMetadata(const QString& id)
{
    if (isBusy() || QThread::currentThread() != thread() || !m_metadataById.contains(id))
    {
        return false;
    }
    m_changing = true;
    emit metadataUnregistering(id);
    delete m_metadataById.take(id);
    emit metadataUnregistered(id);
    m_changing = false;
    return true;
}

bool IDOSToolProviderRegistry::isBusy() const
{
    if (m_changing)
    {
        return true;
    }
    for (const QPointer<IDOSToolBar>& toolBar : m_toolBars)
    {
        if (toolBar != nullptr && toolBar->isBusy())
        {
            return true;
        }
    }
    return false;
}

void IDOSToolProviderRegistry::clear()
{
    const QList<QString> ids = m_metadataById.keys();
    for (const QString& id : ids)
    {
        unregisterMetadata(id);
    }
}

IDOSToolProviderMetadata* IDOSToolProviderRegistry::metadata(const QString& id) const
{
    return m_metadataById.value(id, nullptr);
}

QList<IDOSToolProviderMetadata*> IDOSToolProviderRegistry::metadata() const
{
    return m_metadataById.values();
}

void IDOSToolProviderRegistry::attach(IDOSToolBar* toolBar)
{
    if (toolBar == nullptr || toolBar->registry() != this || toolBar->thread() != thread())
    {
        return;
    }
    if (!m_toolBars.contains(QPointer<IDOSToolBar>(toolBar)))
    {
        m_toolBars.append(QPointer<IDOSToolBar>(toolBar));
    }
}

void IDOSToolProviderRegistry::detach(IDOSToolBar* toolBar)
{
    if (toolBar == nullptr || toolBar->registry() != this)
    {
        return;
    }
    m_toolBars.removeAll(QPointer<IDOSToolBar>(toolBar));
}
