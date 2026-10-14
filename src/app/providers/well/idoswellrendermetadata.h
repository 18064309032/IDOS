#ifndef IDOS_WELL_RENDER_METADATA_H
#define IDOS_WELL_RENDER_METADATA_H

#include <QString>

#include "idos_app.h"
#include "providers/idosrendermetadata.h"

class APP_EXPORT IDOSWellRenderMetadata : public IDOSRenderMetadata
{
  public:
    QString typeId() const override;
    IDOSRenderProvider* create(IDOSDataObject* dataObject) const override;
};

#endif
