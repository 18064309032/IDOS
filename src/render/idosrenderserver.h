#ifndef IDOS_RENDER_SERVER_H
#define IDOS_RENDER_SERVER_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "idos_render.h"

class IDOSDataObject;
class IDOSProject;
class IDOSRenderProvider;
class IDOSRenderRegistry;
class IDOSRenderView;

class RENDER_EXPORT IDOSRenderServer : public QObject
{
    Q_OBJECT
  public:
    // Registry and registered views are borrowed; each registered scene is managed by this server.
    explicit IDOSRenderServer(IDOSRenderRegistry* registry, QObject* parent = nullptr);
    ~IDOSRenderServer() override;
    IDOSProject* project() const;
    void setProject(IDOSProject* project);
    void addView(IDOSRenderView* view);
    void removeView(IDOSRenderView* view);
    QList<IDOSRenderView*> views() const;
    IDOSRenderView* currentView() const;
    bool setCurrentView(IDOSRenderView* view);
    QList<QString> objectIds(IDOSRenderView* view, const QString& dataId) const;
    IDOSDataObject* dataSource(IDOSRenderView* view, const QString& objectId) const;
    QVariantMap presentationState() const;
    void restorePresentationState(const QVariantMap& state);
  signals:
    void objectsChanged(IDOSRenderView* view, const QStringList& dataIds, bool dataChanged);
    void dataAboutToBeRemoved(const QString& dataId);
    void currentViewChanged(IDOSRenderView* view);
  private slots:
    void onMetadataAdded(const QString& typeId);
    void onMetadataAboutToBeRemoved(const QString& typeId);
    void onProjectDestroyed();
    void onViewDestroyed(QObject* object);
    void onObjectAdded(const QString& id);
    void onObjectsAdded(const QStringList& ids);
    void onObjectChanged(const QString& id);
    void onObjectRemoved(const QString& id);
    void onObjectsRemoved(const QStringList& ids);
    void onObjectsChanged(const QStringList& ids);
    void onObjectAboutToBeRemoved(const QString& id);

  private:
    void bindDataReferences(IDOSRenderProvider* provider);
    void reload();
    void populate(IDOSRenderView* view);
    void update(const QStringList& ids, bool dataChanged);
    bool update(IDOSRenderView* view, IDOSDataObject* data, bool dataChanged);
    QPointer<IDOSProject> m_project;
    QPointer<IDOSRenderRegistry> m_registry;
    QList<IDOSRenderView*> m_views;
    QPointer<IDOSRenderView> m_currentView;
};

#endif // IDOS_RENDER_SERVER_H
