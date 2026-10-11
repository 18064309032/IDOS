#include <QObject>

#include "idoswell.h"
#include "idoswellrenderprovider.h"
#include "idosrenderprovider.h"

#include "idoswellrendermetadata.h"

QString IDOSWellRenderMetadata::typeId() const
{
    return QStringLiteral("idos.well");
}

IDOSRenderProvider* IDOSWellRenderMetadata::create(IDOSDataObject* dataObject) const
{
    IDOSWell* source = qobject_cast<IDOSWell*>(dataObject);
    if (source == nullptr)
    {
        return nullptr;
    }
    return new IDOSWellRenderProvider(source);
}
