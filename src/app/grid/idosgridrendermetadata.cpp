#include <QObject>

#include "idosgrid.h"
#include "idosgridrenderprovider.h"
#include "idosrenderprovider.h"

#include "idosgridrendermetadata.h"

QString IDOSGridRenderMetadata::typeId() const
{
    return QStringLiteral("idos.grid");
}

IDOSRenderProvider* IDOSGridRenderMetadata::create(IDOSDataObject* dataObject) const
{
    IDOSGrid* source = qobject_cast<IDOSGrid*>(dataObject);
    if (source == nullptr)
    {
        return nullptr;
    }
    return new IDOSGridRenderProvider(source);
}
