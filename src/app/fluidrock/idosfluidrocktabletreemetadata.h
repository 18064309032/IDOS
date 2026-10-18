#ifndef IDOS_FLUID_ROCK_TABLE_TREE_METADATA_H
#define IDOS_FLUID_ROCK_TABLE_TREE_METADATA_H

#include "data/fluidrock/idosfluidrocktableobject.h"
#include "idos_app.h"
#include "tree/idostreeitemprovidermetadata.h"

class APP_EXPORT IDOSFluidRockTableTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    explicit IDOSFluidRockTableTreeMetadata(IDOSFluidRockTableObject::Category category);
    QString id() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeItemProvider* create(IDOSDataObject* data) const override;

  private:
    IDOSFluidRockTableObject::Category m_category;
};

#endif
