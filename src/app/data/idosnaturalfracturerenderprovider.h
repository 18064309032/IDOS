#ifndef IDOS_NATURAL_FRACTURE_RENDER_PROVIDER_H
#define IDOS_NATURAL_FRACTURE_RENDER_PROVIDER_H

#include <QPointer>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif
#include <vtkSmartPointer.h>

#include "idos_app.h"
#include "idosrenderprovider.h"
#include "idosrendertypes.h"

class IDOSNaturalFractureSetObject;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSNaturalFractureRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSNaturalFractureRenderProvider(IDOSNaturalFractureSetObject* fractures);
    ~IDOSNaturalFractureRenderProvider() override;
    IDOSNaturalFractureSetObject* fractures() const;
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
    IDOSDisplayMode displayMode() const;
    bool setDisplayMode(IDOSDisplayMode mode);

  private:
    void applyActorProperties();
    QPointer<IDOSNaturalFractureSetObject> m_fractures;
    vtkSmartPointer<vtkActor> m_actor;
    bool m_visible;
    bool m_highlighted;
    double m_opacity;
    IDOSDisplayMode m_displayMode;
};

#endif // IDOS_NATURAL_FRACTURE_RENDER_PROVIDER_H
