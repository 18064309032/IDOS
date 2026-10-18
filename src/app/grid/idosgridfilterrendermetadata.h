#ifndef IDOS_GRID_FILTER_RENDER_METADATA_H
#define IDOS_GRID_FILTER_RENDER_METADATA_H

#include "idos_app.h"
#include "render/idosrenderprovidermetadata.h"

class APP_EXPORT IDOSGridFilterRenderMetadata : public IDOSRenderProviderMetadata
{
  public:
    QString typeId() const override;
    bool update(IDOSRenderProvider* provider, const QList<IDOSDataObject*>& references) const override;
    QStringList dependencies(const IDOSDataObject* object, const QVariantMap& state) const override;
    IDOSRenderProvider* create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const override;
};

#endif
