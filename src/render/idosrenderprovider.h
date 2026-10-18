#ifndef IDOS_RENDER_PROVIDER_H
#define IDOS_RENDER_PROVIDER_H

#include <QList>
#include <QVariantMap>

#include "idos_render.h"

class vtkProp;

class RENDER_EXPORT IDOSRenderProvider
{
public:
    virtual ~IDOSRenderProvider();

    virtual void update() = 0;
    virtual QList<vtkProp*> props() const = 0;
    virtual QVariantMap saveState() const = 0;
    virtual bool restoreState(const QVariantMap& state) = 0;
};

#endif
