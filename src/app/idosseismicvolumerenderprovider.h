#ifndef IDOS_SEISMIC_VOLUME_RENDER_PROVIDER_H
#define IDOS_SEISMIC_VOLUME_RENDER_PROVIDER_H

#include <QList>
#include <QPointer>
#include <QStringList>

#include <vtkSmartPointer.h>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif

#include "idos_app.h"
#include "idosrenderprovider.h"

class IDOSSeismicVolume;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSSeismicVolumeRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSSeismicVolumeRenderProvider(IDOSSeismicVolume* volume);
    ~IDOSSeismicVolumeRenderProvider() override;

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
    int inlineNumber() const;
    bool setInlineNumber(int inlineNumber);
    QList<int> inlineNumbers() const;

  private:
    void applyActorProperties();

    QPointer<IDOSSeismicVolume> m_volume;
    vtkSmartPointer<vtkActor> m_actor;
    int m_inlineNumber;
    bool m_visible;
    bool m_highlighted;
    double m_opacity;
};

#endif // IDOS_SEISMIC_VOLUME_RENDER_PROVIDER_H
