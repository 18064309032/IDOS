#ifndef IDOS_GRID_RENDER_PROVIDER_H
#define IDOS_GRID_RENDER_PROVIDER_H

#include <QList>
#include <QPointer>

#include <vtkSmartPointer.h>

#include "idos_app.h"
#include "idosrenderprovider.h"
#include "idosrendertypes.h"

class IDOSGrid;
class IDOSGridProperty;

class APP_EXPORT IDOSGridRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSGridRenderProvider(IDOSGrid* dataObject);
    ~IDOSGridRenderProvider() override;
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
    void setProperty(IDOSGridProperty* property);
    IDOSGridProperty* property() const;
    IDOSDisplayMode displayMode() const;
    bool setDisplayMode(IDOSDisplayMode mode);

  private:
    void applyActorProperties();
    QPointer<IDOSGrid> m_dataObject;
    QPointer<IDOSGridProperty> m_property;
    IDOSDisplayMode m_displayMode;
    QList<vtkSmartPointer<vtkActor>> m_actors;
    QList<vtkSmartPointer<vtkProp>> m_legends;
    bool m_highlighted;
    bool m_visible;
    double m_opacity;
};

#endif
