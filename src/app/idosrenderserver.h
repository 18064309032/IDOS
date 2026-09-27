#ifndef IDOS_RENDER_SERVER_H
#define IDOS_RENDER_SERVER_H

#include "idos_app.h"
#include "idosrendertypes.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

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

    void addProvider(IDOSRenderObjectProvider* provider);

  public slots:
    void onItemCheckedChanged(const QString& objectId, bool checked);
    void onItemStateChanged(const QString& objectId, IDOSItemState state);

  signals:
    void titleChanged(const QString& title);

  private:
    IDOSRenderObject* createObject(const IDOSDataObject* object) const;
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
    QList<IDOSRenderObjectProvider*> m_providers;
};

#endif // IDOS_RENDER_SERVER_H

