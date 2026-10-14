#ifndef IDOS_GRID_SURFACE_RENDER_METADATA_H
#define IDOS_GRID_SURFACE_RENDER_METADATA_H

#include "idos_app.h"
#include "idosrendermetadata.h"

class APP_EXPORT IDOSGridSurfaceRenderMetadata : public IDOSRenderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* data) const override;
};

#endif // IDOS_GRID_SURFACE_RENDER_METADATA_H
