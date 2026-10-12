#ifndef IDOS_RENDER_VIEW_H
#define IDOS_RENDER_VIEW_H

#include <QColor>
#include <QHash>
#include <QImage>
#include <QList>
#include <QPoint>
#include <QPointer>
#include <QString>
#include <QWidget>

#include <vtkSmartPointer.h>

#include "idos_render.h"
#include "idosrendertypes.h"

class IDOSRenderObject;
class IDOSProject;
class IDOSRenderRegistry;
class IDOSRenderScene;
class QFrame;
class QMenu;
class QGraphicsOpacityEffect;
class QPropertyAnimation;
class QVTKOpenGLNativeWidget;
class vtkActor;
class vtkAxesActor;
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
    void setRegistry(IDOSRenderRegistry* registry);
    void setProject(IDOSProject* project);
    IDOSProject* project() const;
    IDOSRenderObject* menuObject() const;
    QList<IDOSRenderObject*> selectedObjects() const;
    QMenu* createMenu(const QString& objectId);

    QList<QString> selection() const;
    void setSelection(const QList<QString>& ids);
    void fit(const QList<QString>& ids);
    void render();
    void resetCamera();
    void setTwoDimensional(bool enabled);
    bool isTwoDimensional() const;
    IDOSOrientation orientation() const;
    QColor background() const;
    void setBackground(const QColor& color);
    void setActive(bool active);
    bool isActive() const;
    QImage captureImage();
    void flashScreenshot();
    bool axesVisible() const;
    void setAxesVisible(bool visible);
    bool legendVisible() const;
    bool legendAvailable() const;
    void setLegendVisible(bool visible);
    bool setOrientation(IDOSOrientation orientation);

  signals:
    void focused();
    void selectionChanged(const QList<QString>& ids);
    void decorationsChanged();
    void activated(const QString& objectId);
    void contextMenuAboutToShow(QMenu* menu);

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

  private slots:
    void onObjectAboutToBeRemoved(const QString& id);
    void onSceneChanged();
    void onFitObject();
    void onOpacity();
    void onHideObject();
    void onResetCamera();

  private:
    void configureVtkOutputWindow();
    void rebuildActors();
    void activateObjectAt(const QPoint& position);
    IDOSRenderObject* pick(const QPoint& position) const;
    IDOSRenderObject* actionObject() const;
    void updateActiveBorder();

    QVTKOpenGLNativeWidget* m_vtkWidget;
    QFrame* m_activeBorder;
    QWidget* m_flashOverlay;
    QGraphicsOpacityEffect* m_flashOpacityEffect;
    QPropertyAnimation* m_flashAnimation;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkInteractorStyleImage> m_twoDInteractorStyle;
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> m_threeDInteractorStyle;
    vtkSmartPointer<vtkAxesActor> m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationMarker;
    IDOSRenderScene* m_scene;
    QPointer<IDOSProject> m_project;
    QPointer<IDOSRenderRegistry> m_registry;
    QString m_menuId;
    QHash<vtkActor*, QString> m_actorObjectIds;
    QPoint m_pressedPosition;
    QList<QString> m_selection;
    IDOSOrientation m_orientation;
    bool m_active;
    bool m_legendVisible;
    QList<vtkProp*> m_legends;
};

#endif // IDOS_RENDER_VIEW_H
