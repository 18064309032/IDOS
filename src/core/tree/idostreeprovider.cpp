#include "idosdataobject.h"

#include "idostreeprovider.h"

IDOSTreeProvider::IDOSTreeProvider(IDOSDataObject* data)
    :
    m_data(data)
{
}

IDOSTreeProvider::~IDOSTreeProvider()
{
}

IDOSDataObject* IDOSTreeProvider::dataSource() const
{
    return m_data;
}

void IDOSTreeProvider::update(IDOSTreeItem* item)
{
    populate(item);
}

bool IDOSTreeProvider::dependsOn(const QStringList& dataIds) const
{
    return m_data != nullptr && dataIds.contains(m_data->objectId());
}
