#include "data/seismic/idosseismicvolume.h"
#include "idosseismicvolumerenderprovider.h"

#include "idosseismicvolumerendermetadata.h"

QString IDOSSeismicVolumeRenderMetadata::typeId() const
{
    return QStringLiteral("idos.seismic.volume");
}

IDOSRenderProvider* IDOSSeismicVolumeRenderMetadata::create(IDOSDataObject* data) const
{
    IDOSSeismicVolume* volume = qobject_cast<IDOSSeismicVolume*>(data);
    if (volume == nullptr)
    {
        return nullptr;
    }
    return new IDOSSeismicVolumeRenderProvider(volume);
}
