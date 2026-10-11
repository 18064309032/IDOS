#ifndef IDOS_RENDER_REGISTRY_H
#define IDOS_RENDER_REGISTRY_H

#include <QMap>
#include <QList>
#include <QObject>
#include <QString>

#include "idos_render.h"

class IDOSRenderMetadata;
class IDOSRenderGuiProvider;

class RENDER_EXPORT IDOSRenderRegistry : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSRenderRegistry(QObject* parent = nullptr);
    ~IDOSRenderRegistry() override;

    bool add(IDOSRenderMetadata* metadata);
    bool remove(const QString& dataTypeId);
    IDOSRenderMetadata* metadata(const QString& dataTypeId) const;
    QList<IDOSRenderMetadata*> metadata() const;
    bool addGuiProvider(IDOSRenderGuiProvider* provider);
    bool removeGuiProvider(const QString& name);
    QList<IDOSRenderGuiProvider*> guiProviders() const;
    void clear();

  signals:
    void metadataAdded(const QString& typeId);
    void metadataAboutToBeRemoved(const QString& typeId);

  private:
    QMap<QString, IDOSRenderMetadata*> m_metadataByTypeId;
    QList<IDOSRenderGuiProvider*> m_guiProviders;
    bool m_changing;
};

#endif // IDOS_RENDER_REGISTRY_H
