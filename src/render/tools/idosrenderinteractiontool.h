#ifndef IDOS_RENDER_INTERACTION_TOOL_H
#define IDOS_RENDER_INTERACTION_TOOL_H


#include <array>

#include <QList>
#include <QPointer>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include "idos_render.h"
#include "idostool.h"

class QEvent;
class QDialog;
class QLabel;
class QPoint;
class IDOSRenderView;
class vtkActor2D;
class vtkActor;
class vtkCallbackCommand;
class vtkGenericOpenGLRenderWindow;
class vtkDataSetAttributes;
class vtkDistanceRepresentation2D;
class vtkDistanceWidget;
class vtkImplicitPlaneRepresentation;
class vtkImplicitPlaneWidget2;
class vtkMapper;
class vtkObject;
class vtkPlane;
class vtkPolyData;

class vtkProp;

class IDOSRenderProvider;

class RENDER_EXPORT IDOSRenderInteractionTool : public IDOSTool
{
    Q_OBJECT
public:
    enum class Mode
    {
        PointZoom,
        PointProbe,
        DistanceMeasurement,
        Selection,
        ClippingPlane,
        SliceByPoints
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
    static QList<vtkActor*> actors(const IDOSRenderProvider* provider);
    static void onClipPlaneInteraction(vtkObject* caller, unsigned long eventId,
                                       void* clientData, void* callData);
    void onDeactivateRequested();
    void onSceneChanged();
    void onSceneObjectAboutToBeRemoved(const QString& id);
    void showPrompt(const QString& text) const;
    void handlePickedPoint(const double worldPosition[3]);
    bool pickSelectedObjectWorldPosition(const QPoint& position, double worldPosition[3]) const;
    bool startClippingPlane();
    void updateClippingPlane();
    void clearClippingPlane();
    void addSlicePoint(const double worldPosition[3]);
    void updateSlicePreview();
    void clearSlicePreview();
    void executeSlice();
    void showSliceProfile(vtkPolyData* profile);
    void clearSliceProfileWindow();
    void onSliceProfileWindowDestroyed(QObject* object);
    void clearMeasurement();
    void updateMeasurement(const double worldPosition[3]);
    void performProbe(const QPoint& position);
    void clearProbe(bool redraw = true);
    QString probeAttributes(vtkDataSetAttributes* attributes, vtkIdType tupleId) const;

    Mode m_mode;
    QLabel* m_promptLabel;
    vtkSmartPointer<vtkDistanceRepresentation2D> m_measurementRepresentation;
    vtkSmartPointer<vtkDistanceWidget> m_measurementWidget;
    vtkSmartPointer<vtkActor2D> m_probeActor;
    vtkSmartPointer<vtkImplicitPlaneWidget2> m_clipWidget;
    vtkSmartPointer<vtkImplicitPlaneRepresentation> m_clipRepresentation;
    vtkSmartPointer<vtkPlane> m_clipPlane;
    vtkSmartPointer<vtkCallbackCommand> m_clipCallback;
    unsigned long m_clipObserverTag;
    QList<vtkSmartPointer<vtkMapper>> m_clippedMappers;
    QString m_clipObjectId;
    vtkSmartPointer<vtkActor> m_slicePreviewActor;
    QPointer<QDialog> m_sliceDialog;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_sliceRenderWindow;
    QString m_sliceObjectId;
    QList<std::array<double, 3>> m_slicePoints;
    bool m_hasFirstPoint;
    double m_firstPoint[3];
};

#endif // IDOS_RENDER_INTERACTION_TOOL_H
