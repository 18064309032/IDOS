#ifndef IDOS_RENDER_OBJECT_PROVIDER_H
#define IDOS_RENDER_OBJECT_PROVIDER_H

#include <QString>

#include "idos_app.h"

class IDOSDataObject;
class IDOSRenderObject;

class APP_EXPORT IDOSRenderObjectProvider
{
  public:
    IDOSRenderObjectProvider();
    virtual ~IDOSRenderObjectProvider();

    virtual QString providerId() const = 0;
    virtual QString displayName() const = 0;
    virtual bool canCreate(const IDOSDataObject* object) const = 0;
    virtual IDOSRenderObject* createObject(const IDOSDataObject* object) const = 0;
};

#endif // IDOS_RENDER_OBJECT_PROVIDER_H

