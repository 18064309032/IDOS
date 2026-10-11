#ifndef IDOS_TREE_H
#define IDOS_TREE_H

#include <QObject>
#include <QPointer>
#include <QString>

#include "idos_core.h"

class IDOSProject;
class IDOSTreeItem;

class CORE_EXPORT IDOSTree : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSTree(QObject* parent = nullptr);
    ~IDOSTree() override;
    IDOSTreeItem* root() const;
    IDOSProject* project() const;
    void setProject(IDOSProject* project);
    QString name() const;
    void setName(const QString& name);
    bool insert(IDOSTreeItem* parent, IDOSTreeItem* item, int row = -1);
    bool remove(IDOSTreeItem* item);
    void clear();
    IDOSTreeItem* item(const QString& id) const;
    IDOSTreeItem* find(const QString& key) const;
    void changed(IDOSTreeItem* item);

  signals:
    void projectChanged();
    void aboutToInsert(IDOSTreeItem* parent, int row);
    void inserted();
    void aboutToRemove(IDOSTreeItem* parent, int row);
    void removed();
    void itemChanged(IDOSTreeItem* item);

  private:
    IDOSTreeItem* lookup(IDOSTreeItem* branch, const QString& value, bool byId) const;
    IDOSTreeItem* m_root;
    QPointer<IDOSProject> m_project;
    QString m_name;
    bool m_changing;
};

#endif
