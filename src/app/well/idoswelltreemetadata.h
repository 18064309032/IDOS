#ifndef IDOS_WELL_TREE_METADATA_H
#define IDOS_WELL_TREE_METADATA_H

#include "idos_app.h"
#include "tree/idostreeitemprovidermetadata.h"

class IDOSWellTreeProvider;

class APP_EXPORT IDOSWellTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    QString id() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeItemProvider* create(IDOSDataObject* data) const override;
};

#endif
