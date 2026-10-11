#ifndef IDOS_TREE_PROVIDER_H
#define IDOS_TREE_PROVIDER_H

#include <QStringList>

#include "idos_core.h"

class IDOSDataObject;
class IDOSTreeItem;

class CORE_EXPORT IDOSTreeProvider
{
  public:
    explicit IDOSTreeProvider(IDOSDataObject* data);
    virtual ~IDOSTreeProvider();
    IDOSDataObject* dataSource() const;
    // Populate a newly created branch before it is inserted into the tree.
    virtual void populate(IDOSTreeItem* item) = 0;
    // Refresh an attached branch; the default implementation calls populate().
    virtual void update(IDOSTreeItem* item);
    virtual bool dependsOn(const QStringList& dataIds) const;

  private:
    IDOSDataObject* m_data;
};

#endif
