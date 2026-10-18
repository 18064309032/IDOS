#ifndef IDOS_SEISMIC_VOLUME_RENDER_PROVIDER_H
#define IDOS_SEISMIC_VOLUME_RENDER_PROVIDER_H

#include <QList>
#include <QPointer>
#include <QStringList>
#include <QVariantMap>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif
#include <vtkSmartPointer.h>

#include "idos_app.h"
#include "idosrenderprovider.h"

class IDOSSeismicVolumeObject;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSSeismicVolumeRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSSeismicVolumeRenderProvider(IDOSSeismicVolumeObject* volume);
    ~IDOSSeismicVolumeRenderProvider() override;

    IDOSSeismicVolumeObject* volume() const;
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
    int inlineNumber() const;
    bool setInlineNumber(int inlineNumber);
    QList<int> inlineNumbers() const;

  private:
    void applyActorProperties();

    QPointer<IDOSSeismicVolumeObject> m_volume;
    vtkSmartPointer<vtkActor> m_actor;
    int m_inlineNumber;
    bool m_visible;
    bool m_highlighted;
    double m_opacity;
};

#endif // IDOS_SEISMIC_VOLUME_RENDER_PROVIDER_H
