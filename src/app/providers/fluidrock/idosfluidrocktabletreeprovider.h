#ifndef IDOS_FLUID_ROCK_TABLE_TREE_PROVIDER_H
#define IDOS_FLUID_ROCK_TABLE_TREE_PROVIDER_H

#include "idos_app.h"
#include "providers/idostreeprovider.h"

class APP_EXPORT IDOSFluidRockTableTreeProvider : public IDOSTreeProvider
{
  public:
    explicit IDOSFluidRockTableTreeProvider(IDOSDataObject* data);
    void populate(IDOSTreeItem* item) override;
};

#endif
