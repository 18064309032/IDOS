#ifndef IDOS_POINT_SET_TREE_METADATA_H
#define IDOS_POINT_SET_TREE_METADATA_H

#include "providers/idostreemetadata.h"

class IDOSPointSetTreeMetadata : public IDOSTreeMetadata
{
  public:
    QString typeId() const override;
    QString treeName() const override;
    IDOSTreeProvider* create(IDOSDataObject* data) const override;
};

#endif
