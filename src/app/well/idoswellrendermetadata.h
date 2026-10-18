#ifndef IDOS_WELL_RENDER_METADATA_H
#define IDOS_WELL_RENDER_METADATA_H

#include <QString>

#include "idos_app.h"
#include "render/idosrenderprovidermetadata.h"

class APP_EXPORT IDOSWellRenderMetadata : public IDOSRenderProviderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* dataObject, const QList<IDOSDataObject*>& references) const override;
};

#endif
