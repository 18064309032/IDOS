#ifndef IDOS_GRID_SURFACE_RENDER_PROVIDER_H
#define IDOS_GRID_SURFACE_RENDER_PROVIDER_H

#include <QPointer>
#include <QStringList>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif
#include <vtkSmartPointer.h>

#include "idos_app.h"
#include "idosrenderprovider.h"

class IDOSGridSurfaceObject;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSGridSurfaceRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSGridSurfaceRenderProvider(IDOSGridSurfaceObject* surface);
    ~IDOSGridSurfaceRenderProvider() override;

    IDOSGridSurfaceObject* surface() const;
    void update() override;
    QList<vtkProp*> props() const override;
    QVariantMap saveState() const override;
    bool restoreState(const QVariantMap& state) override;
    bool isVisible() const;
    void setVisible(bool visible);
    bool isSelected() const;
    void setSelected(bool highlighted);
    double opacity() const;
    void setOpacity(double opacity);

  private:
    void applyActorProperties();

    QPointer<IDOSGridSurfaceObject> m_surface;
    vtkSmartPointer<vtkActor> m_actor;
    bool m_visible;
    bool m_highlighted;
    double m_opacity;
};

#endif // IDOS_GRID_SURFACE_RENDER_PROVIDER_H
