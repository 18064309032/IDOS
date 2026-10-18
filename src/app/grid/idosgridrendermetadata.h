#ifndef IDOS_GRID_RENDER_METADATA_H
#define IDOS_GRID_RENDER_METADATA_H

#include <QString>

#include "idos_app.h"
#include "render/idosrenderprovidermetadata.h"

class APP_EXPORT IDOSGridRenderMetadata : public IDOSRenderProviderMetadata
{
  public:
    QString typeId() const override;
    bool update(IDOSRenderProvider* provider, const QList<IDOSDataObject*>& references) const override;
    QStringList dependencies(const IDOSDataObject* object, const QVariantMap& state) const override;
    IDOSRenderProvider* create(IDOSDataObject* dataObject, const QList<IDOSDataObject*>& references) const override;
};

#endif
