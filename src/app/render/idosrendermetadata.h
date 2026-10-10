#ifndef IDOS_RENDER_METADATA_H
#define IDOS_RENDER_METADATA_H

#include <QString>

#include "idos_app.h"

class IDOSDataObject;
class IDOSRenderObject;

class APP_EXPORT IDOSRenderMetadata
{
  public:
    virtual ~IDOSRenderMetadata();

    virtual QString dataTypeId() const = 0;
    virtual IDOSRenderObject* createRenderObject(IDOSDataObject* dataObject) const = 0;
};

#endif // IDOS_RENDER_METADATA_H
