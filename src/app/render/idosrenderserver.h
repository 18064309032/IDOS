#ifndef IDOS_RENDER_SERVER_H
#define IDOS_RENDER_SERVER_H

#include <QColor>
#include <QImage>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include "idos_app.h"
#include "idosrenderobjectprovider.h"
#include "idosrenderregistry.h"
#include "idosrendertypes.h"

class IDOSDataObject;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderScene;
class IDOSRenderView;
class QWidget;

class APP_EXPORT IDOSRenderServer : public QObject,
                                   public IDOSRenderProviderContext
{
    Q_OBJECT

  public:
    explicit IDOSRenderServer(QObject* parent = nullptr);
    ~IDOSRenderServer() override;

    void setProject(IDOSProject* project);
    IDOSProject* project() const;
    IDOSDataObject* dataObject(const QString& objectId) const override;
    QList<IDOSDataObject*> dataObjects() const override;
    IDOSRenderObject* renderObject(const QString& objectId) const override;
    QList<IDOSRenderObject*> renderObjects() const override;
    void addRenderObject(IDOSRenderObject* object) override;
    void removeRenderObject(const QString& objectId) override;

    QWidget* createView(const QString& viewId, bool parallelProjection);
    void removeView(const QString& viewId);

    void setActiveView(const QString& viewId);
    QString activeViewId() const;
    bool hasActiveView() const;
    void setActiveViewOrientation(IDOSOrientation orientation);
    bool activeViewParallelProjection() const;
    QColor activeViewBackgroundColor() const;
    void setActiveViewBackgroundColor(const QColor& color);
    QImage captureActiveViewImage();
    void flashActiveViewScreenshot();
    bool activeViewOrientationMarkerVisible() const;
    bool activeViewLegendAvailable() const;
    bool activeViewLegendVisible() const;
    void setActiveViewOrientationMarkerVisible(bool visible);
    void setActiveViewLegendVisible(bool visible);
    QString activeViewContextObjectId() const;
    void setActiveViewContextObjectId(const QString& objectId);
    void setSelectedObjectId(const QString& objectId);
    int selectedObjectOpacityPercent() const;
    void setSelectedObjectOpacityPercent(int opacityPercent);
    void resetActiveViewCamera();
    IDOSDisplayMode selectedDisplayMode() const;
    void setSelectedDisplayMode(IDOSDisplayMode mode);

    void registerMetadata(IDOSRenderMetadata* metadata);
    bool unregisterMetadata(const QString& dataTypeId);

  signals:
    void currentViewChanged(const QString& viewId);
    void activeViewDecorationsChanged(bool orientationMarkerVisible,
                                      bool legendAvailable,
                                      bool legendVisible);
    void renderViewObjectActivated(const QString& viewId, const QString& objectId);
    void activeViewContextObjectChanged(const QString& viewId, const QString& objectId);
    void titleChanged(const QString& title);
    void selectedObjectOpacityChanged(int opacityPercent, bool enabled);
    void selectedDisplayModeChanged(IDOSDisplayMode mode, bool enabled);

  private slots:
    void onViewActivated();
    void onViewDecorationsChanged();
    void onViewObjectActivated(const QString& objectId);
    void onObjectAdded(const QString& objectId);
    void onObjectsAdded(const QStringList& objectIds);
    void onObjectRemoved(const QString& objectId);
    void onObjectsRemoved(const QStringList& objectIds);
    void onObjectDataChanged(const QString& objectId);
    void onObjectsDataChanged(const QStringList& objectIds);
    void onObjectVisibilityChanged(const QString& objectId, bool visible);
    void onObjectsVisibilityChanged(const QStringList& objectIds);

  private:
    void setHighlightedObjectId(const QString& objectId);
    bool synchronizeProviders(IDOSRenderObjectChange change, const QStringList& objectIds);
    void applyRenderSettings();
    QString viewIdFor(const IDOSRenderView* view) const;
    void emitActiveViewDecorationsChanged();
    void clearViewContextForObject(const QString& objectId);
    void beginSceneUpdate();
    void endSceneUpdate();
    void refreshViews();
    void resetViews();
    IDOSProject* m_project;
    IDOSRenderScene* m_mainScene;
    QMap<QString, IDOSRenderView*> m_views;
    QMap<QString, QString> m_viewContextObjectIds;
    QString m_activeViewId;
    QString m_highlightedObjectId;
    QString m_selectedDataObjectId;
    QString m_selectedRenderObjectId;
    QMap<QString, int> m_opacityPercentByDataId;
    QMap<QString, IDOSDisplayMode> m_displayModeByDataId;
    IDOSRenderRegistry m_renderRegistry;
    QMap<QString, QString> m_renderObjectIdsByDataId;
    QMap<QString, QString> m_dataObjectIdsByRenderObjectId;
    int m_sceneUpdateDepth;
    bool m_refreshPending;
    bool m_resetViewsPending;
};

#endif // IDOS_RENDER_SERVER_H

