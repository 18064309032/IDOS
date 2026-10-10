#ifndef IDOS_RENDER_OBJECT_VTK_CONVERTER_H
#define IDOS_RENDER_OBJECT_VTK_CONVERTER_H

#include <QList>

#include <vtkSmartPointer.h>

#include "idos_render.h"

class IDOSRenderObject;
class vtkActor;
class vtkProp;

class RENDER_EXPORT IDOSRenderObjectVtkConverter
{
  public:
    virtual ~IDOSRenderObjectVtkConverter();

    virtual bool canConvert(const IDOSRenderObject* object) const = 0;
    virtual QList<vtkActor*> toVtk(const IDOSRenderObject* object,
                                   bool highlighted) const = 0;
    virtual QList<vtkSmartPointer<vtkProp>> toVtkLegends(const IDOSRenderObject* object) const = 0;
};

#endif // IDOS_RENDER_OBJECT_VTK_CONVERTER_H
