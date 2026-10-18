#ifndef IDOS_RENDER_PROVIDER_REGISTRY_H
#define IDOS_RENDER_PROVIDER_REGISTRY_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include "idos_app.h"

class IDOSRenderProviderMetadata;

class APP_EXPORT IDOSRenderProviderRegistry : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSRenderProviderRegistry(QObject* parent = nullptr);
    ~IDOSRenderProviderRegistry() override;

    bool registerMetadata(IDOSRenderProviderMetadata* metadata);
    bool unregisterMetadata(const QString& dataTypeId);
    IDOSRenderProviderMetadata* metadata(const QString& dataTypeId) const;
    QList<IDOSRenderProviderMetadata*> metadata() const;
    void clear();

  signals:
    void metadataRegistered(const QString& typeId);
    void metadataUnregistering(const QString& typeId);
    void metadataUnregistered(const QString& typeId);

  private:
    QMap<QString, IDOSRenderProviderMetadata*> m_metadataByTypeId;
    QString m_unregisteringId;
    bool m_changing;
};

#endif // IDOS_RENDER_PROVIDER_REGISTRY_H
