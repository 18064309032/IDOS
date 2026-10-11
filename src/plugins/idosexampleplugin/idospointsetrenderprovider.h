#ifndef IDOS_POINT_SET_RENDER_PROVIDER_H
#define IDOS_POINT_SET_RENDER_PROVIDER_H

#include <QPointer>

#include <vtkSmartPointer.h>

#include "idosrenderprovider.h"

class IDOSPointSet;
class vtkActor;
class vtkPolyData;

class IDOSPointSetRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSPointSetRenderProvider(IDOSPointSet* data);
    ~IDOSPointSetRenderProvider() override;
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
    double pointSize() const;
    void setPointSize(double size);

  private:
    QPointer<IDOSPointSet> m_data;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkPolyData> m_polyData;
    bool m_highlighted;
};

#endif
