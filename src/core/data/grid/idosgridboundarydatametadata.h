#ifndef IDOS_GRID_BOUNDARY_DATA_METADATA_H
#define IDOS_GRID_BOUNDARY_DATA_METADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSGridBoundaryDataMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif // IDOS_GRID_BOUNDARY_DATA_METADATA_H
