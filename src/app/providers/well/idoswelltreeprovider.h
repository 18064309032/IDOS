#ifndef IDOS_WELL_TREE_PROVIDER_H
#define IDOS_WELL_TREE_PROVIDER_H

#include "idos_app.h"
#include "providers/idostreeitemprovider.h"

class APP_EXPORT IDOSWellTreeProvider : public IDOSTreeItemProvider
{
  public:
    IDOSWellTreeProvider(IDOSDataObject* data);
    void refresh() override;

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title);
};

#endif
