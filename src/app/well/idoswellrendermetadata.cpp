#include <QObject>
#include <QtGlobal>

#include "data/well/idoswellobject.h"
#include "idosrenderprovider.h"
#include "idoswellrenderprovider.h"

#include "idoswellrendermetadata.h"

QString IDOSWellRenderMetadata::typeId() const
{
    return QStringLiteral("idos.well");
}

IDOSRenderProvider* IDOSWellRenderMetadata::create(IDOSDataObject* dataObject, const QList<IDOSDataObject*>& references) const
{
    Q_UNUSED(references)
    IDOSWellObject* source = qobject_cast<IDOSWellObject*>(dataObject);
    if (source == nullptr)
    {
        return nullptr;
    }
    return new IDOSWellRenderProvider(source);
}
