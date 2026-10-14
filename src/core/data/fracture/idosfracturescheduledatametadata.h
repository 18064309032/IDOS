#ifndef IDOS_FRACTURE_SCHEDULE_DATA_METADATA_H
#define IDOS_FRACTURE_SCHEDULE_DATA_METADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSFractureScheduleDataMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif // IDOS_FRACTURE_SCHEDULE_DATA_METADATA_H
