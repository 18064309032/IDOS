#ifndef IDOS_PRODUCTION_HISTORY_DATA_METADATA_H
#define IDOS_PRODUCTION_HISTORY_DATA_METADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSProductionHistoryDataMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif // IDOS_PRODUCTION_HISTORY_DATA_METADATA_H
