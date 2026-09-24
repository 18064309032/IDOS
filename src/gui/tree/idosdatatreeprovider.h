#ifndef IDOS_DATA_TREE_PROVIDER_H
#define IDOS_DATA_TREE_PROVIDER_H

#include "tree/idostreeprovider.h"

class IDOSDataObject;
class IDOSTreeBuilder;

/**
 * @brief 数据树 Provider。
 */
class GUI_EXPORT IDOSDataTreeProvider : public IDOSTreeProvider
{
public:
    ~IDOSDataTreeProvider() override;

    virtual QString typeId() const = 0;
    virtual void buildTree(IDOSTreeBuilder& builder, const IDOSDataObject* object) = 0;
};

#endif // IDOS_DATA_TREE_PROVIDER_H
