#ifndef IDOS_GRID_SURFACE_RENDER_METADATA_H
#define IDOS_GRID_SURFACE_RENDER_METADATA_H

#include "idos_app.h"
#include "render/idosrenderprovidermetadata.h"

class APP_EXPORT IDOSGridSurfaceRenderMetadata : public IDOSRenderProviderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const override;
};

#endif // IDOS_GRID_SURFACE_RENDER_METADATA_H
