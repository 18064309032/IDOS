#ifndef IDOS_GRID_TREE_PROVIDER_H
#define IDOS_GRID_TREE_PROVIDER_H

#include <QtCore/qnamespace.h>

#include "idos_app.h"
#include "providers/idostreeitemprovider.h"

class IDOSGridFilter;
class IDOSGridProperty;

class APP_EXPORT IDOSGridTreeProvider : public IDOSTreeItemProvider
{
  public:
    IDOSGridTreeProvider(IDOSDataObject* data);
    bool dependsOn(const QStringList& dataIds) const override;
    void refresh() override;

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title,
                          IDOSDataObject* data = nullptr);
};

#endif
