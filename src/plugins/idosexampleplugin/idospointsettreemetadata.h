#ifndef IDOS_POINT_SET_TREE_METADATA_H
#define IDOS_POINT_SET_TREE_METADATA_H

#include "providers/idostreeitemprovidermetadata.h"

class IDOSPointSetTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    QString id() const override;
    QString treeName() const override;
    IDOSTreeItemProvider* createProvider(IDOSDataObject* data) const override;
};

#endif
