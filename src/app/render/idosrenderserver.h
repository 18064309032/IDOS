#ifndef IDOS_RENDER_SERVER_H
#define IDOS_RENDER_SERVER_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QtCore/qnamespace.h>

#include "idos_app.h"

class IDOSDataObject;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderProvider;
class IDOSRenderProviderRegistry;
class IDOSRenderView;

class APP_EXPORT IDOSRenderServer : public QObject
{
    Q_OBJECT
  public:
    // Registry and registered views are borrowed; each registered scene is managed by this server.
    explicit IDOSRenderServer(IDOSRenderProviderRegistry* registry, QObject* parent = nullptr);
    ~IDOSRenderServer() override;
    IDOSProject* project() const;
    void setProject(IDOSProject* project);
    void addView(IDOSRenderView* view);
    void removeView(IDOSRenderView* view);
    QList<IDOSRenderView*> views() const;
    IDOSRenderView* currentView() const;
    bool setCurrentView(IDOSRenderView* view);
    bool isCheckable(const QString& dataId) const;
    Qt::CheckState checkState(const QString& dataId, IDOSRenderView* view) const;
    void setChecked(const QString& dataId, Qt::CheckState state, IDOSRenderView* view);
    QList<QString> objectIds(IDOSRenderView* view, const QString& dataId) const;
    IDOSDataObject* dataObject(IDOSRenderView* view, const QString& objectId) const;
    QVariantMap saveState() const;
    bool restoreState(const QVariantMap& state);
  signals:
    void objectsChanged(IDOSRenderView* view, const QStringList& dataIds, bool dataChanged);
    void dataAboutToBeRemoved(const QString& dataId);
    void currentViewChanged(IDOSRenderView* view);
    void stateChanged();
  private slots:
    void onMetadataRegistered(const QString& typeId);
    void onMetadataUnregistering(const QString& typeId);
    void onProjectDestroyed();
    void onViewDestroyed(QObject* object);
    void onObjectAdded(const QString& id);
    void onObjectsAdded(const QStringList& ids);
    void onObjectChanged(const QString& id);
    void onObjectRemoved(const QString& id);
    void onObjectsRemoved(const QStringList& ids);
    void onObjectsChanged(const QStringList& ids);
    void onObjectAboutToBeRemoved(const QString& id);
    void onSceneChanged();
    void onRenderObjectRemoved(const QString& id);

  private:
    void restoreRemovedObjects(IDOSRenderView* view);
    QStringList metadataDependencies(IDOSRenderObject* object, IDOSDataObject* data) const;
    QList<IDOSDataObject*> references(const QStringList& ids) const;
    bool refreshProvider(IDOSRenderObject* object, IDOSDataObject* data, const QVariantMap& state);
    void reload();
    void populate(IDOSRenderView* view);
    void update(const QStringList& ids, bool dataChanged);
    bool update(IDOSRenderView* view, IDOSDataObject* data, bool dataChanged, bool create = false);
    QHash<QString, QString> m_sources;
    QHash<IDOSRenderView*, QHash<QString, QVariantMap>> m_removedStates;
    QStringList m_removingIds;
    QPointer<IDOSProject> m_project;
    QPointer<IDOSRenderProviderRegistry> m_registry;
    QList<IDOSRenderView*> m_views;
    QPointer<IDOSRenderView> m_currentView;
};

#endif // IDOS_RENDER_SERVER_H
