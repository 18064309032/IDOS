#ifndef IDOS_MICROSEISMIC_DATA_METADATA_H
#define IDOS_MICROSEISMIC_DATA_METADATA_H

#include "idosdatametadata.h"

class CORE_EXPORT IDOSMicroseismicDataMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif // IDOS_MICROSEISMIC_DATA_METADATA_H
