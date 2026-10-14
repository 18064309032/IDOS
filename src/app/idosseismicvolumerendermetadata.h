#ifndef IDOS_SEISMIC_VOLUME_RENDER_METADATA_H
#define IDOS_SEISMIC_VOLUME_RENDER_METADATA_H

#include "idos_app.h"
#include "idosrendermetadata.h"

class APP_EXPORT IDOSSeismicVolumeRenderMetadata : public IDOSRenderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* data) const override;
};

#endif // IDOS_SEISMIC_VOLUME_RENDER_METADATA_H
