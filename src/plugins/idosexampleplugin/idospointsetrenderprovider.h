#ifndef IDOS_POINT_SET_RENDER_PROVIDER_H
#define IDOS_POINT_SET_RENDER_PROVIDER_H

#include <QPointer>

#include <vtkSmartPointer.h>

#include "idosrenderprovider.h"

class IDOSPointSetObject;
class vtkActor;
class vtkPolyData;

class IDOSPointSetRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSPointSetRenderProvider(IDOSPointSetObject* data);
    ~IDOSPointSetRenderProvider() override;
    IDOSPointSetObject* pointSet() const;
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
    QPointer<IDOSPointSetObject> m_data;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkPolyData> m_polyData;
    bool m_highlighted;
};

#endif
