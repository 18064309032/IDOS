#ifndef IDOS_FLUID_ROCK_TABLE_TREE_METADATA_H
#define IDOS_FLUID_ROCK_TABLE_TREE_METADATA_H

#include "data/fluidrock/idosfluidrocktable.h"
#include "idos_app.h"
#include "providers/idostreeitemprovidermetadata.h"

class APP_EXPORT IDOSFluidRockTableTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    explicit IDOSFluidRockTableTreeMetadata(IDOSFluidRockTable::Category category);
    QString id() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeItemProvider* createProvider(IDOSDataObject* data) const override;

  private:
    IDOSFluidRockTable::Category m_category;
};

#endif
