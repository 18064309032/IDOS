#ifndef IDOS_GRID_FILTER_RENDER_METADATA_H
#define IDOS_GRID_FILTER_RENDER_METADATA_H

#include "idos_app.h"
#include "providers/idosrendermetadata.h"

class APP_EXPORT IDOSGridFilterRenderMetadata : public IDOSRenderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* data) const override;
};

#endif
