#ifndef IDOS_GRID_SURFACE_RENDER_PROVIDER_H
#define IDOS_GRID_SURFACE_RENDER_PROVIDER_H

#include <QPointer>
#include <QStringList>

#include <vtkSmartPointer.h>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif

#include "idos_app.h"
#include "idosrenderprovider.h"

class IDOSGridSurface;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSGridSurfaceRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSGridSurfaceRenderProvider(IDOSGridSurface* surface);
    ~IDOSGridSurfaceRenderProvider() override;

    IDOSDataObject* dataSource() const override;
    void update() override;
    QList<vtkActor*> actors() const override;
    QList<vtkSmartPointer<vtkProp>> legends() const override;
    bool visible() const override;
    void setVisible(bool visible) override;
    bool highlighted() const override;
    void setHighlighted(bool highlighted) override;
    double opacity() const override;
    void setOpacity(double opacity) override;

  private:
    void applyActorProperties();

    QPointer<IDOSGridSurface> m_surface;
    vtkSmartPointer<vtkActor> m_actor;
    bool m_visible;
    bool m_highlighted;
    double m_opacity;
};

#endif // IDOS_GRID_SURFACE_RENDER_PROVIDER_H
