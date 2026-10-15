#ifndef IDOS_NATURAL_FRACTURE_RENDER_PROVIDER_H
#define IDOS_NATURAL_FRACTURE_RENDER_PROVIDER_H

#include <QPointer>

#include <vtkSmartPointer.h>
#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif

#include "idos_app.h"
#include "providers/idosrenderprovider.h"

class IDOSNaturalFractureSet;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSNaturalFractureRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSNaturalFractureRenderProvider(IDOSNaturalFractureSet* fractures);
    ~IDOSNaturalFractureRenderProvider() override;
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
    IDOSDisplayMode displayMode() const override;
    bool setDisplayMode(IDOSDisplayMode mode) override;

  private:
    void applyActorProperties();
    QPointer<IDOSNaturalFractureSet> m_fractures;
    vtkSmartPointer<vtkActor> m_actor;
    bool m_visible;
    bool m_highlighted;
    double m_opacity;
    IDOSDisplayMode m_displayMode;
};

#endif // IDOS_NATURAL_FRACTURE_RENDER_PROVIDER_H
