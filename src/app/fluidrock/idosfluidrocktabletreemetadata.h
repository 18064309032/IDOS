#ifndef IDOS_FLUID_ROCK_TABLE_TREE_METADATA_H
#define IDOS_FLUID_ROCK_TABLE_TREE_METADATA_H

#include "idos_app.h"
#include "idosfluidrocktable.h"
#include "idostreemetadata.h"

class APP_EXPORT IDOSFluidRockTableTreeMetadata : public IDOSTreeMetadata
{
  public:
    explicit IDOSFluidRockTableTreeMetadata(IDOSFluidRockTable::Category category);
    QString typeId() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeProvider* create(IDOSDataObject* data) const override;

  private:
    IDOSFluidRockTable::Category m_category;
};

#endif
