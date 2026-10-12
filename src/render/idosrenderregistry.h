#ifndef IDOS_RENDER_REGISTRY_H
#define IDOS_RENDER_REGISTRY_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include "idos_render.h"

class IDOSRenderMetadata;

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
    void clear();

  signals:
    void metadataAdded(const QString& typeId);
    void metadataAboutToBeRemoved(const QString& typeId);

  private:
    QMap<QString, IDOSRenderMetadata*> m_metadataByTypeId;
    bool m_changing;
};

#endif // IDOS_RENDER_REGISTRY_H
