#ifndef IDOS_FLUID_ROCK_TABLE_METADATA_H
#define IDOS_FLUID_ROCK_TABLE_METADATA_H

#include "idosdatametadata.h"
#include "idosfluidrocktable.h"

class CORE_EXPORT IDOSFluidRockTableMetadata : public IDOSDataMetadata
{
  public:
    explicit IDOSFluidRockTableMetadata(IDOSFluidRockTable::Category category);
    ~IDOSFluidRockTableMetadata() override;

    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;

  private:
    IDOSFluidRockTable::Category m_category;
};

#endif
