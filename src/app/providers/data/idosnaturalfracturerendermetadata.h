#ifndef IDOS_NATURAL_FRACTURE_RENDER_METADATA_H
#define IDOS_NATURAL_FRACTURE_RENDER_METADATA_H

#include "idos_app.h"
#include "providers/idosrendermetadata.h"

class APP_EXPORT IDOSNaturalFractureRenderMetadata : public IDOSRenderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* data) const override;
};

#endif // IDOS_NATURAL_FRACTURE_RENDER_METADATA_H
