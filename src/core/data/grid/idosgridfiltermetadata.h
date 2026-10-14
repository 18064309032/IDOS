#ifndef IDOS_GRID_FILTER_METADATA_H
#define IDOS_GRID_FILTER_METADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSGridFilterMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif
