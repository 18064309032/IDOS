#ifndef IDOS_TREE_METADATA_H
#define IDOS_TREE_METADATA_H

#include <QString>

#include "idos_core.h"

class IDOSDataObject;
class IDOSTreeProvider;

class CORE_EXPORT IDOSTreeMetadata
{
  public:
    IDOSTreeMetadata();
    virtual ~IDOSTreeMetadata();
    virtual QString typeId() const = 0;
    virtual QString treeName() const = 0;
    virtual QString groupKey() const;
    virtual bool supports(const IDOSDataObject* data) const;
    virtual IDOSTreeProvider* create(IDOSDataObject* data) const = 0;
};

#endif
