#ifndef IDOS_RENDER_VIEW_H
#define IDOS_RENDER_VIEW_H

#include <QColor>
#include <QHash>
#include <QImage>
#include <QList>
#include <QPoint>
#include <QString>
#include <QWidget>

#include <vtkSmartPointer.h>

#include "idos_render.h"
#include "idosrendertypes.h"

class IDOSRenderObject;
class IDOSRenderScene;
class QFrame;
class QGraphicsOpacityEffect;
class QPropertyAnimation;
class QVTKOpenGLNativeWidget;
class vtkActor;
class vtkAxesActor;
class vtkGenericOpenGLRenderWindow;
class vtkOrientationMarkerWidget;
class vtkRenderer;
class vtkProp;

class RENDER_EXPORT IDOSRenderView : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSRenderView(QWidget* parent = nullptr);
    ~IDOSRenderView() override;

    IDOSRenderScene* scene() const;
    void setScene(IDOSRenderScene* scene);

    void addObject(IDOSRenderObject* object);
    void setObject(IDOSRenderObject* object);
    QString currentObjectId() const;
    void clear();
    void refresh();
    void resetCamera();
    void setParallelProjection(bool enabled);
    bool parallelProjection() const;
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);
    void setActive(bool active);
    bool isActive() const;
    QImage captureImage();
    void flashScreenshot();
    bool orientationMarkerVisible() const;
    void setOrientationMarkerVisible(bool visible);
    bool legendVisible() const;
    bool legendAvailable() const;
    void setLegendVisible(bool visible);
    bool setOrientation(IDOSOrientation orientation);
    void setHighlightedObjectId(const QString& objectId);

  signals:
    void activated();
    void decorationsChanged();
    void objectActivated(const QString& objectId);

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

  private:
    void configureVtkOutputWindow();
    void rebuildActors();
    void activateObjectAt(const QPoint& position);
    void updateActiveBorder();

    QVTKOpenGLNativeWidget* m_vtkWidget;
    QFrame* m_activeBorder;
    QWidget* m_flashOverlay;
    QGraphicsOpacityEffect* m_flashOpacityEffect;
    QPropertyAnimation* m_flashAnimation;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkAxesActor> m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationMarker;
    IDOSRenderScene* m_scene;
    QHash<vtkActor*, QString> m_actorObjectIds;
    QPoint m_pressedPosition;
    QString m_highlightedObjectId;
    bool m_active;
    bool m_ownsScene;
    bool m_legendVisible;
    QList<vtkProp*> m_legends;
};

#endif // IDOS_RENDER_VIEW_H

