#include <QThread>

#include "idostoolbar.h"
#include "providers/idostoolmetadata.h"

#include "idostoolregistry.h"

IDOSToolRegistry::IDOSToolRegistry(QObject* parent)
    :
    QObject(parent)
    , m_changing(false)
{
}

IDOSToolRegistry::~IDOSToolRegistry()
{
    clear();
}

bool IDOSToolRegistry::add(IDOSToolMetadata* metadata)
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
    emit metadataAdded(id);
    m_changing = false;
    return true;
}

bool IDOSToolRegistry::remove(const QString& id)
{
    if (isBusy() || QThread::currentThread() != thread() || !m_metadataById.contains(id))
    {
        return false;
    }
    m_changing = true;
    emit metadataAboutToBeRemoved(id);
    delete m_metadataById.take(id);
    m_changing = false;
    return true;
}

bool IDOSToolRegistry::isBusy() const
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

void IDOSToolRegistry::clear()
{
    const QList<QString> ids = m_metadataById.keys();
    for (const QString& id : ids)
    {
        remove(id);
    }
}

IDOSToolMetadata* IDOSToolRegistry::metadata(const QString& id) const
{
    return m_metadataById.value(id, nullptr);
}

QList<IDOSToolMetadata*> IDOSToolRegistry::metadata() const
{
    return m_metadataById.values();
}

void IDOSToolRegistry::attach(IDOSToolBar* toolBar)
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

void IDOSToolRegistry::detach(IDOSToolBar* toolBar)
{
    if (toolBar == nullptr || toolBar->registry() != this)
    {
        return;
    }
    m_toolBars.removeAll(QPointer<IDOSToolBar>(toolBar));
}
