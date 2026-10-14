#ifndef IDOS_GRID_BOUNDARY_RENDER_METADATA_H
#define IDOS_GRID_BOUNDARY_RENDER_METADATA_H

#include "idos_app.h"
#include "providers/idosrendermetadata.h"

class APP_EXPORT IDOSGridBoundaryRenderMetadata : public IDOSRenderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* data) const override;
};

#endif // IDOS_GRID_BOUNDARY_RENDER_METADATA_H
