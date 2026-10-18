#include <QtGlobal>

#include "data/seismic/idosseismicvolumeobject.h"
#include "idosseismicvolumerenderprovider.h"

#include "idosseismicvolumerendermetadata.h"

QString IDOSSeismicVolumeRenderMetadata::typeId() const
{
    return QStringLiteral("idos.seismic.volume");
}

IDOSRenderProvider* IDOSSeismicVolumeRenderMetadata::create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const
{
    Q_UNUSED(references)
    IDOSSeismicVolumeObject* volume = qobject_cast<IDOSSeismicVolumeObject*>(data);
    if (volume == nullptr)
    {
        return nullptr;
    }
    return new IDOSSeismicVolumeRenderProvider(volume);
}
