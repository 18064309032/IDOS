#ifndef IDOS_WELL_TREE_METADATA_H
#define IDOS_WELL_TREE_METADATA_H

#include "idos_app.h"
#include "providers/idostreemetadata.h"

class IDOSWellTreeProvider;

class APP_EXPORT IDOSWellTreeMetadata : public IDOSTreeMetadata
{
  public:
    QString typeId() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeProvider* create(IDOSDataObject* data) const override;
};

#endif
