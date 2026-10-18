#ifndef IDOS_DATA_TREE_ROOT_PROVIDER_H
#define IDOS_DATA_TREE_ROOT_PROVIDER_H


#include "idos_app.h"
#include "tree/idostreeitemprovider.h"

class APP_EXPORT IDOSDataTreeRootProvider : public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    explicit IDOSDataTreeRootProvider();
    void refresh() override;
};

#endif
