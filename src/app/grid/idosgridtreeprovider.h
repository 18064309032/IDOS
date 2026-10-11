#ifndef IDOS_GRID_TREE_PROVIDER_H
#define IDOS_GRID_TREE_PROVIDER_H

#include "idos_app.h"
#include "idostreeprovider.h"

class APP_EXPORT IDOSGridTreeProvider : public IDOSTreeProvider
{
  public:
    explicit IDOSGridTreeProvider(IDOSDataObject* data);
    void populate(IDOSTreeItem* item) override;

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title);
};

#endif
