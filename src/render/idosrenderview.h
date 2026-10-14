#ifndef IDOS_RENDER_VIEW_H
#define IDOS_RENDER_VIEW_H

#include <QColor>
#include <QHash>
#include <QImage>
#include <QList>
#include <QPoint>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <QWidget>

#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

#include "idos_render.h"
#include "idosrendertypes.h"

class IDOSTool;
class IDOSToolBar;

class IDOSRenderObject;
class IDOSRenderScene;
class QFrame;
class QGraphicsOpacityEffect;
class QLabel;
class QPropertyAnimation;
class QVTKOpenGLNativeWidget;
class vtkActor;
class vtkAxesActor;
class vtkCamera;
class vtkObject;
class vtkGenericOpenGLRenderWindow;
class vtkInteractorStyleImage;
class vtkInteractorStyleTrackballCamera;
class vtkOrientationMarkerWidget;
class vtkRenderer;
class vtkProp;

class RENDER_EXPORT IDOSRenderView : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSRenderView(QWidget* parent = nullptr);
    ~IDOSRenderView() override;

    // The view owns this scene for its entire lifetime.
    IDOSRenderScene* scene() const;
    QVariantMap presentationState() const;
    bool restorePresentationState(const QVariantMap& state);

    QList<QString> selection() const;
    void setSelection(const QList<QString>& ids);
    void render();
    bool pickWorldPosition(const QPoint& position, double worldPosition[3]) const;
    QString objectIdAt(const QPoint& position) const;
    void zoomToWorldPosition(const double worldPosition[3]);
    void resetCamera();
    bool fitSelection();
    void rotateView(bool clockwise);
    void saveHomeView();
    void resetToHomeView();
    bool setParallelProjection(bool enabled);
    bool parallelProjection() const;
    void scaleView(double factor);
    void setDisplayVisible(bool visible);
    bool linkCameraTo(IDOSRenderView* sourceView);
    void unlinkCamera();
    bool isCameraLinked() const;
    bool isCameraLinkedTo(const IDOSRenderView* view) const;
    bool cursorTrackingEnabled() const;
    void setCursorTrackingEnabled(bool enabled);
    void setTwoDimensional(bool enabled);
    bool isTwoDimensional() const;
    IDOSOrientation orientation() const;
    QColor background() const;
    void setBackground(const QColor& color);
    void setActive(bool active);
    QImage captureImage();
    void flashScreenshot();
    bool axesVisible() const;
    void setAxesVisible(bool visible);
    bool legendVisible() const;
    bool legendAvailable() const;
    void setLegendVisible(bool visible);
    bool setOrientation(IDOSOrientation orientation);
    bool setToolBar(IDOSToolBar* toolBar);
    IDOSToolBar* toolBar() const;
    QVTKOpenGLNativeWidget* vtkWidget() const;
    bool enableTool(IDOSTool* tool);
    bool disableTool(IDOSTool* tool);
    bool isToolEnabled(const IDOSTool* tool) const;
    QList<IDOSTool*> enabledTools() const;
    bool captureTool(IDOSTool* tool);
    void releaseTool(IDOSTool* tool);
    IDOSTool* capturedTool() const;
    bool isToolTransitioning() const;

  signals:
    void focused();
    void selectionChanged(const QList<QString>& ids);
    void decorationsChanged();
    void activated(const QString& objectId);
    void toolEnabled(IDOSTool* tool);
    void toolDisabled(IDOSTool* tool);
    void viewStateChanged();
    void viewModeChanged();

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

  private slots:
    void onObjectAboutToBeRemoved(const QString& id);
    void onSceneChanged();
    void onToolDestroyed(QObject* object);
    void onToolBarDestroyed(QObject* object);
    void onPendingTools();
    void onToolCallbackFinished(bool consumed);
    void onToolBarLayoutChanged();
    void onLinkedCameraModified(vtkObject* source, unsigned long eventId, void* callData);
    void onCameraInteraction();

  private:
    bool toolsBusy() const;
    static QVariantMap cameraState(vtkCamera* camera);
    static bool restoreCameraState(vtkCamera* camera, const QVariantMap& state);
    void positionToolBar();
    void cancelToolInteraction();
    void clearEnabledTools();
    QPointer<IDOSToolBar> m_toolBar;
    QList<QPointer<IDOSTool>> m_enabledTools;
    QPointer<IDOSTool> m_capturedTool;
    QList<QPointer<IDOSTool>> m_pendingTools;
    QList<bool> m_pendingEnabled;
    bool m_toolsChanging;
    void configureVtkOutputWindow();
    void rebuildActors();
    IDOSRenderObject* pick(const QPoint& position) const;
    void updateActiveBorder();

    QVTKOpenGLNativeWidget* m_vtkWidget;
    QFrame* m_activeBorder;
    QWidget* m_flashOverlay;
    QLabel* m_cursorCoordinates;
    QGraphicsOpacityEffect* m_flashOpacityEffect;
    QPropertyAnimation* m_flashAnimation;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkCamera> m_homeCamera;
    vtkSmartPointer<vtkCamera> m_savedCamera;
    vtkWeakPointer<vtkCamera> m_linkedCamera;
    unsigned long m_cameraObserverTag;
    unsigned long m_interactionObserverTag;
    vtkSmartPointer<vtkInteractorStyleImage> m_twoDInteractorStyle;
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> m_threeDInteractorStyle;
    vtkSmartPointer<vtkAxesActor> m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationMarker;
    IDOSRenderScene* m_scene;
    QHash<vtkActor*, QString> m_actorObjectIds;
    QList<QString> m_selection;
    QHash<QString, bool> m_displayVisibilitySnapshot;
    IDOSOrientation m_orientation;
    bool m_active;
    bool m_twoDimensional;
    bool m_homeTwoDimensional;
    bool m_homeViewInitialized;
    bool m_cursorTrackingEnabled;
    bool m_legendVisible;
    QList<vtkProp*> m_legends;
};

#endif // IDOS_RENDER_VIEW_H
