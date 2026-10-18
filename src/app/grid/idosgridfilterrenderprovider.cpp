#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"

#include "idosgridfilterrenderprovider.h"

IDOSGridFilterRenderProvider::IDOSGridFilterRenderProvider(IDOSGridFilterObject* filter, IDOSGridObject* grid)
    :
    IDOSGridRenderProvider(grid)
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

IDOSGridFilterObject* IDOSGridFilterRenderProvider::filter() const
{
    return m_filter.data();
}


void IDOSGridFilterRenderProvider::setProperties(const QList<IDOSGridPropertyObject*>& properties)
{
    IDOSGridRenderProvider::setProperties(properties);
    m_displayProperty = nullptr;
    for (IDOSGridPropertyObject* property : properties)
    {
        if (m_filter != nullptr && property != nullptr &&
            property->objectId() == m_filter->criteria().value(QStringLiteral("displayPropertyId")).toString())
        {
            m_displayProperty = property;
        }
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
