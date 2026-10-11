#ifndef IDOS_WELL_RENDER_PROVIDER_H
#define IDOS_WELL_RENDER_PROVIDER_H

#include <QList>
#include <QPointer>

#include <vtkSmartPointer.h>

#include "idos_app.h"
#include "idosrenderprovider.h"

class IDOSWell;

class APP_EXPORT IDOSWellRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSWellRenderProvider(IDOSWell* dataObject);
    ~IDOSWellRenderProvider() override;
    IDOSDataObject* dataSource() const override;
    void update() override;
    QList<vtkActor*> actors() const override;
    QList<vtkSmartPointer<vtkProp>> legends() const override;
    bool highlighted() const override;
    void setHighlighted(bool highlighted) override;
    bool visible() const override;
    void setVisible(bool visible) override;
    double opacity() const override;
    void setOpacity(double opacity) override;

  private:
    void applyActorProperties();
    QPointer<IDOSWell> m_dataObject;
    QList<vtkSmartPointer<vtkActor>> m_actors;
    QList<vtkSmartPointer<vtkProp>> m_legends;
    bool m_highlighted;
    bool m_visible;
    double m_opacity;
};

#endif
