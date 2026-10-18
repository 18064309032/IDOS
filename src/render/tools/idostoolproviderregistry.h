#ifndef IDOS_TOOL_PROVIDER_REGISTRY_H
#define IDOS_TOOL_PROVIDER_REGISTRY_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>

#include "idos_render.h"

class IDOSToolBar;
class IDOSToolProviderMetadata;

class RENDER_EXPORT IDOSToolProviderRegistry : public QObject
{
    Q_OBJECT
public:
    explicit IDOSToolProviderRegistry(QObject* parent = nullptr);
    ~IDOSToolProviderRegistry() override;
    bool registerMetadata(IDOSToolProviderMetadata* metadata);
    bool unregisterMetadata(const QString& id);
    void clear();
    bool isBusy() const;
    void attach(IDOSToolBar* toolBar);
    void detach(IDOSToolBar* toolBar);
    IDOSToolProviderMetadata* metadata(const QString& id) const;
    QList<IDOSToolProviderMetadata*> metadata() const;
signals:
    void metadataRegistered(const QString& id);
    void metadataUnregistering(const QString& id);
    void metadataUnregistered(const QString& id);
private:
    QMap<QString, IDOSToolProviderMetadata*> m_metadataById;
    QList<QPointer<IDOSToolBar>> m_toolBars;
    bool m_changing;
};

#endif // IDOS_TOOL_PROVIDER_REGISTRY_H
