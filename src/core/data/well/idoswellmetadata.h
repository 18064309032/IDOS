#ifndef IDOSWELLMETADATA_H
#define IDOSWELLMETADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSWellMetadata : public IDOSDataMetadata
{
  public:
    IDOSWellMetadata();
    ~IDOSWellMetadata() override;

    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif
