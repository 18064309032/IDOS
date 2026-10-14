#ifndef IDOS_RENDER_INTERACTION_TOOL_H
#define IDOS_RENDER_INTERACTION_TOOL_H

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include "idos_render.h"
#include "idostool.h"

class QEvent;
class QLabel;
class QPoint;
class IDOSRenderView;
class vtkActor2D;
class vtkDataSetAttributes;
class vtkDistanceRepresentation2D;
class vtkDistanceWidget;

class RENDER_EXPORT IDOSRenderInteractionTool : public IDOSTool
{
    Q_OBJECT
public:
    enum class Mode
    {
        PointZoom,
        PointProbe,
        DistanceMeasurement,
        Selection
    };

    IDOSRenderInteractionTool(IDOSRenderView* view, Mode mode);
    ~IDOSRenderInteractionTool() override;
    QString exclusiveGroup() const override;
    void cancel() override;

protected:
    bool onEventFilter(QObject* watched, QEvent* event) override;
    void onActivated() override;
    void onDeactivated() override;

private:
    void onDeactivateRequested();
    void showPrompt(const QString& text) const;
    void handlePickedPoint(const double worldPosition[3]);
    void clearMeasurement();
    void updateMeasurement(const double worldPosition[3]);
    void performProbe(const QPoint& position);
    void clearProbe();
    QString probeAttributes(vtkDataSetAttributes* attributes, vtkIdType tupleId) const;

    Mode m_mode;
    QLabel* m_promptLabel;
    vtkSmartPointer<vtkDistanceRepresentation2D> m_measurementRepresentation;
    vtkSmartPointer<vtkDistanceWidget> m_measurementWidget;
    vtkSmartPointer<vtkActor2D> m_probeActor;
    bool m_hasFirstPoint;
    double m_firstPoint[3];
};

#endif // IDOS_RENDER_INTERACTION_TOOL_H
