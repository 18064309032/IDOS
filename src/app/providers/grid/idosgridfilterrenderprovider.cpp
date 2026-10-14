#include "data/grid/idosgrid.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"

#include "idosgridfilterrenderprovider.h"

IDOSGridFilterRenderProvider::IDOSGridFilterRenderProvider(IDOSGridFilter* filter)
    :
    IDOSGridRenderProvider(nullptr)
    , m_filter(filter)
    , m_displayProperty(nullptr)
    , m_boundDisplayId()
    , m_displayInitialized(false)
{
    if (filter != nullptr)
    {
        setFilterCriteria(filter->criteria());
    }
}

IDOSGridFilterRenderProvider::~IDOSGridFilterRenderProvider()
{
}

IDOSDataObject* IDOSGridFilterRenderProvider::dataSource() const
{
    return m_filter.data();
}

QStringList IDOSGridFilterRenderProvider::dependencyIds() const
{
    QStringList ids = IDOSGridRenderProvider::dependencyIds();
    if (m_filter != nullptr)
    {
        ids.append(m_filter->gridId());
        const QString displayId = m_filter->criteria().value(QStringLiteral("displayPropertyId")).toString();
        if (!displayId.isEmpty() && !ids.contains(displayId))
        {
            ids.append(displayId);
        }
        for (const QString& id : IDOSGridFilter::propertyIds(m_filter->criteria()))
        {
            if (!ids.contains(id))
            {
                ids.append(id);
            }
        }
    }
    return ids;
}

void IDOSGridFilterRenderProvider::setDataReference(IDOSDataObject* data)
{
    if (m_filter == nullptr)
    {
        return;
    }
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
    if (property != nullptr && property->objectId() == m_filter->criteria().value(QStringLiteral("displayPropertyId")).toString())
    {
        m_displayProperty = property;
    }
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(data);
    if (grid != nullptr && grid->objectId() == m_filter->gridId())
    {
        setGridSource(grid);
    }
    else
    {
        IDOSGridRenderProvider::setDataReference(data);
    }
}

void IDOSGridFilterRenderProvider::update()
{
    if (m_filter != nullptr)
    {
        setFilterCriteria(m_filter->criteria());
        const QString displayId = m_filter->criteria().value(QStringLiteral("displayPropertyId")).toString();
        if (m_filter->criteria().contains(QStringLiteral("displayPropertyId")) && (!m_displayInitialized || m_boundDisplayId != displayId))
        {
            m_boundDisplayId = displayId;
            m_displayInitialized = true;
            setPropertySource(m_displayProperty != nullptr && m_displayProperty->objectId() == m_filter->criteria().value(QStringLiteral("displayPropertyId")).toString()
                ? m_displayProperty.data() : nullptr);
        }
    }
    IDOSGridRenderProvider::update();
}

bool IDOSGridFilterRenderProvider::removeDataReference(const QString& dataId)
{
    if (m_filter != nullptr && m_filter->gridId() == dataId)
    {
        setGridSource(nullptr);
        return true;
    }
    if (m_displayProperty != nullptr && m_displayProperty->objectId() == dataId)
    {
        m_displayProperty = nullptr;
    }
    return IDOSGridRenderProvider::removeDataReference(dataId);
}
