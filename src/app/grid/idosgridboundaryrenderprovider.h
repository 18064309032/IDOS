#ifndef IDOS_GRID_BOUNDARY_RENDER_PROVIDER_H
#define IDOS_GRID_BOUNDARY_RENDER_PROVIDER_H

#include <QPointer>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif
#include <vtkSmartPointer.h>

#include "idos_app.h"
#include "idosrenderprovider.h"

class IDOSGridBoundaryObject;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSGridBoundaryRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSGridBoundaryRenderProvider(IDOSGridBoundaryObject* boundary);
    ~IDOSGridBoundaryRenderProvider() override;

    IDOSGridBoundaryObject* boundary() const;
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

    QPointer<IDOSGridBoundaryObject> m_boundary;
    vtkSmartPointer<vtkActor> m_actor;
    bool m_visible;
    bool m_highlighted;
    double m_opacity;
};

#endif // IDOS_GRID_BOUNDARY_RENDER_PROVIDER_H
