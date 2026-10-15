#include "data/fracture/idosnaturalfractureset.h"
#include "idosnaturalfracturerenderprovider.h"

#include "idosnaturalfracturerendermetadata.h"

QString IDOSNaturalFractureRenderMetadata::typeId() const
{
    return QStringLiteral("idos.naturalFracture.set");
}

IDOSRenderProvider* IDOSNaturalFractureRenderMetadata::create(IDOSDataObject* data) const
{
    IDOSNaturalFractureSet* fractures = qobject_cast<IDOSNaturalFractureSet*>(data);
    if (fractures == nullptr)
    {
        return nullptr;
    }
    return new IDOSNaturalFractureRenderProvider(fractures);
}
