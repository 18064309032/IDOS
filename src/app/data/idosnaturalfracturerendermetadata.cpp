#include <QtGlobal>

#include "data/fracture/idosnaturalfracturesetobject.h"
#include "idosnaturalfracturerenderprovider.h"

#include "idosnaturalfracturerendermetadata.h"

QString IDOSNaturalFractureRenderMetadata::typeId() const
{
    return QStringLiteral("idos.naturalFracture.set");
}

IDOSRenderProvider* IDOSNaturalFractureRenderMetadata::create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const
{
    Q_UNUSED(references)
    IDOSNaturalFractureSetObject* fractures = qobject_cast<IDOSNaturalFractureSetObject*>(data);
    if (fractures == nullptr)
    {
        return nullptr;
    }
    return new IDOSNaturalFractureRenderProvider(fractures);
}
