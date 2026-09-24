#ifndef IDOS_CASE_TREE_PROVIDER_H
#define IDOS_CASE_TREE_PROVIDER_H

#include "tree/idostreeprovider.h"

class IDOSCaseObject;
class IDOSTreeBuilder;

/**
 * @brief 工况树 Provider。
 */
class GUI_EXPORT IDOSCaseTreeProvider : public IDOSTreeProvider
{
public:
    ~IDOSCaseTreeProvider() override;

    virtual QString caseTypeId() const = 0;
    virtual void buildCaseTree(IDOSTreeBuilder& builder, const IDOSCaseObject* caseObject) = 0;
};

#endif // IDOS_CASE_TREE_PROVIDER_H
