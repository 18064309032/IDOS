#ifndef IDOS_DATA_METADATA_H
#define IDOS_DATA_METADATA_H

#include <QString>

#include "idos_core.h"

class IDOSDataObject;

class CORE_EXPORT IDOSDataMetadata
{
  public:
    IDOSDataMetadata();
    virtual ~IDOSDataMetadata();

    virtual QString typeId() const = 0;
    virtual QString displayName() const = 0;
    virtual IDOSDataObject* create() const = 0;
};

#endif
