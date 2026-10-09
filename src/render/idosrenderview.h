#ifndef IDOS_RENDER_VIEW_H
#define IDOS_RENDER_VIEW_H

#include <QHash>
#include <QList>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QWidget>

#include <vtkSmartPointer.h>

#include "idos_render.h"

class IDOSRenderObject;
class IDOSRenderMesh;
class IDOSRenderScene;
class IDOSWellRenderObject;
class QVTKOpenGLNativeWidget;
class vtkActor;
class vtkAxesActor;
class vtkDataSetMapper;
class vtkGenericOpenGLRenderWindow;
class vtkOrientationMarkerWidget;
class vtkRenderer;
class vtkScalarBarActor;
class vtkUnstructuredGrid;

class RENDER_EXPORT IDOSRenderView : public QWidget
{
    Q_OBJECT

  public:
    enum class ViewPreset
    {
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom,
        Isometric
    };

    explicit IDOSRenderView(QWidget* parent = nullptr);
    ~IDOSRenderView() override;

    IDOSRenderScene* scene() const;
    void setScene(IDOSRenderScene* scene);

    void addObject(IDOSRenderObject* object);
    void setObject(IDOSRenderObject* object);
    QString currentObjectId() const;
    bool setCellScalars(const QString& name, const QVector<double>& values);
    void clear();
    void refresh();
    void resetCamera();
    bool orientationMarkerVisible() const;
    void setOrientationMarkerVisible(bool visible);
    bool legendVisible() const;
    bool legendAvailable() const;
    void setLegendVisible(bool visible);
    bool setViewPreset(ViewPreset preset);
    void setHighlightedObjectId(const QString& objectId);

  signals:
    void activated();
    void decorationsChanged();
    void objectActivated(const QString& objectId);

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

  private:
    vtkSmartPointer<vtkActor> createMeshActor(const IDOSRenderMesh* mesh);
    vtkSmartPointer<vtkActor> createWellHeadActor(const IDOSWellRenderObject* well);
    vtkSmartPointer<vtkActor> createWellTrajectoryActor(const IDOSWellRenderObject* well);
    void applyWellStyle(vtkActor* actor, const IDOSWellRenderObject* well) const;
    void applyCellScalars(vtkUnstructuredGrid* grid, vtkDataSetMapper* mapper,
                          const IDOSRenderMesh* mesh) const;
    void configureVtkOutputWindow();
    void clearActivePipeline();
    void rebuildActors();
    void activateObjectAt(const QPoint& position);

    QVTKOpenGLNativeWidget* m_vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkAxesActor> m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationMarker;
    vtkSmartPointer<vtkActor> m_activeActor;
    vtkSmartPointer<vtkUnstructuredGrid> m_activeGrid;
    vtkSmartPointer<vtkDataSetMapper> m_activeMapper;
    IDOSRenderScene* m_scene;
    QHash<vtkActor*, QString> m_actorObjectIds;
    QPoint m_pressedPosition;
    QString m_highlightedObjectId;
    bool m_ownsScene;
    bool m_legendVisible;
    QList<vtkSmartPointer<vtkScalarBarActor>> m_scalarBars;
};

#endif // IDOS_RENDER_VIEW_H

