#ifndef IDOSGRIDMETADATA_H
#define IDOSGRIDMETADATA_H

#include "providers/idosdatametadata.h"

class CORE_EXPORT IDOSGridMetadata : public IDOSDataMetadata
{
  public:
    IDOSGridMetadata();
    ~IDOSGridMetadata() override;

    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
};

#endif
