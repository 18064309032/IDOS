#ifndef IDOS_POINT_SET_TREE_PROVIDER_H
#define IDOS_POINT_SET_TREE_PROVIDER_H

#include "providers/idostreeprovider.h"

class IDOSPointSetTreeProvider : public IDOSTreeProvider
{
  public:
    explicit IDOSPointSetTreeProvider(IDOSDataObject* data);
    void populate(IDOSTreeItem* item) override;
};

#endif
