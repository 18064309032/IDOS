#ifndef IDOS_GRID_SURFACE_DATA_METADATA_H
#define IDOS_GRID_SURFACE_DATA_METADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSGridSurfaceDataMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif // IDOS_GRID_SURFACE_DATA_METADATA_H
