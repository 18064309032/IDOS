#ifndef IDOS_RENDER_METADATA_H
#define IDOS_RENDER_METADATA_H

#include <QString>

#include "idos_render.h"

class IDOSDataObject;
class IDOSRenderProvider;

class RENDER_EXPORT IDOSRenderMetadata
{
  public:
    virtual ~IDOSRenderMetadata();

    virtual QString typeId() const = 0;
    virtual IDOSRenderProvider* create(IDOSDataObject* data) const = 0;
};

#endif // IDOS_RENDER_METADATA_H
