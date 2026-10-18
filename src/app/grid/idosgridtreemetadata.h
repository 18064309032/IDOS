#ifndef IDOS_GRID_TREE_METADATA_H
#define IDOS_GRID_TREE_METADATA_H

#include "idos_app.h"
#include "tree/idostreeitemprovidermetadata.h"

class IDOSGridTreeProvider;

class APP_EXPORT IDOSGridTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    QString id() const override;
    QString treeName() const override;
    QString groupKey() const override;
    bool supports(const IDOSDataObject* data) const override;
    IDOSTreeItemProvider* create(IDOSDataObject* data) const override;
};

#endif
