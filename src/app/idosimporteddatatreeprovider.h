#ifndef IDOS_IMPORTED_DATA_TREE_PROVIDER_H
#define IDOS_IMPORTED_DATA_TREE_PROVIDER_H

#include "idos_app.h"
#include "idostreeprovider.h"

class APP_EXPORT IDOSImportedDataTreeProvider : public IDOSTreeProvider
{
  public:
    explicit IDOSImportedDataTreeProvider(IDOSDataObject* data);
    void populate(IDOSTreeItem* item) override;
};

#endif // IDOS_IMPORTED_DATA_TREE_PROVIDER_H
