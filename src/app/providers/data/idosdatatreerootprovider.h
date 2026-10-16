#ifndef IDOS_DATA_TREE_ROOT_PROVIDER_H
#define IDOS_DATA_TREE_ROOT_PROVIDER_H

#include <QObject>

#include "idos_app.h"
#include "providers/idostreeitemprovider.h"

class APP_EXPORT IDOSDataTreeRootProvider : public QObject, public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    explicit IDOSDataTreeRootProvider();
    void refresh() override;
};

#endif
