#include "idosgridfilter.h"
#include "idosgridfilterrenderprovider.h"

#include "idosgridfilterrendermetadata.h"

QString IDOSGridFilterRenderMetadata::typeId() const
{
    return QStringLiteral("idos.grid.filter");
}

IDOSRenderProvider* IDOSGridFilterRenderMetadata::create(IDOSDataObject* data) const
{
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(data);
    if (filter == nullptr)
    {
        return nullptr;
    }
    return new IDOSGridFilterRenderProvider(filter);
}
