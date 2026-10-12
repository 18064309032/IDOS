#ifndef IDOS_TREE_REGISTRY_H
#define IDOS_TREE_REGISTRY_H

#include <memory>

#include <QList>
#include <QObject>
#include <QString>

#include "idos_core.h"

class IDOSTreeGuiProvider;
class IDOSTreeItem;
class IDOSTreeMetadata;
class IDOSDataRegistry;
class IDOSDataMetadata;

class CORE_EXPORT IDOSTreeRegistry : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSTreeRegistry(QObject* parent = nullptr);
    ~IDOSTreeRegistry() override;
    bool add(IDOSTreeMetadata* metadata);
    bool remove(const QString& typeId);
    const IDOSTreeMetadata* metadata(const QString& typeId) const;
    QList<const IDOSTreeMetadata*> metadata() const;
    QList<const IDOSDataMetadata*> dataMetadata(const IDOSDataRegistry* dataRegistry) const;
    bool addGuiProvider(const std::shared_ptr<IDOSTreeGuiProvider>& provider);
    bool removeGuiProvider(IDOSTreeGuiProvider* provider);
    QList<std::shared_ptr<IDOSTreeGuiProvider>> guiProviders() const;
    bool addGroup(const QString& treeName, IDOSTreeItem* group);
    QList<const IDOSTreeItem*> groups(const QString& treeName) const;

  signals:
    void added(const QString& typeId);
    void aboutToRemove(const QString& typeId);
    void removed(const QString& typeId);
    void guiProvidersChanged();
    void groupsChanged(const QString& treeName);

  private:
    QList<IDOSTreeMetadata*> m_metadata;
    QList<std::shared_ptr<IDOSTreeGuiProvider>> m_guiProviders;
    QList<IDOSTreeItem*> m_groups;
    QList<QString> m_groupTrees;
    bool m_changing;
};

#endif
