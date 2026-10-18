#ifndef IDOS_POINT_SET_RENDER_METADATA_H
#define IDOS_POINT_SET_RENDER_METADATA_H

#include "render/idosrenderprovidermetadata.h"

class IDOSPointSetRenderMetadata : public IDOSRenderProviderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const override;
};

#endif
