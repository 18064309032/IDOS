#ifndef IDOS_TREE_SERVER_H
#define IDOS_TREE_SERVER_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QStringList>

#include "idos_core.h"

class IDOSDataObject;
class IDOSProject;
class IDOSTree;
class IDOSTreeItem;
class IDOSTreeRegistry;

class CORE_EXPORT IDOSTreeServer : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSTreeServer(IDOSTreeRegistry* registry, QObject* parent = nullptr);
    ~IDOSTreeServer() override;
    void setProject(IDOSProject* project);
    IDOSProject* project() const;
    bool addTree(IDOSTree* tree);
    bool removeTree(IDOSTree* tree);

  private slots:
    void onObjectAdded(const QString& id);
    void onObjectRemoved(const QString& id);
    void onObjectAboutToRemove(const QString& id);
    void onObjectsChanged(const QStringList& ids);
    void onVisibilityChanged(const QString& id, bool visible);
    void onObjectsVisibilityChanged(const QStringList& ids);
    void onMetadataAdded(const QString& typeId);
    void onMetadataRemoving(const QString& typeId);
    void onGroupsChanged(const QString& name);
    void onProjectDestroyed();

  private:
    void populate(IDOSTree* tree);
    void populateGroups(IDOSTree* tree);
    IDOSTreeItem* copyGroup(const IDOSTreeItem* source) const;
    void addObject(IDOSTree* tree, IDOSDataObject* data);
    void update(const QStringList& ids);
    void updateChecks(IDOSTreeItem* item, const QStringList& ids);
    QPointer<IDOSProject> m_project;
    QPointer<IDOSTreeRegistry> m_registry;
    QList<QPointer<IDOSTree>> m_trees;
};

#endif
