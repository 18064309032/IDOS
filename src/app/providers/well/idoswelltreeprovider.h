#ifndef IDOS_WELL_TREE_PROVIDER_H
#define IDOS_WELL_TREE_PROVIDER_H

#include "idos_app.h"
#include "providers/idostreeprovider.h"

class APP_EXPORT IDOSWellTreeProvider : public IDOSTreeProvider
{
  public:
    explicit IDOSWellTreeProvider(IDOSDataObject* data);
    void populate(IDOSTreeItem* item) override;

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title);
};

#endif
