#ifndef IDOS_SEISMIC_VOLUME_DATA_METADATA_H
#define IDOS_SEISMIC_VOLUME_DATA_METADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSSeismicVolumeDataMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif // IDOS_SEISMIC_VOLUME_DATA_METADATA_H
