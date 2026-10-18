#ifndef IDOS_DATA_REGISTRY_H
#define IDOS_DATA_REGISTRY_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

#include "idos_core.h"

class IDOSDataMetadata;

class CORE_EXPORT IDOSDataRegistry : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSDataRegistry(QObject* parent = nullptr);
    ~IDOSDataRegistry() override;

    bool registerMetadata(IDOSDataMetadata* metadata);
    bool unregisterMetadata(const QString& typeId);
    const IDOSDataMetadata* metadata(const QString& typeId) const;
    QList<const IDOSDataMetadata*> metadata() const;

  signals:
    void metadataRegistered(const QString& typeId);
    void metadataUnregistering(const QString& typeId);
    void metadataUnregistered(const QString& typeId);

  private:
    Q_DISABLE_COPY(IDOSDataRegistry)

    QHash<QString, IDOSDataMetadata*> m_metadata;
    bool m_changing;
};

#endif
