#ifndef IDOS_WELL_DATA_TREE_PROVIDER_H
#define IDOS_WELL_DATA_TREE_PROVIDER_H

#include "idosdatatreeprovider.h"

/**
 * @brief 井对象的数据树 Provider。
 */
class GUI_EXPORT IDOSWellDataTreeProvider : public IDOSDataTreeProvider
{
  public:
    QString groupKey() const override;
    QString providerId() const override;
    QString typeId() const override;
    void buildTree(IDOSTreeBuilder& builder, const IDOSDataObject* object) override;
};

#endif // IDOS_WELL_DATA_TREE_PROVIDER_H
