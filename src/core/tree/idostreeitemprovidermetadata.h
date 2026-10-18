#ifndef IDOS_TREE_ITEM_PROVIDER_METADATA_H
#define IDOS_TREE_ITEM_PROVIDER_METADATA_H

#include <QString>

#include "idos_core.h"

class IDOSDataObject;
class IDOSTreeItem;
class IDOSTreeItemProvider;

class CORE_EXPORT IDOSTreeItemProviderMetadata
{
  public:
    IDOSTreeItemProviderMetadata();
    virtual ~IDOSTreeItemProviderMetadata();
    virtual QString id() const = 0;
    virtual QString treeName() const = 0;
    virtual QString groupKey() const;
    virtual bool supports(const IDOSDataObject* data) const;
    virtual IDOSTreeItemProvider* create(IDOSDataObject* data) const = 0;
};

#endif
