#ifndef IDOS_FLUID_ROCK_TABLE_METADATA_H
#define IDOS_FLUID_ROCK_TABLE_METADATA_H

#include "data/idosdatametadata.h"
#include "idosfluidrocktableobject.h"

class CORE_EXPORT IDOSFluidRockTableMetadata : public IDOSDataMetadata
{
  public:
    explicit IDOSFluidRockTableMetadata(IDOSFluidRockTableObject::Category category);
    ~IDOSFluidRockTableMetadata() override;

    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;

  private:
    IDOSFluidRockTableObject::Category m_category;
};

#endif
