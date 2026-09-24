#ifndef IDOS_SIMULATION_CASE_TREE_PROVIDER_H
#define IDOS_SIMULATION_CASE_TREE_PROVIDER_H

#include "tree/idoscasetreeprovider.h"

/**
 * @brief 模拟工况树 Provider。
 */
class GUI_EXPORT IDOSSimulationCaseTreeProvider : public IDOSCaseTreeProvider
{
public:
    QString providerId() const override;
    QString caseTypeId() const override;
    void buildCaseTree(IDOSTreeBuilder& builder, const IDOSCaseObject* caseObject) override;
};

#endif // IDOS_SIMULATION_CASE_TREE_PROVIDER_H
