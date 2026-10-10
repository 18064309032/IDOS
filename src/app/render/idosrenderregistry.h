#ifndef IDOS_RENDER_REGISTRY_H
#define IDOS_RENDER_REGISTRY_H

#include <QHash>
#include <QList>
#include <QString>

#include "idos_app.h"

class IDOSRenderMetadata;

class APP_EXPORT IDOSRenderRegistry
{
  public:
    IDOSRenderRegistry();
    ~IDOSRenderRegistry();

    bool registerMetadata(IDOSRenderMetadata* metadata);
    bool unregisterMetadata(const QString& dataTypeId);
    IDOSRenderMetadata* metadata(const QString& dataTypeId);
    QList<IDOSRenderMetadata*> metadataList();
    void clear();

  private:
    QHash<QString, IDOSRenderMetadata*> m_metadataByTypeId;
};

#endif // IDOS_RENDER_REGISTRY_H
