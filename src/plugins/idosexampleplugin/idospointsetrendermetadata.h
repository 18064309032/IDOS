#ifndef IDOS_POINT_SET_RENDER_METADATA_H
#define IDOS_POINT_SET_RENDER_METADATA_H

#include "providers/idosrendermetadata.h"

class IDOSPointSetRenderMetadata : public IDOSRenderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* data) const override;
};

#endif
