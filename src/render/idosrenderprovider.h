#ifndef IDOS_RENDER_PROVIDER_H
#define IDOS_RENDER_PROVIDER_H

#include <QList>

#include <vtkSmartPointer.h>

#include "idos_render.h"
#include "idosrendertypes.h"

class vtkActor;
class vtkProp;

class RENDER_EXPORT IDOSRenderProvider
{
  public:
    virtual ~IDOSRenderProvider();

    virtual void update() = 0;
    virtual QList<vtkActor*> actors() const = 0;
    virtual QList<vtkSmartPointer<vtkProp>> legends() const = 0;
    virtual void setHighlighted(bool highlighted) = 0;
    virtual bool visible() const = 0;
    virtual void setVisible(bool visible) = 0;
    virtual double opacity() const = 0;
    virtual void setOpacity(double opacity) = 0;
    virtual bool supportsDisplayMode() const;
    virtual IDOSDisplayMode displayMode() const;
    virtual bool setDisplayMode(IDOSDisplayMode mode);
};

#endif // IDOS_RENDER_PROVIDER_H
