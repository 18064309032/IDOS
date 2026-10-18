#ifndef IDOS_FRACTURE_MATERIAL_CATALOG_DATA_METADATA_H
#define IDOS_FRACTURE_MATERIAL_CATALOG_DATA_METADATA_H

#include "data/idosdatametadata.h"
#include "idosfracturematerialcatalogobject.h"

class CORE_EXPORT IDOSFractureMaterialCatalogDataMetadata : public IDOSDataMetadata
{
  public:
    explicit IDOSFractureMaterialCatalogDataMetadata(IDOSFractureMaterialCatalogObject::Kind kind);

    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;

  private:
    IDOSFractureMaterialCatalogObject::Kind m_kind;
};

#endif // IDOS_FRACTURE_MATERIAL_CATALOG_DATA_METADATA_H
