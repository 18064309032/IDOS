#ifndef IDOS_CASE_TREE_ROOT_PROVIDER_H
#define IDOS_CASE_TREE_ROOT_PROVIDER_H

#include "idos_app.h"
#include "tree/idostreeitemprovider.h"

class APP_EXPORT IDOSCaseTreeRootProvider : public IDOSTreeItemProvider
{
  public:
    explicit IDOSCaseTreeRootProvider();
    void refresh() override;
};

#endif
