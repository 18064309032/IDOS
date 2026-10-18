#ifndef IDOS_TREE_ITEM_PROVIDER_REGISTRY_H
#define IDOS_TREE_ITEM_PROVIDER_REGISTRY_H

#include <QList>
#include <QObject>
#include <QString>

#include "idos_core.h"

class IDOSDataObject;
class IDOSTreeItem;
class IDOSTreeItemProvider;
class IDOSTreeItemProviderMetadata;

class CORE_EXPORT IDOSTreeItemProviderRegistry : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSTreeItemProviderRegistry(QObject* parent = nullptr);
    ~IDOSTreeItemProviderRegistry() override;
    bool registerMetadata(IDOSTreeItemProviderMetadata* metadata);
    bool unregisterMetadata(const QString& metadataId);
    const IDOSTreeItemProviderMetadata* metadata(const QString& metadataId) const;
    QList<const IDOSTreeItemProviderMetadata*> metadata() const;
    IDOSTreeItemProvider* create(IDOSDataObject* data) const;

  signals:
    void metadataRegistered(const QString& metadataId);
    void metadataUnregistering(const QString& metadataId);
    void metadataUnregistered(const QString& metadataId);

  private:
    QList<IDOSTreeItemProviderMetadata*> m_metadata;
    bool m_changing;
    QString m_unregisteringId;
};

#endif
