#ifndef IDOS_RENDER_SERVER_H
#define IDOS_RENDER_SERVER_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include "idos_app.h"
#include "idosrendertypes.h"

class IDOSDataObject;
class IDOSGridProperty;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderObjectProvider;
class IDOSRenderScene;
class IDOSRenderView;

class APP_EXPORT IDOSRenderServer : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSRenderServer(QObject* parent = nullptr);
    ~IDOSRenderServer() override;

    void setProject(IDOSProject* project);
    IDOSProject* project() const;

    IDOSRenderScene* mainScene() const;

    void addView(const QString& viewId, IDOSRenderView* view);
    void removeView(const QString& viewId);
    IDOSRenderView* view(const QString& viewId) const;

    void setActiveView(const QString& viewId);
    QString activeViewId() const;
    IDOSRenderView* activeView() const;
    void setHighlightedObjectId(const QString& objectId);
    void setSelectedObjectId(const QString& objectId);
    int selectedObjectOpacityPercent() const;
    void setSelectedObjectOpacityPercent(int opacityPercent);
    void resetActiveViewCamera();
    void setActiveViewOrientation(IDOSOrientation orientation);
    IDOSDisplayMode selectedGridDisplayMode() const;
    void setSelectedGridDisplayMode(IDOSDisplayMode mode);

    void addProvider(IDOSRenderObjectProvider* provider);

  public slots:
    void onItemCheckedChanged(const QString& objectId, bool checked);
    void onItemStateChanged(const QString& objectId, IDOSItemState state);

  signals:
    void currentViewChanged(const QString& viewId);
    void titleChanged(const QString& title);
    void selectedObjectOpacityChanged(int opacityPercent, bool enabled);
    void selectedGridDisplayModeChanged(IDOSDisplayMode mode, bool enabled);

  private slots:
    void onViewActivated();
    void onObjectAdded(const QString& objectId);
    void onObjectsAdded(const QStringList& objectIds);
    void onObjectRemoved(const QString& objectId);
    void onObjectsRemoved(const QStringList& objectIds);

  private:
    IDOSRenderObject* createObject(const IDOSDataObject* object) const;
    bool addWellToScene(const IDOSDataObject* object);
    IDOSRenderView* firstView() const;
    void refreshViews();
    void resetViews();
    bool showGridProperty(const IDOSGridProperty* property);
    bool hideGridProperty(const IDOSGridProperty* property);
    bool applyGridProperty(IDOSRenderObject* renderObject, const IDOSGridProperty* property);
    void setItemShown(const QString& objectId);
    void setItemHidden(const QString& objectId);
    void setItemSelectionState(const QString& objectId, IDOSItemState state);
    void setItemHighlightState(const QString& objectId, IDOSItemState state);

    IDOSProject* m_project;
    IDOSRenderScene* m_mainScene;
    QMap<QString, IDOSRenderView*> m_views;
    QString m_activeViewId;
    QString m_highlightedObjectId;
    QString m_selectedGridId;
    QMap<QString, int> m_opacityPercentByObjectId;
    QMap<QString, IDOSDisplayMode> m_displayModeByGridId;
    QList<IDOSRenderObjectProvider*> m_providers;
};

#endif // IDOS_RENDER_SERVER_H

