#ifndef IDOS_GRID_TREE_METADATA_H
#define IDOS_GRID_TREE_METADATA_H

#include "idos_app.h"
#include "idostreemetadata.h"

class IDOSGridTreeProvider;

class APP_EXPORT IDOSGridTreeMetadata : public IDOSTreeMetadata
{
  public:
    QString typeId() const override;
    QString treeName() const override;
    QString groupKey() const override;
    bool supports(const IDOSDataObject* data) const override;
    IDOSTreeProvider* create(IDOSDataObject* data) const override;
};

#endif
